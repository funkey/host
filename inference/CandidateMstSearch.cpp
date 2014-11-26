#include <lemon/kruskal.h>
#include <util/Logger.h>
#include "CandidateMstSearch.h"
#include "ProximalBundleMethod.h"

logger::LogChannel cmstsearchlog("cmstsearchlog", "[CandidateMstSearch] ");

CandidateMstSearch::CandidateMstSearch(
		const host::Graph&     graph,
		const host::EdgeTypes& edgeTypes) :
	_graph(graph),
	_currentWeights(_graph) {

	findExclusiveEdges(edgeTypes);
}

bool
CandidateMstSearch::find(
		const host::EdgeWeights&   weights,
		host::EdgeSelection&       mst,
		double&                    value,
		unsigned int               maxIterations) {

	ValueGradientCallback valueGradientCallback(*this, weights, mst);

	ProximalBundleMethod<ValueGradientCallback> bundleMethod(
			_exclusiveEdges.size(), // one lambda per pair of exclusive edges
			maxIterations,
			valueGradientCallback);

	// all lambdas have to be positive
	for (unsigned int lambdaNum = 0; lambdaNum < _exclusiveEdges.size(); lambdaNum++)
		bundleMethod.setVariableBound(lambdaNum, 0, ProximalBundleMethod<ValueGradientCallback>::Infinity);

	bundleMethod.optimize();

	value = bundleMethod.getOptimalValue();

	LOG_ALL(cmstsearchlog)
			<< "final λ is : " << std::endl;
	for (const auto& exclusiveEdgesLambda : _exclusiveEdges)
		LOG_ALL(cmstsearchlog)
				<< "\t" << toString(std::get<0>(exclusiveEdgesLambda))
				<< ":\t" << std::get<1>(exclusiveEdgesLambda) << std::endl;
	LOG_ALL(cmstsearchlog) << std::endl;

	LOG_ALL(cmstsearchlog)
			<< "final weights are:" << std::endl;
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(cmstsearchlog)
				<< _graph.id(_graph.u(edge)) << " - "
				<< _graph.id(_graph.v(edge)) << ": "
				<< _currentWeights[edge] << std::endl;

	if (bundleMethod.getStatus() == ProximalBundleMethod<ValueGradientCallback>::ExactOptimiumFound)
		return true;

	return false;
}

void
CandidateMstSearch::findExclusiveEdges(const host::EdgeTypes& edgeTypes) {

	// for each conflict edge
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge) {

		if (edgeTypes[edge] != host::Conflict)
			continue;

		// for each source {link,conflict} edge
		for (host::Graph::IncEdgeIt source(_graph, _graph.u(edge)); source != lemon::INVALID; ++source) {

			if (static_cast<host::Edge>(source) == static_cast<host::Edge>(edge))
				continue;

			bool sourceIsLink = (edgeTypes[source] == host::Link);

			// for each target {link,conflict} edge
			for (host::Graph::IncEdgeIt target(_graph, _graph.v(edge)); target != lemon::INVALID; ++target) {

				if (source == target)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"conflict edge (" << _graph.id(_graph.u(edge)) << ", " << _graph.id(_graph.v(edge)) <<
							" has a parallel edge");

				if (static_cast<host::Edge>(target) == static_cast<host::Edge>(edge))
					continue;

				bool targetIsLink = (edgeTypes[target] == host::Link);

				int numAdjacentLinks = sourceIsLink + targetIsLink;

				// link, conflict, conflict
				// --> should not be selected at same time
				if (numAdjacentLinks == 1)
					_exclusiveEdges.push_back(std::make_tuple(ExclusiveEdges{source, edge, target}, 0));

				// link, conflict, link
				// --> the two links should not be selected at same time
				if (numAdjacentLinks == 2)
					_exclusiveEdges.push_back(std::make_tuple(ExclusiveEdges{source, target}, 0));
			}
		}
	}

	LOG_ALL(cmstsearchlog)
			<< "exclusive edges are:" << std::endl;
	for (const auto& exclusiveEdgesLambda : _exclusiveEdges)
		LOG_ALL(cmstsearchlog)
				<< "\t" << toString(std::get<0>(exclusiveEdgesLambda)) << std::endl;
	LOG_ALL(cmstsearchlog) << std::endl;
}

void
CandidateMstSearch::setLambdas(const std::vector<double>& x) {

	unsigned int pairNum = 0;

	LOG_ALL(cmstsearchlog)
			<< "λ set to :" << std::endl;

	for (auto& exclusiveEdgesLambda : _exclusiveEdges) {

		std::get<1>(exclusiveEdgesLambda) = x[pairNum];
		pairNum++;

		LOG_ALL(cmstsearchlog)
				<< "\t" << toString(std::get<0>(exclusiveEdgesLambda))
				<< ":\t" << std::get<1>(exclusiveEdgesLambda) << std::endl;
	}

	LOG_ALL(cmstsearchlog) << std::endl;
}

void
CandidateMstSearch::updateCurrentWeights(const host::EdgeWeights& originalWeights) {

	// set original weights
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		_currentWeights[edge] = originalWeights[edge];

	for (const auto& exclusiveEdgesLambda : _exclusiveEdges) {

		const ExclusiveEdges& edges  = std::get<0>(exclusiveEdgesLambda);
		double                lambda = std::get<1>(exclusiveEdgesLambda);

		for (const auto& edge : edges)
			_currentWeights[edge] += lambda;
	}

	LOG_ALL(cmstsearchlog) << "updated weights are:" << std::endl;
	for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(cmstsearchlog)
				<< "\t(" << _graph.id(_graph.u(edge)) << ", "
				<< _graph.id(_graph.v(edge)) << ")\t"
				<< _currentWeights[edge] << std::endl;
	LOG_ALL(cmstsearchlog) << std::endl;
}

double
CandidateMstSearch::getCurrentMst(host::EdgeSelection& currentMst) {

	double mstValue = lemon::kruskal(_graph, _currentWeights, currentMst);

	// to the mst value obtained above, we have to add a constant to compensate 
	// for the introduced lambdas
	for (auto exclusiveEdgesLambda : _exclusiveEdges)
		mstValue -= std::get<1>(exclusiveEdgesLambda);

	return mstValue;
}

void
CandidateMstSearch::getGradient(
		const host::EdgeSelection& currentMst,
		std::vector<double>&       gradient) {

	LOG_ALL(cmstsearchlog)
			<< "gradient is:" << std::endl;

	unsigned int pairNum = 0;
	for (const auto& exclusiveEdgesLambda : _exclusiveEdges) {

		const ExclusiveEdges& edges = std::get<0>(exclusiveEdgesLambda);

		int sum = 0;
		for (const auto& edge : edges)
			sum += currentMst[edge];

		gradient[pairNum] = sum - (static_cast<int>(edges.size()) - 1);

		LOG_ALL(cmstsearchlog) << "\t" << toString(edges) << ":\t" << gradient[pairNum] << std::endl;

		pairNum++;
	}

	LOG_ALL(cmstsearchlog) << std::endl;
}

void
CandidateMstSearch::ValueGradientCallback::operator()(
		const std::vector<double>& lambdas,
		double&                    value,
		std::vector<double>&       gradient) {

	_cmstSearch.setLambdas(lambdas);

	_cmstSearch.updateCurrentWeights(_originalWeights);

	value = _cmstSearch.getCurrentMst(_mst);

	_cmstSearch.getGradient(_mst, gradient);
}

std::string
CandidateMstSearch::toString(const ExclusiveEdges& edges) {

	std::stringstream ss;
	bool first = true;

	for (const auto& edge : edges) {

		if (!first)
			ss << "--";

		ss << "(" << _graph.id(_graph.u(edge)) << ", " << _graph.id(_graph.v(edge)) << ")";
		first = false;
	}

	return ss.str();
}
