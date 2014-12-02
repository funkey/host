#include <util/Logger.h>
#include <util/exceptions.h>
#include "UndirectedCandidateConflictTerm.h"

namespace host {

logger::LogChannel ucctlog("ucctlog", "[UndirectedCandidateConflictTerm] ");

UndirectedCandidateConflictTerm::UndirectedCandidateConflictTerm(
		const Graph& graph,
		const ArcTypes& arcTypes) :
	_graph(graph) {

	findExclusiveArcs(arcTypes);
}

void
UndirectedCandidateConflictTerm::lambdaBounds(
		Lambdas::iterator beginLower,
		Lambdas::iterator endLower,
		Lambdas::iterator,
		Lambdas::iterator) {

	// all lambdas have to be positive
	for (Lambdas::iterator i = beginLower; i != endLower; i++)
		*i = 0;
}

void
UndirectedCandidateConflictTerm::setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end) {

	Lambdas::const_iterator i = begin;

	LOG_ALL(ucctlog) << "λ set to :" << std::endl;

	for (auto& exclusiveArcsLambda : _exclusiveArcs) {

		std::get<1>(exclusiveArcsLambda) = *i;
		i++;

		LOG_ALL(ucctlog)
				<< "\t" << toString(std::get<0>(exclusiveArcsLambda))
				<< ":\t" << std::get<1>(exclusiveArcsLambda) << std::endl;
	}

	LOG_ALL(ucctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
UndirectedCandidateConflictTerm::addArcWeights(ArcWeights& weights) {

	for (const auto& exclusiveArcsLambda : _exclusiveArcs) {

		const ExclusiveArcs& arcs   = std::get<0>(exclusiveArcsLambda);
		double               lambda = std::get<1>(exclusiveArcsLambda);

		for (const auto& arc : arcs)
			weights[arc] += lambda;
	}

	LOG_ALL(ucctlog) << "updated weights are:" << std::endl;
	for (Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		LOG_ALL(ucctlog)
				<< "\t(" << _graph.id(_graph.source(arc)) << ", "
				<< _graph.id(_graph.target(arc)) << ")\t"
				<< weights[arc] << std::endl;
	LOG_ALL(ucctlog) << std::endl;
}

double
UndirectedCandidateConflictTerm::constant() {

	double constant = 0;

	// we have to add a constant to compensate for the introduced lambdas
	for (auto exclusiveArcsLambda : _exclusiveArcs) {

		const ExclusiveArcs& arcs  = std::get<0>(exclusiveArcsLambda);
		double                lambda = std::get<1>(exclusiveArcsLambda);

		constant -= (static_cast<int>(arcs.size()) - 1)*lambda;
	}

	return constant;
}

void
UndirectedCandidateConflictTerm::gradient(
		const ArcSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          end) {

	Lambdas::iterator i = begin;

	LOG_ALL(ucctlog) << "gradient is:" << std::endl;

	for (const auto& exclusiveArcsLambda : _exclusiveArcs) {

		const ExclusiveArcs& arcs = std::get<0>(exclusiveArcsLambda);

		int sum = 0;
		for (const auto& arc : arcs)
			sum += mst[arc];

		*i = sum - (static_cast<int>(arcs.size()) - 1);

		LOG_ALL(ucctlog) << "\t" << toString(arcs) << ":\t" << *i << std::endl;

		i++;
	}

	LOG_ALL(ucctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
UndirectedCandidateConflictTerm::findExclusiveArcs(const ArcTypes& arcTypes) {

	// for each conflict arc
	for (Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc) {

		if (arcTypes[arc] != Conflict)
			continue;

		// for each source {link,conflict} arc
		for (Graph::OutArcIt source(_graph, _graph.source(arc)); source != lemon::INVALID; ++source) {

			if (static_cast<Arc>(source) == static_cast<Arc>(arc))
				continue;

			bool sourceIsLink = (arcTypes[source] == Link);

			// for each target {link,conflict} arc
			for (Graph::OutArcIt target(_graph, _graph.target(arc)); target != lemon::INVALID; ++target) {

				if (source == target)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"conflict arc (" << _graph.id(_graph.source(arc)) << ", " << _graph.id(_graph.target(arc)) <<
							" has a parallel arc");

				if (static_cast<Arc>(target) == static_cast<Arc>(arc))
					continue;

				bool targetIsLink = (arcTypes[target] == Link);

				int numAdjacentLinks = sourceIsLink + targetIsLink;

				// link, conflict, conflict
				// --> should not be selected at same time
				if (numAdjacentLinks == 1)
					_exclusiveArcs.push_back(std::make_tuple(ExclusiveArcs{source, arc, target}, 0));

				// link, conflict, link
				// --> the two links should not be selected at same time
				if (numAdjacentLinks == 2)
					_exclusiveArcs.push_back(std::make_tuple(ExclusiveArcs{source, target}, 0));
			}
		}
	}

	LOG_ALL(ucctlog)
			<< "exclusive arcs are:" << std::endl;
	for (const auto& exclusiveArcsLambda : _exclusiveArcs)
		LOG_ALL(ucctlog)
				<< "\t" << toString(std::get<0>(exclusiveArcsLambda)) << std::endl;
	LOG_ALL(ucctlog) << std::endl;
}

std::string
UndirectedCandidateConflictTerm::toString(const ExclusiveArcs& arcs) {

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
