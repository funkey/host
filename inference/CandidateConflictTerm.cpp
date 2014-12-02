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
	findArcLambdas(arcTypes);
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

	for (auto& exclusive : _exclusiveArcs) {

		exclusive.lambda = *i; i++;

		LOG_ALL(cctlog)
				<< "\t" << toString(exclusive.arcs)
				<< ":\t" << exclusive.lambda
				<< std::endl;
	}

	for (auto& conflict : _conflictArcs) {

		conflict.lambda = *i; i++;

		LOG_ALL(cctlog)
				<< "\t" << toString(conflict.arc)
				<< ": " << toString(conflict.conflictArcs)
				<< ":\t" << conflict.lambda
				<< std::endl;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::addArcWeights(ArcWeights& weights) {

	for (const auto& exclusive : _exclusiveArcs)
		for (const auto& arc : exclusive.arcs)
			weights[arc] += exclusive.lambda;

	for (const auto& conflict : _conflictArcs) {

		const Arc&   arc    = conflict.arc;
		unsigned int n      = conflict.conflictArcs.size();
		double       lambda = conflict.lambda;

		weights[arc] += n*lambda;
		for (const auto& conflictArc : conflict.conflictArcs)
			weights[conflictArc] += lambda;
	}

	LOG_ALL(cctlog) << "updated weights are:" << std::endl;
	for (ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		LOG_ALL(cctlog)
				<< "\t(" << _graph.id(_graph.source(arc)) << ", "
				<< _graph.id(_graph.target(arc)) << ")\t"
				<< weights[arc] << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

double
CandidateConflictTerm::constant() {

	double constant = 0;

	for (auto exclusive : _exclusiveArcs)
		constant -= (static_cast<int>(exclusive.arcs.size()) - 1)*exclusive.lambda;

	for (const auto& conflict : _conflictArcs)
		constant -= conflict.conflictArcs.size()*conflict.lambda;

	return constant;
}

void
CandidateConflictTerm::gradient(
		const ArcSelection& mst,
		Lambdas::iterator          begin,
		Lambdas::iterator          end) {

	Lambdas::iterator i = begin;

	LOG_ALL(cctlog) << "gradient is:" << std::endl;

	for (const auto& conflict : _conflictArcs) {

		const Arc&   arc = conflict.arc;
		unsigned int n   = conflict.conflictArcs.size();

		int sum = 0;
		for (const auto& conflictArc : conflict.conflictArcs)
			sum += mst[conflictArc];

		*i = (static_cast<double>(mst[arc]) - 1)*n + sum;

		LOG_ALL(cctlog)
				<< "\t" << toString(arc)
				<< ": " << toString(conflict.conflictArcs)
				<< ":\t" << *i
				<< std::endl;
		i++;
	}

	for (const auto& exclusive : _exclusiveArcs) {

		int sum = 0;
		for (const auto& arc : exclusive.arcs)
			sum += mst[arc];

		*i = sum - (static_cast<int>(exclusive.arcs.size()) - 1);

		LOG_ALL(cctlog) << "\t" << toString(exclusive.arcs) << ":\t" << *i << std::endl;

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

		// for each incoming source link arc
		for (Graph::InArcIt source(_graph, _graph.source(arc)); source != lemon::INVALID; ++source) {

			if (static_cast<Arc>(source) == static_cast<Arc>(arc))
				continue;

			if (arcTypes[source] != Link)
				continue;

			// for each incoming target link arc
			for (Graph::InArcIt target(_graph, _graph.target(arc)); target != lemon::INVALID; ++target) {

				if (source == target)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"conflict arc (" << _graph.id(_graph.source(arc)) << ", " << _graph.id(_graph.target(arc)) <<
							" has a parallel arc");

				if (static_cast<Arc>(target) == static_cast<Arc>(arc))
					continue;

				if (arcTypes[target] != Link)
					continue;

				_exclusiveArcs.push_back(ExclusiveArcsLambda{{source,target},0});
			}
		}
	}

	LOG_ALL(cctlog)
			<< "exclusive arcs are:" << std::endl;
	for (const auto& exclusive : _exclusiveArcs)
		LOG_ALL(cctlog)
				<< "\t" << toString(exclusive.arcs) << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

void
CandidateConflictTerm::findArcLambdas(const ArcTypes& arcTypes) {

	// for each conflict arc
	for (Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc) {

		if (arcTypes[arc] != Conflict)
			continue;

		// get all outgoing conflict arcs
		std::vector<Arc> conflictArcs;
		for (OutArcIt out(_graph, _graph.target(arc)); out != lemon::INVALID; ++out)
			if (arcTypes[out] == Conflict && _graph.target(out) != _graph.source(arc))
				conflictArcs.push_back(out);

		if (conflictArcs.size() == 0)
			continue;

		ArcLambda arcLambda;
		arcLambda.arc = arc;
		arcLambda.conflictArcs = conflictArcs;

		_conflictArcs.push_back(arcLambda);
	}

	LOG_ALL(cctlog)
			<< "conflict arcs are:" << std::endl;
	for (const auto& arcLambda : _conflictArcs)
		LOG_ALL(cctlog)
				<< "\t" << toString(arcLambda.arc)
				<< ": " << toString(arcLambda.conflictArcs)
				<< std::endl;
	LOG_ALL(cctlog) << std::endl;
}

std::string
CandidateConflictTerm::toString(const Arc& arc) {

	std::stringstream ss;

	ss << "(" << _graph.id(_graph.source(arc)) << ", " << _graph.id(_graph.target(arc)) << ")";

	return ss.str();
}

std::string
CandidateConflictTerm::toString(const ExclusiveArcs& arcs) {

	std::stringstream ss;
	bool first = true;

	for (const auto& arc : arcs) {

		if (!first)
			ss << "--";

		ss << toString(arc);
		first = false;
	}

	return ss.str();
}

} // namespace host
