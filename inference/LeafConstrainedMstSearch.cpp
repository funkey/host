#include <lemon/kruskal.h>
#include <util/Logger.h>
#include "LeafConstrainedMstSearch.h"
#include "ProximalBundleMethod.h"

logger::LogChannel lcmstsearchlog("lcmstsearchlog", "[LeafConstrainedMstSearch] ");

bool
LeafConstrainedMstSearch::find(
		const host::EdgeWeights&   weights,
		const host::NodeSelection& leaves,
		host::EdgeSelection&       mst,
		unsigned int               maxIterations) {

	// TODO: inform proximal bundle mehthod, that some of the lambdas have to 
	// stay positive

	ValueGradientCallback valueGradientCallback(*this, weights, leaves, mst);

	ProximalBundleMethod<ValueGradientCallback> bundleMethod(
			lemon::countNodes(_graph),
			maxIterations,
			valueGradientCallback);

	bundleMethod.optimize();

	LOG_ALL(lcmstsearchlog)
			<< "final λ is : ";
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node)
		LOG_ALL(lcmstsearchlog) << _graph.id(node) << ": " << _lambdas[node] << ", ";
	LOG_ALL(lcmstsearchlog) << std::endl;

	LOG_ALL(lcmstsearchlog)
			<< "final weights are:" << std::endl;
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(lcmstsearchlog)
				<< _graph.id(_graph.u(edge)) << " - "
				<< _graph.id(_graph.v(edge)) << ": "
				<< _currentWeights[edge] << std::endl;

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
LeafConstrainedMstSearch::updateCurrentWeights(const host::EdgeWeights& originalWeights) {

	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		_currentWeights[edge] = originalWeights[edge] - _lambdas[_graph.u(edge)] - _lambdas[_graph.v(edge)];
}

double
LeafConstrainedMstSearch::getCurrentMst(host::EdgeSelection& currentMst) {

	return lemon::kruskal(_graph, _currentWeights, currentMst);
}

void
LeafConstrainedMstSearch::getGradient(
		const host::EdgeSelection& currentMst,
		const host::NodeSelection& leaves,
		std::vector<double>&       gradient) {

	unsigned int nodeNum = 0;
	for (host::Graph::NodeIt node(_graph); node != lemon::INVALID; ++node) {

		gradient[nodeNum] = 0.0;
		unsigned int degree = 0;

		// subtract one for every edge involving this edge
		for (host::Graph::IncEdgeIt edge(_graph, node); edge != lemon::INVALID; ++edge) {

			if (currentMst[edge])
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

	value = _lcmstSearch.getCurrentMst(_mst);

	_lcmstSearch.getGradient(_mst, _leaves, gradient);
}
