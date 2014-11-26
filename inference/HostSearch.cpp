#include <lemon/kruskal.h>
#include <util/Logger.h>
#include "ProximalBundleMethod.h"
#include "HostSearch.h"

logger::LogChannel hostsearchlog("hostsearchlog", "[HostSearch] ");

void
HostSearch::addTerm(EdgeTerm* term) {

	_edgeTerms.push_back(term);

	if (HigherOrderEdgeTerm* higherOrderTerm = dynamic_cast<HigherOrderEdgeTerm*>(term))
		_higherOrderEdgeTerms.push_back(higherOrderTerm);
}

bool
HostSearch::find(
		host::EdgeSelection& mst,
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

	for (auto* term : _higherOrderEdgeTerms) {

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
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(hostsearchlog)
				<< _graph.id(_graph.u(edge)) << " - "
				<< _graph.id(_graph.v(edge)) << ": "
				<< _currentWeights[edge] << std::endl;

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

	for (auto* term : _higherOrderEdgeTerms)
		numLambdas += term->numLambdas();

	return numLambdas;
}

void
HostSearch::setLambdas(const Lambdas& x) {

	Lambdas::const_iterator i = x.begin();

	for (auto* term : _higherOrderEdgeTerms) {

		term->setLambdas(i, i + term->numLambdas());
		i += term->numLambdas();
	}
}

void
HostSearch::updateWeights() {

	// set weights to zero
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		_currentWeights[edge] = 0;

	for (auto* term : _edgeTerms)
		term->addEdgeWeights(_currentWeights);
}

double
HostSearch::mst(host::EdgeSelection& currentMst) {

	double mstValue = lemon::kruskal(_graph, _currentWeights, currentMst);

	// to the mst value obtained above, we have to add a constant for each 
	// higher order term
	for (auto* term : _higherOrderEdgeTerms)
		mstValue += term->constant();

	return mstValue;
}

void
HostSearch::gradient(
			const host::EdgeSelection& mst,
			Lambdas&                   gradient) {

	Lambdas::iterator i = gradient.begin();

	for (auto* term : _higherOrderEdgeTerms) {

		term->gradient(mst, i, i + term->numLambdas());
		i += term->numLambdas();
	}
}
