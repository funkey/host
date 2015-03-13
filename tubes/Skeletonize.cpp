#include <limits>
#include "Skeletonize.h"
#include <vigra/multi_distance.hxx>
#include <vigra/multi_gridgraph.hxx>
#include <util/timing.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>

util::ProgramOption optionSkeletonBoundaryWeight(
		util::_long_name        = "skeletonBoundaryWeight",
		util::_description_text = "The weight of the boundary term to find the tube's skeletons.",
		util::_default_value    = 1);

util::ProgramOption optionSkeletonMaxNumSegments(
		util::_long_name        = "skeletonMaxNumSegments",
		util::_description_text = "The maximal number of segments to extract for a skeleton.",
		util::_default_value    = 10);

util::ProgramOption optionSkeletonMinSegmentLength(
		util::_long_name        = "skeletonMinSegmentLength",
		util::_description_text = "The mininal length of a segment (including the boundary penalty) to extract for a skeleton.",
		util::_default_value    = 0);

util::ProgramOption optionSkeletonMinSegmentLengthRatio(
		util::_long_name        = "skeletonMinSegmentLengthRatio",
		util::_description_text = "The mininal length of a segment (including the boundary penalty) as a ration of the largest segment to extract for a skeleton.",
		util::_default_value    = 1);

util::ProgramOption optionSkeletonSkipExplainedNodes(
		util::_long_name        = "skeletonSkipExplainedNodes",
		util::_description_text = "Don't add segments to nodes that are already explained by the current skeleton. "
		                          "Nodes are explained, if they fall within a sphere around any current skeleton node. "
		                          "The size of the sphere is determined by boundary distance * skeletonExplanationWeight.");

util::ProgramOption optionSkeletonExplanationWeight(
		util::_long_name        = "skeletonExplanationWeight",
		util::_description_text = "A factor to multiply with the boundary distance to create 'explanation spheres'. "
		                          "See skeletonSkipExplainedNodes.",
		util::_default_value    = 1);

logger::LogChannel skeletonizelog("skeletonizelog", "[Skeletonize] ");

Skeletonize::Skeletonize(ExplicitVolume<unsigned char>& volume) :
	_volume(volume),
	_boundaryDistance(_volume.data().shape(), 0.0),
	_positionMap(_graph),
	_distanceMap(_graph),
	_boundaryWeight(optionSkeletonBoundaryWeight),
	_dijkstra(_graph, _distanceMap),
	_minSegmentLength(optionSkeletonMinSegmentLength),
	_minSegmentLengthRatio(optionSkeletonMinSegmentLengthRatio),
	_skipExplainedNodes(optionSkeletonSkipExplainedNodes),
	_explanationWeight(optionSkeletonExplanationWeight) {

	vigra::MultiArray<3, Graph::Node> nodeIds(_volume.data().shape());
	vigra::GridGraph<3> grid(_volume.data().shape(), vigra::IndirectNeighborhood);

	// add all non-background nodes
	for (vigra::GridGraph<3>::NodeIt node(grid); node != lemon::INVALID; ++node) {

		if (_volume[node] == Background)
			continue;

		Graph::Node n   = _graph.addNode();
		nodeIds[node]   = n;
		_positionMap[n] = *node;
	}

	// add all edges between non-background nodes and label boundary nodes 
	// on-the-fly
	for (vigra::GridGraph<3>::EdgeIt edge(grid); edge != lemon::INVALID; ++edge) {

		int insideVoxels = (_volume[grid.u(edge)] != Background) + (_volume[grid.v(edge)] != Background);

		if (insideVoxels == 1) {

			if (_volume[grid.u(edge)] != Background)
				_volume[grid.u(edge)] = Boundary;
			if (_volume[grid.v(edge)] != Background)
				_volume[grid.v(edge)] = Boundary;
		}

		if (insideVoxels != 2)
			continue;

		Graph::Node u = nodeIds[grid.u(edge)];
		Graph::Node v = nodeIds[grid.v(edge)];

		_graph.addEdge(u, v);
	}

	// get a list of boundary nodes
	for (Graph::NodeIt node(_graph); node != lemon::INVALID; ++node)
		if (_volume[_positionMap[node]] == Boundary)
			_boundary.push_back(node);
}

Skeleton
Skeletonize::getSkeleton() {

	Timer t(__FUNCTION__);

	initializeEdgeMap();

	Graph::Node root = findRoot();

	setRoot(root);

	int maxNumSegments = optionSkeletonMaxNumSegments;
	int segmentsFound = 0;
	while (extractLongestSegment() && ++segmentsFound < maxNumSegments) {}

	return parseVolumeSkeleton();
}

void
Skeletonize::initializeEdgeMap() {

	Timer t(__FUNCTION__);

	// We assume the pitch vigra needs is the number of measurements per unit. 
	// Our units are nm, and the volume tells us via getResolution?() the size 
	// of a pixel. Hence, the number of measurements per nm in either direction 
	// is 1/resolution of this direction.
	float pitch[3];
	pitch[0] = 1.0/_volume.getResolutionX();
	pitch[1] = 1.0/_volume.getResolutionY();
	pitch[2] = 1.0/_volume.getResolutionZ();

	vigra::separableMultiDistSquared(
			_volume.data(),
			_boundaryDistance,
			false,  /* compute distance from object (non-zero) to background (0) */
			pitch);

	// find center point with maximal boundary distance
	_maxBoundaryDistance2 = 0;
	for (Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		const Position& pos = _positionMap[node];
		if (_boundaryDistance[pos] > _maxBoundaryDistance2) {

			_center = node;
			_maxBoundaryDistance2 = _boundaryDistance[pos];
		}
	}

	std::cout << _maxBoundaryDistance2 << std::endl;

	using namespace vigra::functor;

	// create initial edge map from boundary penalty
	for (Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e)
		_distanceMap[e] = boundaryPenalty(
				0.5*(
						_boundaryDistance[_positionMap[_graph.u(e)]] +
						_boundaryDistance[_positionMap[_graph.v(e)]]));

	// multiply with Euclidean node distances
	//
	// The TEASAR paper suggests to add the Euclidean distances. However, for 
	// the penalty to be meaningful in anistotropic volumes, it should be 
	// multiplied with the Euclidean distance between the nodes (otherwise, I is 
	// more expensive to move in the high-resolution dimensions). Therefore, the 
	// final value is
	//
	//   penalty*euclidean + euclidean = euclidean*(penalty + 1)

	float nodeDistances[8];
	nodeDistances[0] = 0;
	nodeDistances[1] = _volume.getResolutionZ();
	nodeDistances[2] = _volume.getResolutionY();
	nodeDistances[3] = sqrt(pow(_volume.getResolutionY(), 2) + pow(_volume.getResolutionZ(), 2));
	nodeDistances[4] = _volume.getResolutionX();
	nodeDistances[5] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionZ(), 2));
	nodeDistances[6] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionY(), 2));
	nodeDistances[7] = sqrt(pow(_volume.getResolutionX(), 2) + pow(_volume.getResolutionY(), 2) + pow(_volume.getResolutionZ(), 2));

	for (Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e) {

		Position u = _positionMap[_graph.u(e)];
		Position v = _positionMap[_graph.v(e)];

		int i = 0;
		if (u[0] != v[0]) i |= 4;
		if (u[1] != v[1]) i |= 2;
		if (u[2] != v[2]) i |= 1;

		_distanceMap[e] = nodeDistances[i]*(_distanceMap[e] + 1);
	}
}

Skeletonize::Graph::Node
Skeletonize::findRoot() {

	Timer t(__FUNCTION__);

	_dijkstra.run(_center);

	// find furthest point on boundary
	Graph::Node root = Graph::NodeIt(_graph);
	float maxValue = -1;
	for (Graph::Node n : _boundary)
		if (_dijkstra.distMap()[n] > maxValue) {

			root     = n;
			maxValue = _dijkstra.distMap()[n];
		}

	if (maxValue == -1)
		UTIL_THROW_EXCEPTION(
				NoNodeFound,
				"could not find a root boundary point");

	// mark root as being part of skeleton
	_volume[_positionMap[root]] = OnSkeleton;

	return root;
}

bool
Skeletonize::extractLongestSegment() {

	Timer t(__FUNCTION__);

	_dijkstra.run(_root);

	// find furthest point on boundary
	Graph::Node furthest = Graph::NodeIt(_graph);
	float maxValue = -1;
	for (Graph::Node n : _boundary) {

		if (_skipExplainedNodes && _volume[_positionMap[n]] == Explained)
			continue;

		if (_dijkstra.distMap()[n] > maxValue) {

			furthest = n;
			maxValue = _dijkstra.distMap()[n];
		}
	}

	// no more points or length smaller then min segment length
	if (maxValue == -1 || maxValue < _minSegmentLength)
		return false;

	LOG_DEBUG(skeletonizelog) << "extracting segment with length " << maxValue << std::endl;

	Graph::Node n = furthest;

	// walk backwards to next skeleton point
	while (_volume[_positionMap[n]] != OnSkeleton) {

		_volume[_positionMap[n]] = OnSkeleton;

		if (_skipExplainedNodes)
			drawExplanationSphere(_positionMap[n]);

		Graph::Edge pred = _dijkstra.predMap()[n];
		Graph::Node u = _graph.u(pred);
		Graph::Node v = _graph.v(pred);

		n = (u == n ? v : u);

		_distanceMap[pred] = 0.0;
	}

	// first segment?
	if (n == _root) {

		LOG_DEBUG(skeletonizelog) << "longest segment has length " << maxValue << std::endl;

		_minSegmentLength = std::max(_minSegmentLength, _minSegmentLengthRatio*maxValue);

		LOG_DEBUG(skeletonizelog) << "setting min segment length to " << _minSegmentLength << std::endl;
	}

	return true;
}

void
Skeletonize::drawExplanationSphere(const Position& center) {

	double radius2 = _boundaryDistance[center]*pow(_explanationWeight, 2);

	double resX2 = pow(_volume.getResolutionX(), 2);
	double resY2 = pow(_volume.getResolutionY(), 2);
	double resZ2 = pow(_volume.getResolutionZ(), 2);

	for (Graph::Node n : _boundary) {

		const Position& pos = _positionMap[n];
		double distance2 =
				resX2*pow(pos[0] - center[0], 2) +
				resY2*pow(pos[1] - center[1], 2) +
				resZ2*pow(pos[2] - center[2], 2);

		if (distance2 <= radius2)
			if (_volume[pos] != OnSkeleton)
				_volume[pos] = Explained;
	}
}

double
Skeletonize::boundaryPenalty(double boundaryDistance) {

	// penalty = w*(1.0 - bd/max_bd)^16
	//
	//   bd    : boundary distance
	//   max_bd: max boundary distance
	//   16    : magic number, taken from TEASAR paper
	return _boundaryWeight*pow(1.0 - sqrt(boundaryDistance/_maxBoundaryDistance2), 16);
}

Skeleton
Skeletonize::parseVolumeSkeleton() {

	Skeleton skeleton;

	skeleton.setBoundingBox(_volume.getBoundingBox());

	traverse(_positionMap[_root], skeleton);

	return skeleton;
}

void
Skeletonize::traverse(const Position& pos, Skeleton& skeleton) {

	_volume[pos] = Visited;

	float x, y, z;
	_volume.getRealLocation(pos[0], pos[1], pos[2], x, y, z);
	Skeleton::Position realPos(x, y, z);

	int neighbors = numNeighbors(pos);
	bool isNode = (neighbors != 2);

	if (isNode)
		skeleton.openNode(realPos);
	else
		skeleton.extendEdge(realPos);

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == _volume.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == _volume.height() - 1 ? 0 : 1);
	int ez = (pos[2] == _volume.depth()  - 1 ? 0 : 1);

	// as soon as 'neighbors' is negative, we know that there are no more 
	// neighbors left to test (0 is not sufficient, since we count ourselves as 
	// well)
	for (int dz = sz; dz <= ez && neighbors >= 0; dz++)
	for (int dy = sy; dy <= ey && neighbors >= 0; dy++)
	for (int dx = sx; dx <= ex && neighbors >= 0; dx++) {

		vigra::Shape3 p = pos + vigra::Shape3(dx, dy, dz);

		if (_volume[p] >= OnSkeleton) {

			neighbors--;

			if (_volume[p] != Visited)
				traverse(p, skeleton);
		}
	}

	if (isNode)
		skeleton.closeNode();
}

int
Skeletonize::numNeighbors(const Position& pos) {

	int num = 0;

	int sx = (pos[0] == 0 ? 0 : -1);
	int sy = (pos[1] == 0 ? 0 : -1);
	int sz = (pos[2] == 0 ? 0 : -1);
	int ex = (pos[0] == _volume.width()  - 1 ? 0 : 1);
	int ey = (pos[1] == _volume.height() - 1 ? 0 : 1);
	int ez = (pos[2] == _volume.depth()  - 1 ? 0 : 1);

	for (int dz = sz; dz <= ez; dz++)
	for (int dy = sy; dy <= ey; dy++)
	for (int dx = sx; dx <= ex; dx++) {

		if (_volume(pos[0] + dx, pos[1] + dy, pos[2] + dz) >= OnSkeleton)
			num++;
	}

	if (_volume[pos] >= OnSkeleton)
		num--;

	return num;
}

