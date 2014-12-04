#ifndef HOST_UNDIRECTED_CANDIDATE_CONFLICT_TERM_H__
#define HOST_UNDIRECTED_CANDIDATE_CONFLICT_TERM_H__

#include <graph/Graph.h>
#include "HigherOrderArcTerm.h"

namespace host {

class UndirectedCandidateConflictTerm : public HigherOrderArcTerm {

public:

	/**
	 * Construct a candidate minimal spanning tree search for the given graph.
	 */
	UndirectedCandidateConflictTerm(const Graph& graph, const ArcTypes& arcTypes);

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas() { return _exclusiveArcs.size(); }

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
	 * Add the lambda contributions of this higher order term to the given arc 
	 * weights.
	 */
	void addArcWeights(ArcWeights& weights);

	/**
	 * Get the constant contribution of this higher order term to the objective.
	 */
	double constant();

	/**
	 * For the given MST (represented as boolean flags on arcs), compute the 
	 * gradient for each lambda and store it in the range pointed to with the 
	 * given iterator.
	 */
	bool gradient(
			const ArcSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	typedef std::vector<Arc>            ExclusiveArcs;
	typedef std::tuple<ExclusiveArcs, double> ExclusiveArcsLambda;

	void findExclusiveArcs(const ArcTypes& arcTypes);

	std::string toString(const ExclusiveArcs& arcs);

	// list of exclusive arcs and their lambda values, derived from conflicting 
	// candidates
	std::vector<ExclusiveArcsLambda> _exclusiveArcs;

	const Graph& _graph;
};

} // namespace host

#endif // HOST_UNDIRECTED_CANDIDATE_CONFLICT_TERM_H__

