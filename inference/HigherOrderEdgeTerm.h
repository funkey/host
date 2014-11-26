#ifndef HOST_INFERENCE_HIGHER_ORDER_TERM_H__
#define HOST_INFERENCE_HIGHER_ORDER_TERM_H__

#include "EdgeTerm.h"
#include "Lambdas.h"

class HigherOrderEdgeTerm : public EdgeTerm {

public:

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	virtual size_t numLambdas() = 0;

	/**
	 * Change the upper and lower bounds for each lambda in the the given 
	 * ranges. The defaults are -inf for the lower bounds and +inf for the upper 
	 * bounds.
	 */
	virtual void lambdaBounds(
			Lambdas::iterator beginLower,
			Lambdas::iterator endLower,
			Lambdas::iterator beginUpper,
			Lambdas::iterator endUpper) = 0;

	/**
	 * Set the lambda parameters.
	 */
	virtual void setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end) = 0;

	/**
	 * Add the lambda contributions of this higher order term to the given edge 
	 * weights.
	 */
	virtual void addEdgeWeights(host::EdgeWeights& weights) = 0;

	/**
	 * Get the constant contribution of this higher order term to the objective.
	 */
	virtual double constant() = 0;

	/**
	 * For the given MST (represented as boolean flags on edges), compute the 
	 * gradient for each lambda and store it in the range pointed to with the 
	 * given iterator.
	 */
	virtual void gradient(
			const host::EdgeSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end) = 0;
};

#endif // HOST_INFERENCE_HIGHER_ORDER_TERM_H__

