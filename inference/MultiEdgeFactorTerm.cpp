#include "MultiEdgeFactorTerm.h"
#include <util/exceptions.h>

size_t
MultiEdgeFactorTerm::numLambdas() {

	// two lambdas per factor
	return 2*_factors.size();
}

void
MultiEdgeFactorTerm::lambdaBounds(
		Lambdas::iterator beginLower,
		Lambdas::iterator endLower,
		Lambdas::iterator /*beginUpper*/,
		Lambdas::iterator /*endUpper*/) {

	// all our lambdas should be positive
	for (Lambdas::iterator i = beginLower; i != endLower; i++)
		*i = 0;
}

void
MultiEdgeFactorTerm::setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end) {

	Lambdas::const_iterator i = begin;

	_constant = 0;

	for (const auto& edgesFactor : _factors) {

		const std::vector<host::Edge>& edges = edgesFactor.first;

		double lambda1 = *i; i++;
		double lambda2 = *i; i++;

		// set the lambdas

		_lambdas[edges] = std::make_pair(lambda1, lambda2);

		// update z on the fly

		const double value = edgesFactor.second;

		// the cost w_f for this z_f
		double w = value + 2*lambda1 - lambda2;

		// z_f == 1 iff w_f < 0
		_z[edges] = (w <= 0);

		_constant += (w <= 0)*w - lambda2;
	}

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
MultiEdgeFactorTerm::addEdgeWeights(host::EdgeWeights& weights) {

	for (const auto& edgesLambdas : _lambdas) {

		const std::vector<host::Edge>&  edges   = edgesLambdas.first;
		const std::pair<double, double>& lambdas = edgesLambdas.second;

		for (auto& edge : edges) {

			// lambda1 gets subtracted from the edge weights
			weights[edge] -= lambdas.first;

			// lambda2 gets added to the edge weights
			weights[edge] += lambdas.second;
		}
	}
}

double
MultiEdgeFactorTerm::constant() {

	return _constant;
}

void
MultiEdgeFactorTerm::gradient(
		const host::EdgeSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          /*end*/) {

	Lambdas::iterator i = begin;

	for (const auto& edgesFactor : _factors) {

		const std::vector<host::Edge>& edges = edgesFactor.first;

		int sumEdges = 0;
		for (const auto& edge : edges)
			sumEdges += mst[edge];

		double gradient1 = 2*_z[edges] - sumEdges;
		double gradient2 = sumEdges - _z[edges] - 1;

		// store the gradients in the same order we retrieved the lambdas
		*i = gradient1; i++;
		*i = gradient2; i++;
	}
}
