#ifndef HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__
#define HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__

#include <graphs/Graph.h>

class LeafConstrainedMstSearch {

public:

	/**
	 * Construct a leaf constrained minimal spanning tree search for the given 
	 * graph.
	 */
	LeafConstrainedMstSearch(const host::Graph& graph) :
		_graph(graph),
		_lambdas(_graph),
		_currentWeights(_graph) {}

	/**
	 * Find a minimal spanning tree, where only a given subset of nodes are 
	 * leaves.
	 *
	 * @param weights
	 *              Arc weights for the graph associated to this search.
	 *
	 * @param leaves
	 *              Marked nodes, which should form the leaves of the minimal 
	 *              spanning tree.
	 *
	 * @param mst
	 *              The arcs that are part of the minimal spanning tree.
	 *
	 * @param value
	 *              The length of the minimal spanning tree.
	 *
	 * @param maxIterations
	 *              The maximal number of iterations to spent on the search.
	 *
	 * @return
	 *              True, if a minimal spanning tree that fulfills all 
	 *              constraints could be found.
	 */
	bool find(
			const host::ArcWeights&   weights,
			const host::NodeSelection& leaves,
			host::ArcSelection&       mst,
			double&                    value,
			unsigned int               maxIterations = 1000);

private:

	class ValueGradientCallback {

	public:

		ValueGradientCallback(
				LeafConstrainedMstSearch&  lcmstSearch,
				const host::ArcWeights&   weights,
				const host::NodeSelection& leaves,
				host::ArcSelection&       mst) :
			_lcmstSearch(lcmstSearch),
			_originalWeights(weights),
			_leaves(leaves),
			_mst(mst) {}

		void operator()(const std::vector<double>& x, double& value, std::vector<double>& gradient);

	private:

		LeafConstrainedMstSearch&  _lcmstSearch;
		const host::ArcWeights&   _originalWeights;
		const host::NodeSelection& _leaves;
		host::ArcSelection&       _mst;
	};

	void setLambdas(const std::vector<double>& x);

	void updateCurrentWeights(const host::ArcWeights& originalWeights);

	double getCurrentMst(
			const host::NodeSelection& leaves,
			host::ArcSelection&       currentMst);

	void getGradient(
			const host::ArcSelection& currentMst,
			const host::NodeSelection& leaves,
			std::vector<double>&       gradient);

	const host::Graph& _graph;

	// the changes to the adjacent arcs of a node
	host::NodeWeights _lambdas;

	// the current weights under consideration of the lambdas
	host::ArcWeights _currentWeights;
};

#endif // HOST_INFERENCE_LEAF_CONSTRAINED_MST_SEARCH_H__

