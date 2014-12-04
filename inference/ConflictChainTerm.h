#ifndef HOST_INFERENCE_CONFLICT_CHAIN_TERM_H__
#define HOST_INFERENCE_CONFLICT_CHAIN_TERM_H__

#include <graph/Graph.h>
#include "HigherOrderArcTerm.h"

namespace host {

/**
 * Implements Lagrangian terms and multipliers for conflict chain constraints up 
 * to a given length.
 * 
 * A conflict chain constraint states that along an acyclic chain of directed 
 * conflict edges C = (e_1,e_2,...,e_n), at not all edges can be selected:
 *
 *   Σ_{e∈C} y_e ≤ |C|-1
 *   =
 *   Σ_{e∈C} y_e - |C|+1 ≤ 0
 *
 *
 * The objective will be augmented with the term
 *
 *   λ_C( Σ_{e∈C} y_e - |C|+1)
 *
 * hence, λ_C gets added to the weights of each edge in C, and λ_C is subtracted 
 * from the constant.
 */
class ConflictChainTerm : public HigherOrderArcTerm {

public:

	/**
	 * Construct a candidate conflict term for the given graph.
	 */
	ConflictChainTerm(
			const Graph& graph,
			const ArcTypes& arcTypes,
			unsigned int maxChainLength);

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas() { return _chains.size(); }

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
	 *
	 * @return true, if the current mst is feasible
	 */
	bool gradient(
			const ArcSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	void findChains();

	void findChains(
			std::vector<Arc>& arcs,
			const Arc&        nextArc);

	typedef std::vector<Arc> Arcs;

	struct Chain {

		size_t length() const { return arcs.size(); }

		Arcs   arcs;
		double lambda;
	};

	unsigned int _maxChainLength;

	std::vector<Chain> _chains;

	const Graph&    _graph;
	const ArcTypes& _arcTypes;
};

} // namespace host

#endif // HOST_INFERENCE_CONFLICT_CHAIN_TERM_H__

