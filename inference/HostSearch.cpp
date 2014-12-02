#include <lemon/min_cost_arborescence.h>
#include <util/Logger.h>
#include "ProximalBundleMethod.h"
#include "HostSearch.h"

logger::LogChannel hostsearchlog("hostsearchlog", "[HostSearch] ");

void
HostSearch::addTerm(ArcTerm* term) {

	_arcTerms.push_back(term);

	if (HigherOrderArcTerm* higherOrderTerm = dynamic_cast<HigherOrderArcTerm*>(term))
		_higherOrderArcTerms.push_back(higherOrderTerm);
}

bool
HostSearch::find(
		host::ArcSelection& mst,
		double&              value,
		unsigned int         maxIterations) {

	ValueGradientCallback valueGradientCallback(*this, mst);

	ProximalBundleMethod<ValueGradientCallback> bundleMethod(
			numLambdas(),
			maxIterations,
			valueGradientCallback);

	Lambdas lowerBounds(numLambdas(), -ProximalBundleMethod<ValueGradientCallback>::Infinity);
	Lambdas upperBounds(numLambdas(),  ProximalBundleMethod<ValueGradientCallback>::Infinity);
	Lambdas::iterator li = lowerBounds.begin();
	Lambdas::iterator ui = upperBounds.begin();

	for (auto* term : _higherOrderArcTerms) {

		term->lambdaBounds(li, li + term->numLambdas(), ui, ui + term->numLambdas());

		li += term->numLambdas();
		ui += term->numLambdas();
	}

	for (unsigned int lambdaNum = 0; lambdaNum < numLambdas(); lambdaNum++)
		bundleMethod.setVariableBound(lambdaNum, lowerBounds[lambdaNum], upperBounds[lambdaNum]);

	bundleMethod.optimize();

	value = bundleMethod.getOptimalValue();

	LOG_ALL(hostsearchlog)
			<< "final weights are:" << std::endl;
	for (host::Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		LOG_ALL(hostsearchlog)
				<< _graph.id(_graph.source(arc)) << " - "
				<< _graph.id(_graph.target(arc)) << ": "
				<< _currentWeights[arc] << std::endl;

	if (bundleMethod.getStatus() == ProximalBundleMethod<ValueGradientCallback>::ExactOptimiumFound)
		return true;

	return false;
}

void
HostSearch::ValueGradientCallback::operator()(
		const Lambdas& lambdas,
		double&        value,
		Lambdas&       gradient) {

	_hostSearch.setLambdas(lambdas);

	_hostSearch.updateWeights();

	value = _hostSearch.mst(_mst);

	_hostSearch.gradient(_mst, gradient);
}

size_t
HostSearch::numLambdas() {

	size_t numLambdas = 0;

	for (auto* term : _higherOrderArcTerms)
		numLambdas += term->numLambdas();

	return numLambdas;
}

void
HostSearch::setLambdas(const Lambdas& x) {

	Lambdas::const_iterator i = x.begin();

	for (auto* term : _higherOrderArcTerms) {

		term->setLambdas(i, i + term->numLambdas());
		i += term->numLambdas();
	}
}

void
HostSearch::updateWeights() {

	// set weights to zero
	for (host::Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		_currentWeights[arc] = 0;

	for (auto* term : _arcTerms)
		term->addArcWeights(_currentWeights);
}

double
HostSearch::mst(host::ArcSelection& currentMst) {

	double mstValue = lemon::minCostArborescence(_graph, _currentWeights, host::Graph::NodeIt(_graph), currentMst);

	// to the mst value obtained above, we have to add a constant for each 
	// higher order term
	for (auto* term : _higherOrderArcTerms)
		mstValue += term->constant();

	return mstValue;
}

void
HostSearch::gradient(
			const host::ArcSelection& mst,
			Lambdas&                   gradient) {

	Lambdas::iterator i = gradient.begin();

	for (auto* term : _higherOrderArcTerms) {

		term->gradient(mst, i, i + term->numLambdas());
		i += term->numLambdas();
	}
}
