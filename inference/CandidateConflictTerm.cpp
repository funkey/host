#include <util/Logger.h>
#include <util/exceptions.h>
#include "CandidateConflictTerm.h"

namespace host {

logger::LogChannel cctlog("cctlog", "[CandidateConflictTerm] ");

CandidateConflictTerm::CandidateConflictTerm(
		const Graph& graph,
		const ArcTypes& arcTypes) :
	_graph(graph) {

	findExclusiveArcs(arcTypes);
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

	for (auto& exclusiveArcsLambda : _exclusiveArcs) {

		std::get<1>(exclusiveArcsLambda) = *i;
		i++;

		LOG_ALL(cctlog)
				<< "\t" << toString(std::get<0>(exclusiveArcsLambda))
				<< ":\t" << std::get<1>(exclusiveArcsLambda) << std::endl;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::addArcWeights(ArcWeights& weights) {

	for (const auto& exclusiveArcsLambda : _exclusiveArcs) {

		const ExclusiveArcs& arcs   = std::get<0>(exclusiveArcsLambda);
		double               lambda = std::get<1>(exclusiveArcsLambda);

		for (const auto& arc : arcs)
			weights[arc] += lambda;
	}

	LOG_ALL(cctlog) << "updated weights are:" << std::endl;
	for (Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		LOG_ALL(cctlog)
				<< "\t(" << _graph.id(_graph.source(arc)) << ", "
				<< _graph.id(_graph.target(arc)) << ")\t"
				<< weights[arc] << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

double
CandidateConflictTerm::constant() {

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
CandidateConflictTerm::gradient(
		const ArcSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          end) {

	Lambdas::iterator i = begin;

	LOG_ALL(cctlog) << "gradient is:" << std::endl;

	for (const auto& exclusiveArcsLambda : _exclusiveArcs) {

		const ExclusiveArcs& arcs = std::get<0>(exclusiveArcsLambda);

		int sum = 0;
		for (const auto& arc : arcs)
			sum += mst[arc];

		*i = sum - (static_cast<int>(arcs.size()) - 1);

		LOG_ALL(cctlog) << "\t" << toString(arcs) << ":\t" << *i << std::endl;

		i++;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::findExclusiveArcs(const ArcTypes& arcTypes) {

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

	LOG_ALL(cctlog)
			<< "exclusive arcs are:" << std::endl;
	for (const auto& exclusiveArcsLambda : _exclusiveArcs)
		LOG_ALL(cctlog)
				<< "\t" << toString(std::get<0>(exclusiveArcsLambda)) << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

std::string
CandidateConflictTerm::toString(const ExclusiveArcs& arcs) {

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
