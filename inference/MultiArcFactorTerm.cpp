#include "MultiArcFactorTerm.h"
#include <util/exceptions.h>
#include <util/Logger.h>

namespace host {

logger::LogChannel meflog("meflog", "[MultiArcFactorTerm] ");

size_t
MultiArcFactorTerm::numLambdas() {

	// two lambdas per factor
	return 2*_factors.size();
}

void
MultiArcFactorTerm::lambdaBounds(
		Lambdas::iterator beginLower,
		Lambdas::iterator endLower,
		Lambdas::iterator /*beginUpper*/,
		Lambdas::iterator /*endUpper*/) {

	// all our lambdas should be positive
	for (Lambdas::iterator i = beginLower; i != endLower; i++)
		*i = 0;
}

void
MultiArcFactorTerm::setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end) {

	Lambdas::const_iterator i = begin;

	_constant = 0;

	LOG_ALL(meflog) << "lambdas set to:" << std::endl;

	for (const auto& arcsFactor : _factors) {

		const MultiArcFactors::Arcs& arcs = arcsFactor.first;

		double lambda1 = *i; i++;
		double lambda2 = *i; i++;

		// set the lambdas

		_lambdas[arcs] = std::make_pair(lambda1, lambda2);

		// update z on the fly

		const double value = arcsFactor.second;

		// the cost w_f for this z_f
		double w = value + 2*lambda1 - lambda2;

		// z_f == 1 iff w_f < 0
		bool z = (w < 0);
		_z[arcs] = z;

		_constant += z*w - lambda2;

		LOG_ALL(meflog)
				<< "\t" << toString(arcs)
				<< ": λ¹ = " << lambda1
				<< ",\tλ² = " << lambda2
				<< ",\tz = " << z
				<< std::endl;
	}

	LOG_ALL(meflog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
MultiArcFactorTerm::addArcWeights(host::ArcWeights& weights) {

	for (const auto& arcsLambdas : _lambdas) {

		const MultiArcFactors::Arcs&   arcs   = arcsLambdas.first;
		const std::pair<double, double>& lambdas = arcsLambdas.second;

		for (auto& arc : arcs) {

			// lambda1 gets subtracted from the arc weights
			weights[arc] -= lambdas.first;

			// lambda2 gets added to the arc weights
			weights[arc] += lambdas.second;
		}
	}
}

double
MultiArcFactorTerm::constant() {

	return _constant;
}

void
MultiArcFactorTerm::gradient(
		const host::ArcSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          /*end*/) {

	Lambdas::iterator i = begin;

	LOG_ALL(meflog) << "gradient is:" << std::endl;

	for (const auto& arcsFactor : _factors) {

		const MultiArcFactors::Arcs& arcs = arcsFactor.first;

		int sumArcs = 0;
		for (const auto& arc : arcs)
			sumArcs += mst[arc];

		double gradient1 = 2*_z[arcs] - sumArcs;
		double gradient2 = sumArcs - _z[arcs] - 1;

		// store the gradients in the same order we retrieved the lambdas
		*i = gradient1; i++;
		*i = gradient2; i++;

		LOG_ALL(meflog)
				<< "\t" << toString(arcs)
				<< ": δλ¹ = " << gradient1
				<< ",\tδλ² = " << gradient2
				<< std::endl;
	}

	LOG_ALL(meflog) << std::endl;
}

std::string
MultiArcFactorTerm::toString(const MultiArcFactors::Arcs& arcs) {

	std::stringstream ss;
	bool first = true;

	for (const auto& arc : arcs) {

		if (!first)
			ss << "--";

		ss << "(" << _graph.id(_graph.source(arc)) << ", " << _graph.id(_graph.target(arc)) << ")";
		first = false;
	}

	return ss.str();
}

} // namespace host
