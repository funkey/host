#include <limits>
#include "Skeletonize.h"
#include <vigra/multi_distance.hxx>
#include <vigra/multi_gridgraph.hxx>
#include <util/timing.h>
#include <util/ProgramOptions.h>

util::ProgramOption optionSkeletonBoundaryWeight(
		util::_long_name        = "skeletonBoundaryWeight",
		util::_description_text = "The weight of the boundary term to find the tube's skeletons.",
		util::_default_value    = 1);

util::ProgramOption optionSkeletonMaxNumBranches(
		util::_long_name        = "skeletonMaxNumBranches",
		util::_description_text = "The maximal number of branches to extract for a skeleton.",
		util::_default_value    = 2);

Skeletonize::Skeletonize(ExplicitVolume<unsigned char>& volume) :
	_volume(volume),
	_positionMap(_graph),
	_distanceMap(_graph),
	_dijkstra(_graph, _distanceMap) {

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

	Skeleton skeleton;

	int maxNumBranches = optionSkeletonMaxNumBranches;
	int branchesFound = 0;
	while (extractLongestBranch(skeleton) && ++branchesFound < maxNumBranches) {}

	return skeleton;
}

void
Skeletonize::initializeEdgeMap() {

	Timer t(__FUNCTION__);

	const double boundaryWeight = optionSkeletonBoundaryWeight;

	// perform distance transform to get boundary distance
	vigra::MultiArray<3, float> boundaryDistance(_volume.data().shape(), 0.0);

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
			boundaryDistance,
			false,  /* compute distance from object (non-zero) to background (0) */
			pitch);

	// find center point with maximal boundary distance
	float maxBoundaryDistance2 = 0;
	for (Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		const Position& pos = _positionMap[node];
		if (boundaryDistance[pos] > maxBoundaryDistance2) {

			_center = node;
			maxBoundaryDistance2 = boundaryDistance[pos];
		}
	}

	using namespace vigra::functor;

	// penalty = w*(1.0 - bd/max_bd)^16
	//
	//   bd    : boundary distance
	//   max_bd: max boundary distance
	//   16    : magic number, taken from TEASAR paper
	vigra::transformMultiArray(
			boundaryDistance,
			boundaryDistance,
			Param(boundaryWeight)*pow(Param(1.0) - sqrt(Arg1()/Param(maxBoundaryDistance2)), Param(16)));

	// create initial edge map from boundary penalty
	for (Graph::EdgeIt e(_graph); e != lemon::INVALID; ++e)
		_distanceMap[e] = 0.5*(
				boundaryDistance[_positionMap[_graph.u(e)]] +
				boundaryDistance[_positionMap[_graph.v(e)]]);

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
Skeletonize::extractLongestBranch(Skeleton& skeleton) {

	Timer t(__FUNCTION__);

	_dijkstra.run(_root);

	// find furthest point on boundary
	Graph::Node furthest = Graph::NodeIt(_graph);
	float maxValue = -1;
	for (Graph::Node n : _boundary)
		if (_dijkstra.distMap()[n] > maxValue) {

			furthest = n;
			maxValue = _dijkstra.distMap()[n];
		}

	if (maxValue == -1)
		UTIL_THROW_EXCEPTION(
				NoNodeFound,
				"could not find a furthest boundary point");

	// everything is part of the skeleton
	if (maxValue == 0)
		return false;

	skeleton.openNode(gridToVolume(_positionMap[furthest]));

	Graph::Node n = furthest;

	// walk backwards to next skeleton point
	while (_volume[_positionMap[n]] != OnSkeleton) {

		_volume[_positionMap[n]] = OnSkeleton;

		Graph::Edge pred = _dijkstra.predMap()[n];
		Graph::Node u = _graph.u(pred);
		Graph::Node v = _graph.v(pred);

		n = (u == n ? v : u);

		skeleton.extendEdge(gridToVolume(_positionMap[n]));
		_distanceMap[pred] = 0.0;
	}

	skeleton.openNode(gridToVolume(_positionMap[_root]));
	skeleton.closeNode();

	skeleton.closeNode();

	return true;
}
