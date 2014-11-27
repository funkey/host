#ifndef HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__
#define HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__

#include <map>
#include <vector>
#include "HigherOrderEdgeTerm.h"

class MultiEdgeFactorTerm : public HigherOrderEdgeTerm {

public:

	MultiEdgeFactorTerm(
			const host::Graph&                        graph,
			std::map<std::vector<host::Edge>, double> factors) :
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

	const host::Graph& _graph;

	std::map<std::vector<host::Edge>,double>                   _factors;
	std::map<std::vector<host::Edge>,std::pair<double,double>> _lambdas;

	// indicators for joint edge selection (one per factor)
	std::map<std::vector<host::Edge>,bool> _z;

	// the constant contribution of this term
	double _constant;
};

#endif // HOST_INFERENCE_MULTI_EDGE_FACTOR_TERM_H__

