#ifndef HOST_CANDIDATE_CONFLICT_TERM_H__
#define HOST_CANDIDATE_CONFLICT_TERM_H__

#include <graphs/Graph.h>
#include "HigherOrderEdgeTerm.h"

class CandidateConflictTerm : public HigherOrderEdgeTerm {

public:

	/**
	 * Construct a candidate minimal spanning tree search for the given graph.
	 */
	CandidateConflictTerm(const host::Graph& graph, const host::EdgeTypes& edgeTypes);

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas() { return _exclusiveEdges.size(); }

	/**
	 * Store the upper and lower bounds for each lambda in the the given ranges.
	 */
	void lambdaBounds(
			Lambdas::iterator beginLower,
			Lambdas::iterator endLower,
			Lambdas::iterator beginUpper,
			Lambdas::iterator endUpper);

	/**
	 * Set the lambda parameters.
	 */
	void setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end);

	/**
	 * Add the lambda contributions of this higher order term to the given edge 
	 * weights.
	 */
	void addEdgeWeights(host::EdgeWeights& weights);

	/**
	 * Get the constant contribution of this higher order term to the objective.
	 */
	double constant();

	/**
	 * For the given MST (represented as boolean flags on edges), compute the 
	 * gradient for each lambda and store it in the range pointed to with the 
	 * given iterator.
	 */
	void gradient(
			const host::EdgeSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	typedef std::vector<host::Edge>            ExclusiveEdges;
	typedef std::tuple<ExclusiveEdges, double> ExclusiveEdgesLambda;

	void findExclusiveEdges(const host::EdgeTypes& edgeTypes);

	std::string toString(const ExclusiveEdges& edges);

	// list of exclusive edges and their lambda values, derived from conflicting 
	// candidates
	std::vector<ExclusiveEdgesLambda> _exclusiveEdges;

	const host::Graph& _graph;
};

#endif // HOST_CANDIDATE_CONFLICT_TERM_H__

