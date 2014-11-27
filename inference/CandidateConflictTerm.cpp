#include <util/Logger.h>
#include <util/exceptions.h>
#include "CandidateConflictTerm.h"

namespace host {

logger::LogChannel cctlog("cctlog", "[CandidateConflictTerm] ");

CandidateConflictTerm::CandidateConflictTerm(
		const Graph& graph,
		const EdgeTypes& edgeTypes) :
	_graph(graph) {

	findExclusiveEdges(edgeTypes);
}

void
CandidateConflictTerm::lambdaBounds(
		Lambdas::iterator beginLower,
		Lambdas::iterator endLower,
		Lambdas::iterator,
		Lambdas::iterator) {

	// all lambdas have to be positive
	for (Lambdas::iterator i = beginLower; i != endLower; i++)
		*i = 0;
}

void
CandidateConflictTerm::setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end) {

	Lambdas::const_iterator i = begin;

	LOG_ALL(cctlog) << "λ set to :" << std::endl;

	for (auto& exclusiveEdgesLambda : _exclusiveEdges) {

		std::get<1>(exclusiveEdgesLambda) = *i;
		i++;

		LOG_ALL(cctlog)
				<< "\t" << toString(std::get<0>(exclusiveEdgesLambda))
				<< ":\t" << std::get<1>(exclusiveEdgesLambda) << std::endl;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::addEdgeWeights(EdgeWeights& weights) {

	for (const auto& exclusiveEdgesLambda : _exclusiveEdges) {

		const ExclusiveEdges& edges  = std::get<0>(exclusiveEdgesLambda);
		double                lambda = std::get<1>(exclusiveEdgesLambda);

		for (const auto& edge : edges)
			weights[edge] += lambda;
	}

	LOG_ALL(cctlog) << "updated weights are:" << std::endl;
	for (Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
		LOG_ALL(cctlog)
				<< "\t(" << _graph.id(_graph.u(edge)) << ", "
				<< _graph.id(_graph.v(edge)) << ")\t"
				<< weights[edge] << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

double
CandidateConflictTerm::constant() {

	double constant = 0;

	// we have to add a constant to compensate for the introduced lambdas
	for (auto exclusiveEdgesLambda : _exclusiveEdges) {

		const ExclusiveEdges& edges  = std::get<0>(exclusiveEdgesLambda);
		double                lambda = std::get<1>(exclusiveEdgesLambda);

		constant -= (static_cast<int>(edges.size()) - 1)*lambda;
	}

	return constant;
}

void
CandidateConflictTerm::gradient(
		const EdgeSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          end) {

	Lambdas::iterator i = begin;

	LOG_ALL(cctlog) << "gradient is:" << std::endl;

	for (const auto& exclusiveEdgesLambda : _exclusiveEdges) {

		const ExclusiveEdges& edges = std::get<0>(exclusiveEdgesLambda);

		int sum = 0;
		for (const auto& edge : edges)
			sum += mst[edge];

		*i = sum - (static_cast<int>(edges.size()) - 1);

		LOG_ALL(cctlog) << "\t" << toString(edges) << ":\t" << *i << std::endl;

		i++;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::findExclusiveEdges(const EdgeTypes& edgeTypes) {

	// for each conflict edge
	for (Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge) {

		if (edgeTypes[edge] != Conflict)
			continue;

		// for each source {link,conflict} edge
		for (Graph::IncEdgeIt source(_graph, _graph.u(edge)); source != lemon::INVALID; ++source) {

			if (static_cast<Edge>(source) == static_cast<Edge>(edge))
				continue;

			bool sourceIsLink = (edgeTypes[source] == Link);

			// for each target {link,conflict} edge
			for (Graph::IncEdgeIt target(_graph, _graph.v(edge)); target != lemon::INVALID; ++target) {

				if (source == target)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"conflict edge (" << _graph.id(_graph.u(edge)) << ", " << _graph.id(_graph.v(edge)) <<
							" has a parallel edge");

				if (static_cast<Edge>(target) == static_cast<Edge>(edge))
					continue;

				bool targetIsLink = (edgeTypes[target] == Link);

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

	LOG_ALL(cctlog)
			<< "exclusive edges are:" << std::endl;
	for (const auto& exclusiveEdgesLambda : _exclusiveEdges)
		LOG_ALL(cctlog)
				<< "\t" << toString(std::get<0>(exclusiveEdgesLambda)) << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

std::string
CandidateConflictTerm::toString(const ExclusiveEdges& edges) {

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

} // namespace host
