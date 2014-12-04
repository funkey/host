#include <lemon/kruskal.h>
#include <util/Logger.h>
#include "LeafConstraintTerm.h"
#include "ProximalBundleMethod.h"

logger::LogChannel lcmstsearchlog("lcmstsearchlog", "[LeafConstrainedMstSearch] ");

bool
LeafConstrainedMstSearch::find(
		const host::ArcWeights&   weights,
		const host::NodeSelection& leaves,
		host::ArcSelection&       mst,
		double&                    value,
		unsigned int               maxIterations) {

	// TODO: inform proximal bundle mehthod, that some of the lambdas have to 
	// stay positive

	ValueGradientCallback valueGradientCallback(*this, weights, leaves, mst);

	ProximalBundleMethod<ValueGradientCallback> bundleMethod(
			lemon::countNodes(_graph),
			maxIterations,
			valueGradientCallback);

	unsigned int nodeNum = 0;
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		if (!leaves[node])
			bundleMethod.setVariableBound(nodeNum, 0, ProximalBundleMethod<ValueGradientCallback>::Infinity);

		nodeNum++;
	}

	//bundleMethod.optimize();

	value = bundleMethod.getOptimalValue();

	LOG_ALL(lcmstsearchlog)
			<< "final λ is : ";
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node)
		LOG_ALL(lcmstsearchlog) << _graph.id(node) << ": " << _lambdas[node] << ", ";
	LOG_ALL(lcmstsearchlog) << std::endl;

	LOG_ALL(lcmstsearchlog)
			<< "final weights are:" << std::endl;
	for (host::Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		LOG_ALL(lcmstsearchlog)
				<< _graph.id(_graph.source(arc)) << " - "
				<< _graph.id(_graph.target(arc)) << ": "
				<< _currentWeights[arc] << std::endl;

	if (bundleMethod.getStatus() == ProximalBundleMethod<ValueGradientCallback>::ExactOptimiumFound)
		return true;

	return false;
}

void
LeafConstrainedMstSearch::setLambdas(const std::vector<double>& x) {

	unsigned int nodeNum = 0;

	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		_lambdas[node] = x[nodeNum];
		nodeNum++;
	}
}

void
LeafConstrainedMstSearch::updateCurrentWeights(const host::ArcWeights& originalWeights) {

	for (host::Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		_currentWeights[arc] = originalWeights[arc] - _lambdas[_graph.source(arc)] - _lambdas[_graph.target(arc)];
}

double
LeafConstrainedMstSearch::getCurrentMst(
		const host::NodeSelection& leaves,
		host::ArcSelection&       currentMst) {

	double mstValue = lemon::kruskal(_graph, _currentWeights, currentMst);

	// to the mst value obtained above, we have to add a constant to compensate 
	// for the introduced lambdas
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		if (leaves[node])
			mstValue += _lambdas[node];
		else
			mstValue += 2*_lambdas[node];
	}

	return mstValue;
}

void
LeafConstrainedMstSearch::getGradient(
		const host::ArcSelection& currentMst,
		const host::NodeSelection& leaves,
		std::vector<double>&       gradient) {

	unsigned int nodeNum = 0;
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		gradient[nodeNum] = 0.0;
		unsigned int degree = 0;

		// subtract one for every arc involving this arc
		for (host::Graph::OutArcIt arc(_graph, node); arc != lemon::INVALID; ++arc) {

			if (currentMst[arc])
				degree++;
		}
		gradient[nodeNum] -= degree;

		// add one for every requested leaf node, two for all others
		if (leaves[node])
			gradient[nodeNum] += 1;
		else
			gradient[nodeNum] += 2;

		nodeNum++;
	}
}

void
LeafConstrainedMstSearch::ValueGradientCallback::operator()(
		const std::vector<double>& lambdas,
		double&                    value,
		std::vector<double>&       gradient) {

	_lcmstSearch.setLambdas(lambdas);

	_lcmstSearch.updateCurrentWeights(_originalWeights);

	value = _lcmstSearch.getCurrentMst(_leaves, _mst);

	_lcmstSearch.getGradient(_mst, _leaves, gradient);
}
