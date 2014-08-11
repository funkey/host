#ifndef HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__
#define HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__

#include <graphs/Graph.h>

class LeafConstrainedMstSearch {

public:

	LeafConstrainedMstSearch(
			unsigned int maxIterations = 1000,
			double stepSize            = 1.0,
			double decceleration       = 0.99) :
		_maxIterations(maxIterations),
		_stepSize(stepSize),
		_decceleration(decceleration) {}

	/**
	 * Find a minimal spanning tree, where only a given subset of nodes are 
	 * leaves.
	 *
	 * @param graph
	 *              The graph to work on.
	 *
	 * @param weights
	 *              Edge weights.
	 *
	 * @param leaves
	 *              Marked nodes, which should form the leaves of the minimal 
	 *              spanning tree.
	 *
	 * @param mst
	 *              The edges that are part of the minimal spanning tree.
	 */
	void find(
			const host::Graph&         graph,
			const host::EdgeWeights&   weights,
			const host::NodeSelection& leaves,
			host::EdgeSelection&       mst);

private:

	void updateCurrentWeights(
			const host::Graph&       graph,
			const host::EdgeWeights& weights,
			const host::NodeWeights& lambdas,
			host::EdgeWeights& currentWeights);

	void getCurrentMst(
			const host::Graph&       graph,
			const host::EdgeWeights& weights,
			host::EdgeSelection&     currentMst);

	bool updateLambdas(
			const host::Graph&         graph,
			const host::EdgeSelection& currentMst,
			const host::NodeSelection& leaves,
			host::NodeWeights&         lambdas);

	// the maximal number of iterations in the search
	unsigned int _maxIterations;

	// the initial step size for the gradient ascent
	double _stepSize;

	// the amount by which to slow down the gradient ascent
	double _decceleration;
};

#endif // HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__

