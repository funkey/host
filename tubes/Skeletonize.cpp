#include <limits>
#include "Skeletonize.h"
#include <vigra/multi_distance.hxx>
#include <vigra/graph_algorithms.hxx>
#include <util/timing.h>
#include <util/ProgramOptions.h>

util::ProgramOption optionSkeletonBoundaryWeight(
		util::_long_name        = "skeletonBoundaryWeight",
		util::_description_text = "The weight of the boundary term to find the tube's skeletons.",
		util::_default_value    = 1);

Skeletonize::Skeletonize(ExplicitVolume<unsigned char>& volume) :
	_volume(volume),
	_grid(GridGraphType(_volume.data().shape(), vigra::IndirectNeighborhood)),
	_edgeMap(_grid),
	_dijkstra(_grid, _edgeMap) {}

Skeleton
Skeletonize::getSkeleton() {

	Timer t(__FUNCTION__);

	initializeEdgeMap();

	GridGraphType::Node root = findRoot();

	setRoot(root);

	Skeleton skeleton;

	// TODO:
	// • repeat until next longest branch is shorter than a threshold
	// • or certain number of branches extracted
	extractLongestBranch(skeleton);
	extractLongestBranch(skeleton);

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
			false,  /* compute distance from object (1) to background (0) */
			pitch);

	// find center point with maximal boundary distance
	float maxBoundaryDistance2 = 0;
	for (GridGraphType::NodeIt node(_grid); node != lemon::INVALID; ++node) {

		if (boundaryDistance[*node] > maxBoundaryDistance2) {

			_center = *node;
			maxBoundaryDistance2 = boundaryDistance[*node];
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
	vigra::edgeWeightsFromNodeWeights(
			_grid,
			boundaryDistance,
			_edgeMap);

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

	// length of diagonal in grid coordinates in volume
	float maxDiameter =
			sqrt(
					pow(_volume.getResolutionX(), 2) +
					pow(_volume.getResolutionY(), 2) +
					pow(_volume.getResolutionZ(), 2));

	for (GridGraphType::EdgeIt e(_grid); e != lemon::INVALID; e++) {

		GridGraphType::Node u = _grid.u(e);
		GridGraphType::Node v = _grid.v(e);

		// edges outside the volume
		if (_volume[u] == 0 || _volume[v] == 0) {

			// more expensive than longest possible path inside volume
			_edgeMap[e] =
					maxDiameter*                           /* >= length of longest straight path */
					nodeDistances[7]*(boundaryWeight + 1); /* >= max edge value */
			continue;
		}

		int i = 0;
		if (u[0] != v[0]) i |= 4;
		if (u[1] != v[1]) i |= 2;
		if (u[2] != v[2]) i |= 1;

		_edgeMap[e] = nodeDistances[i]*(_edgeMap[e] + 1);
	}
}

Skeletonize::GridGraphType::Node
Skeletonize::findRoot() {

	Timer t(__FUNCTION__);

	_dijkstra.run(_center);

	GridGraphType::Node root;
	float maxValue = 0;
	for (GridGraphType::NodeIt n(_grid); n != lemon::INVALID; n++)
		if (_volume[n] != 0 && _dijkstra.distMap()[n] > maxValue) {

			root     = n;
			maxValue = _dijkstra.distMap()[n];
		}

	// mark root as being part of skeleton
	_volume[root] = 2;

	return root;
}

void
Skeletonize::extractLongestBranch(Skeleton& skeleton) {

	Timer t(__FUNCTION__);

	_dijkstra.run(_root);

	// find furthest point
	GridGraphType::Node furthest;
	float maxValue = 0;
	for (GridGraphType::NodeIt n(_grid); n != lemon::INVALID; n++) {
		if (_volume[n] != 0 && _dijkstra.distMap()[n] > maxValue) {

			furthest = n;
			maxValue = _dijkstra.distMap()[n];
		}
	}

	skeleton.openNode(gridToVolume(furthest));

	GridGraphType::Node n = furthest;

	// walk backwards to next skeleton point
	while (_volume[n] != 2) {

		GridGraphType::Edge pred = _dijkstra.predMap()[n];

		GridGraphType::Node u = _grid.u(pred);
		GridGraphType::Node v = _grid.v(pred);

		n = (u == n ? v : u);

		skeleton.extendEdge(gridToVolume(n));
		_edgeMap[pred] = 0;
	}

	skeleton.openNode(gridToVolume(_root));
	skeleton.closeNode();

	skeleton.closeNode();
}
