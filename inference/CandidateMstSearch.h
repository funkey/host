#ifndef HOST_CANDIDATE_MST_SEARCH_H__
#define HOST_CANDIDATE_MST_SEARCH_H__

#include <graphs/Graph.h>

/**
 * A candidate minimal spanning tree (CMST) search. The graph is allowed to 
 * contain conflicting candidate nodes for the MST search. Conflicting 
 * candidates should be connected with a special conflict edge (see the EdgeType 
 * property of edges) and the graph of non-conflict edges needs to be connected.  
 * The CMST is the minimal spanning tree on a consistent subset of all 
 * candidates (so called accepted candidates). Rejected candidates are not part 
 * of the tree.
 */
class CandidateMstSearch {

public:

	/**
	 * Construct a candidate minimal spanning tree search for the given graph.
	 */
	CandidateMstSearch(const host::Graph& graph, const host::EdgeTypes& edgeTypes);

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
			const host::EdgeWeights& weights,
			host::EdgeSelection&     mst,
			double&                  value,
			unsigned int             maxIterations = 1000);

private:

	typedef std::vector<host::Edge>            ExclusiveEdges;
	typedef std::tuple<ExclusiveEdges, double> ExclusiveEdgesLambda;

	class ValueGradientCallback {

	public:

		ValueGradientCallback(
				CandidateMstSearch&      cmstSearch,
				const host::EdgeWeights& weights,
				host::EdgeSelection&     mst) :
			_cmstSearch(cmstSearch),
			_originalWeights(weights),
			_mst(mst) {}

		void operator()(const std::vector<double>& x, double& value, std::vector<double>& gradient);

	private:

		CandidateMstSearch&        _cmstSearch;
		const host::EdgeWeights&   _originalWeights;
		host::EdgeSelection&       _mst;
	};

	void findExclusiveEdges(const host::EdgeTypes& edgeTypes);

	void setLambdas(const std::vector<double>& x);

	void updateCurrentWeights(const host::EdgeWeights& originalWeights);

	double getCurrentMst(host::EdgeSelection& currentMst);

	void getGradient(
			const host::EdgeSelection& currentMst,
			std::vector<double>&       gradient);

	inline std::string toString(const ExclusiveEdges& edges);

	const host::Graph& _graph;

	// the current weights under consideration of the lambdas
	host::EdgeWeights _currentWeights;

	// list of exclusive edges and their lambda values, derived from conflicting 
	// candidates
	std::vector<ExclusiveEdgesLambda> _exclusiveEdges;
};

#endif // HOST_CANDIDATE_MST_SEARCH_H__

