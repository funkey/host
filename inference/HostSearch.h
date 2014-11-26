#ifndef HOST_INFERENCE_HOST_SEARCH_H__
#define HOST_INFERENCE_HOST_SEARCH_H__

#include <vector>
#include "EdgeTerm.h"
#include "HigherOrderEdgeTerm.h"

class HostSearch {

public:

	HostSearch(const host::Graph& graph) :
		_currentWeights(graph),
		_graph(graph) {}

	/**
	 * Add an edge term to the objective of this search.
	 */
	void addTerm(EdgeTerm* term);

	/**
	 * Find a minimal spanning tree on a consistent subset of the provided 
	 * candidate nodes.
	 *
	 * @param weights
	 *              Edge weights for the graph associated to this search.
	 *
	 * @param mst
	 *              The edges that are part of the minimal spanning tree.
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
			host::EdgeSelection&     mst,
			double&                  value,
			unsigned int             maxIterations = 1000);

private:

	class ValueGradientCallback {

	public:

		ValueGradientCallback(
				HostSearch&              hostSearch,
				host::EdgeSelection&     mst) :
			_hostSearch(hostSearch),
			_mst(mst) {}

		void operator()(const Lambdas& x, double& value, Lambdas& gradient);

	private:

		HostSearch&          _hostSearch;
		host::EdgeSelection& _mst;
	};

	size_t numLambdas();

	// set the lambdas in all higher-order edge terms
	void setLambdas(const Lambdas& x);

	// assemble the edge weights from all edge terms
	void updateWeights();

	// find the minimal spanning tree on the current weights
	double mst(host::EdgeSelection& currentMst);

	// get the gradient for the given mst
	void gradient(
			const host::EdgeSelection& mst,
			Lambdas&                   gradient);

	std::vector<EdgeTerm*>            _edgeTerms;
	std::vector<HigherOrderEdgeTerm*> _higherOrderEdgeTerms;

	// the current weights under consideration of all terms
	host::EdgeWeights _currentWeights;

	const host::Graph& _graph;
};

#endif // HOST_INFERENCE_HOST_SEARCH_H__

