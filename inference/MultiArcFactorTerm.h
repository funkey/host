#ifndef HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__
#define HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__

#include <map>
#include <vector>
#include "HigherOrderArcTerm.h"
#include "MultiArcFactors.h"

namespace host {

class MultiArcFactorTerm : public HigherOrderArcTerm {

public:

	MultiArcFactorTerm(
			const host::Graph&      graph,
			const MultiArcFactors& factors) :
		_graph(graph),
		_factors(factors) {}

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas();

	/**
	 * Change the upper and lower bounds for each lambda in the the given 
	 * ranges. The defaults are -inf for the lower bounds and +inf for the upper 
	 * bounds.
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
	void addArcWeights(host::ArcWeights& weights);

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
			const host::ArcSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	std::string toString(const MultiArcFactors::Arcs& arcs);

	const host::Graph& _graph;

	MultiArcFactors                                           _factors;
	std::map<MultiArcFactors::Arcs,std::pair<double,double>> _lambdas;

	// indicators for joint arc selection (one per factor)
	std::map<MultiArcFactors::Arcs,bool> _z;

	// the constant contribution of this term
	double _constant;
};

} // namespace host

#endif // HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__

