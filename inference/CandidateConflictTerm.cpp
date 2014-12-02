#include <util/Logger.h>
#include <util/exceptions.h>
#include "Configuration.h"
#include "CandidateConflictTerm.h"

namespace host {

logger::LogChannel cctlog("cctlog", "[CandidateConflictTerm] ");

CandidateConflictTerm::CandidateConflictTerm(
		const Graph& graph,
		const ArcTypes& arcTypes) :
	_graph(graph) {

	findExclusiveEdges(arcTypes);
	findConflictArcs(arcTypes);
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

	for (auto& exclusive : _exclusiveEdges) {

		exclusive.lambda = *i; i++;

		LOG_ALL(cctlog)
				<< "\t" << toString(exclusive.edges)
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

	// add the edge weight to all arcs of the corresponding edge
	for (const auto& exclusive : _exclusiveEdges)
		for (const auto& edge : exclusive.edges)
			for (const auto& arc : edge)
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

	for (auto exclusive : _exclusiveEdges)
		constant -= (static_cast<int>(exclusive.edges.size()) - 1)*exclusive.lambda;

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

	for (const auto& exclusive : _exclusiveEdges) {

		int sum = 0;
		for (const auto& edge : exclusive.edges)
			for (const auto& arc : edge)
				sum += mst[arc];

		*i = sum - (static_cast<int>(exclusive.edges.size()) - 1);

		if (*i < 0 && exclusive.lambda < Configuration::LambdaEpsilon) {

			LOG_ALL(cctlog) << "\tfixed from " << (*i) << " to 0, λ is " << exclusive.lambda << std::endl;
			*i = 0;
		}

		LOG_ALL(cctlog) << "\t" << toString(exclusive.edges) << ":\t" << *i << std::endl;

		i++;
	}

	for (const auto& conflict : _conflictArcs) {

		const Arc&   arc = conflict.arc;
		unsigned int n   = conflict.conflictArcs.size();

		int sum = 0;
		for (const auto& conflictArc : conflict.conflictArcs)
			sum += mst[conflictArc];

		*i = (static_cast<double>(mst[arc]) - 1)*n + sum;

		if (*i < 0 && conflict.lambda < Configuration::LambdaEpsilon) {

			LOG_ALL(cctlog) << "\tfixed from " << (*i) << " to 0, λ is " << conflict.lambda << std::endl;
			*i = 0;
		}

		LOG_ALL(cctlog)
				<< "\t" << toString(arc)
				<< ": " << toString(conflict.conflictArcs)
				<< ":\t" << *i
				<< std::endl;
		i++;
	}

	LOG_ALL(cctlog) << std::endl;

	if (i != end)
		UTIL_THROW_EXCEPTION(
				UsageError,
				"given range of lambdas does not match number of lambdas");
}

void
CandidateConflictTerm::findExclusiveEdges(const ArcTypes& arcTypes) {

	// for each conflict edge
	for (Graph::ArcIt arc(_graph); arc != lemon::INVALID; ++arc) {

		if (arcTypes[arc] != Conflict)
			continue;

		// consider only one direction
		if (_graph.id(_graph.source(arc)) >= _graph.id(_graph.target(arc)))
			continue;

		Edges sourceEdges = findLinkEdges(_graph.source(arc), arcTypes);
		Edges targetEdges = findLinkEdges(_graph.target(arc), arcTypes);

		// for each source link edge
		for (const Edge& sourceEdge : sourceEdges) {

			// for each target link edge
			for (const Edge& targetEdge : targetEdges) {

				if (sourceEdge == targetEdge)
					UTIL_THROW_EXCEPTION(
							UsageError,
							"conflict arc (" << _graph.id(_graph.source(arc)) << ", " << _graph.id(_graph.target(arc)) <<
							" has parallel link arcs: " << toString(sourceEdge));

				_exclusiveEdges.push_back(ExclusiveEdgesLambda{{sourceEdge,targetEdge},0});
			}
		}
	}

	LOG_ALL(cctlog)
			<< "exclusive arcs are:" << std::endl;
	for (const auto& exclusive : _exclusiveEdges)
		LOG_ALL(cctlog)
				<< "\t" << toString(exclusive.edges) << std::endl;
	LOG_ALL(cctlog) << std::endl;
}

CandidateConflictTerm::Edges
CandidateConflictTerm::findLinkEdges(const Node& node, const ArcTypes& arcTypes) {

	Edges edges;
	std::map<Node, size_t> nodeEdgeMap;

	// for each outgoing link arc
	for (OutArcIt out(_graph, node); out != lemon::INVALID; ++out) {

		if (arcTypes[out] != Link)
			continue;

		// create one edge
		Edge edge;
		edge.addArc(out);
		edges.push_back(edge);

		// remember mapping of target node to edge
		nodeEdgeMap[_graph.target(out)] = edges.size() - 1;
	}

	// for each incoming link arc
	for (InArcIt in(_graph, node); in != lemon::INVALID; ++in) {

		if (arcTypes[in] != Link)
			continue;

		// add to existing edge...
		if (nodeEdgeMap.count(_graph.source(in))) {

			edges[nodeEdgeMap[_graph.source(in)]].addArc(in);

		// ...or create a new one
		} else {

			Edge edge;
			edge.addArc(in);
			edges.push_back(edge);
		}
	}

	return edges;
}

void
CandidateConflictTerm::findConflictArcs(const ArcTypes& arcTypes) {

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

		ConflictArcsLambda arcLambda;
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
CandidateConflictTerm::toString(const Arcs& arcs) {

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

std::string
CandidateConflictTerm::toString(const Edge& edge) {

	return "[" + toString(*edge.begin()) + "]";
}

std::string
CandidateConflictTerm::toString(const Edges& edges) {

	std::stringstream ss;
	bool first = true;

	for (const auto& edge : edges) {

		if (!first)
			ss << "--";

		ss << toString(edge);
		first = false;
	}

	return ss.str();
}

} // namespace host
