#ifndef HOST_TUBES_SKELETONIZE_H__
#define HOST_TUBES_SKELETONIZE_H__

#include <imageprocessing/ExplicitVolume.h>
#define WITH_LEMON
#include <lemon/dijkstra.h>
#include "Skeleton.h"

class Skeletonize {

	typedef vigra::MultiArray<3, unsigned char> VolumeType;
	typedef VolumeType::difference_type         Position;
	typedef lemon::ListGraph                    Graph;
	typedef Graph::NodeMap<Position>            PositionMap;
	typedef Graph::EdgeMap<double>              DistanceMap;

public:

	Skeletonize(ExplicitVolume<unsigned char>& volume);

	Skeleton getSkeleton();

private:

	/**
	 * Initialize the edge map, such that initial edges inside the volume are 
	 * Euclidean distance plus boundary distance penalty, and all other ones 
	 * infinite.
	 */
	void initializeEdgeMap();

	/**
	 * Find the root node as the furthest point from the highest boundary 
	 * distance point.
	 */
	Graph::Node findRoot();

	/**
	 * Set the root node of the skeleton. This should be a point with maximal 
	 * distance to some internal point.
	 */
	void setRoot(Graph::Node root) { _root = root; }

	/**
	 * Compute or update the shortest paths from the root node to all other 
	 * points. Takes the current edge map for distances.
	 */
	void findShortestPaths();

	/**
	 * Find the furthest point and walk backwards along the shortest path to the 
	 * current skeleton. This marks all points on the path as being part of the 
	 * skeleton, and all points in the vicinity as beeing processed. The edge 
	 * values along the shortest path will be set to zero. Additionally, the 
	 * branch will be added to the passed skeleton.
	 */
	void extractLongestBranch(Skeleton& skeleton);

	/**
	 * Find the point with the largest distance according to the last call to 
	 * findShortestPaths().
	 */
	Position getFurthestPoint();

	/**
	 * Convert grid positions to volume positions.
	 */
	Skeleton::Position gridToVolume(Position pos) {

		return Skeleton::Position(
				_volume.getBoundingBox().getMinX() + (float)pos[0]*_volume.getResolutionX(),
				_volume.getBoundingBox().getMinY() + (float)pos[1]*_volume.getResolutionY(),
				_volume.getBoundingBox().getMinZ() + (float)pos[2]*_volume.getResolutionZ());
	}

	// reference to the volume to process
	ExplicitVolume<unsigned char>& _volume;

	// lemon graph compatible datastructures for Dijkstra
	Graph       _graph;
	PositionMap _positionMap;
	DistanceMap _distanceMap;

	lemon::Dijkstra<Graph, DistanceMap> _dijkstra;

	Graph::Node _root;
	Graph::Node _center;
};

#endif // HOST_TUBES_SKELETONIZE_H__

