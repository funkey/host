#ifndef HOST_CANDIDATE_CONFLICT_TERM_H__
#define HOST_CANDIDATE_CONFLICT_TERM_H__

#include <graphs/Graph.h>
#include "HigherOrderArcTerm.h"

namespace host {

class CandidateConflictTerm : public HigherOrderArcTerm {

public:

	/**
	 * Construct a candidate conflict term for the given graph.
	 */
	CandidateConflictTerm(const Graph& graph, const ArcTypes& arcTypes);

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas() { return _exclusiveArcs.size() + _conflictArcs.size(); }

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
	void gradient(
			const ArcSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	typedef std::vector<Arc> ExclusiveArcs;

	// list of exclusive arcs (not all of them can be selected at the same time) 
	// and the lambda value assoticated with them
	struct ExclusiveArcsLambda {

		ExclusiveArcs arcs;
		double        lambda;
	};

	// arc and a list of conflict arcs (no conflict arc can be selected, if arc 
	// is selected) and the lambda value associated with them
	struct ArcLambda {

		Arc              arc;
		std::vector<Arc> conflictArcs;
		double           lambda;
	};

	// find conflict nodes and add an ArcLambda for each incoming edge
	void findArcLambdas(const ArcTypes& arcTypes);

	// find pairs of mutual exclusive arcs
	void findExclusiveArcs(const ArcTypes& arcTypes);

	inline std::string toString(const Arc& arc);

	inline std::string toString(const ExclusiveArcs& arcs);

	// list of conflict nodes and their lambda values
	std::vector<ExclusiveArcsLambda> _exclusiveArcs;
	std::vector<ArcLambda>           _conflictArcs;

	const Graph& _graph;
};

} // namespace host

#endif // HOST_CANDIDATE_CONFLICT_TERM_H__

