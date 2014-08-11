#include <lemon/kruskal.h>
#include <util/Logger.h>
#include "LeafConstrainedMstSearch.h"

logger::LogChannel lcmstsearchlog("lcmstsearchlog", "[LeafConstrainedMstSearch] ");

void
LeafConstrainedMstSearch::find(
		const host::Graph&         graph,
		const host::EdgeWeights&   weights,
		const host::NodeSelection& leaves,
		host::EdgeSelection&       mst) {

	host::NodeWeights   lambdas(graph);
	host::EdgeWeights   currentWeights(graph);

	bool noViolation = false;

	for (unsigned int i = 0; i < _maxIterations; i++) {

		updateCurrentWeights(graph, weights, lambdas, currentWeights);

		getCurrentMst(graph, currentWeights, mst);

		noViolation = updateLambdas(graph, mst, leaves, lambdas);

		if (noViolation) {

			LOG_USER(lcmstsearchlog)
					<< "minimal spanning tree with requested leaf nodes found"
					<< std::endl;
			break;
		}
	}

	if (!noViolation)
		LOG_USER(lcmstsearchlog)
				<< "exceeded maximal number of iterations ("
				<< _maxIterations << ") -- aborting search"
				<< std::endl;

	LOG_ALL(lcmstsearchlog)
			<< "final λ is : ";
	for (host::Graph::NodeIt node(graph); node != lemon::INVALID; ++node)
		LOG_ALL(lcmstsearchlog) << graph.id(node) << ": " << lambdas[node] << ", ";
	LOG_ALL(lcmstsearchlog) << std::endl;

	LOG_ALL(lcmstsearchlog)
			<< "final weights are:" << std::endl;
	for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(lcmstsearchlog)
				<< graph.id(graph.u(edge)) << " - "
				<< graph.id(graph.v(edge)) << ": "
				<< currentWeights[edge] << std::endl;
}

void
LeafConstrainedMstSearch::updateCurrentWeights(
		const host::Graph&       graph,
		const host::EdgeWeights& weights,
		const host::NodeWeights& lambdas,
		host::EdgeWeights& currentWeights) {

	for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
		currentWeights[edge] = weights[edge] - lambdas[graph.u(edge)] - lambdas[graph.v(edge)];
}

void
LeafConstrainedMstSearch::getCurrentMst(
		const host::Graph&       graph,
		const host::EdgeWeights& weights,
		host::EdgeSelection&     currentMst) {

	lemon::kruskal(graph, weights, currentMst);
}

bool
LeafConstrainedMstSearch::updateLambdas(
		const host::Graph&         graph,
		const host::EdgeSelection& currentMst,
		const host::NodeSelection& leaves,
		host::NodeWeights&         lambdas) {

	LOG_ALL(lcmstsearchlog) << "updating lambdas" << std::endl;

	bool constraintsViolated = false;

	// for each node

	for (host::Graph::NodeIt node(graph); node != lemon::INVALID; ++node) {

		double gradient = 0.0;
		unsigned int degree = 0;

		// subtract one for every edge involving this edge
		for (host::Graph::IncEdgeIt edge(graph, node); edge != lemon::INVALID; ++edge) {

			if (currentMst[edge])
				degree++;
		}
		gradient -= degree;

		// add one for every requested leaf node, two for all others
		if (leaves[node])
			gradient += 1;
		else
			gradient += 2;

		// move in the direction of the gradient
		lambdas[node] += _stepSize*gradient;

		// non-leaf lambdas have to stay positive
		if (!leaves[node])
			lambdas[node] = std::max(0.0, lambdas[node]);

		// check, whether this node violates the constraints
		if ((leaves[node] && degree > 1) || (!leaves[node] && degree == 1))
			constraintsViolated = true;

		LOG_ALL(lcmstsearchlog)
				<< "degree of node " << graph.id(node) << " is " << degree << std::endl;
	}

	// decrease the step size
	_stepSize *= _decceleration;

	// are we done?
	return !constraintsViolated;
}
