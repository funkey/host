#include <util/Logger.h>
#include <graph/Logging.h>
#include "Configuration.h"
#include "ConflictChainTerm.h"

logger::LogChannel conflictchainlog("conflictchainlog", "[ConflictChainTerm] ");

namespace host {

ConflictChainTerm::ConflictChainTerm(
		const Graph&    graph,
		const ArcTypes& arcTypes,
		unsigned int    maxChainLength) :
	_maxChainLength(maxChainLength),
	_graph(graph),
	_arcTypes(arcTypes) {

	findChains();
}

void
ConflictChainTerm::lambdaBounds(
		Lambdas::iterator beginLower,
		Lambdas::iterator endLower,
		Lambdas::iterator,
		Lambdas::iterator) {

	// all lambdas have to be positive
	for (Lambdas::iterator i = beginLower; i != endLower; i++)
		*i = 0;
}

void
ConflictChainTerm::setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator) {

	Lambdas::const_iterator i = begin;

	LOG_ALL(conflictchainlog) << "λ set to:" << _graph << std::endl;

	for (Chain& chain : _chains) {

		LOG_ALL(conflictchainlog) << "\t" << chain.arcs << ":\t" << *i << std::endl;

		chain.lambda = *i;
		i++;
	}

	LOG_ALL(conflictchainlog) << std::endl;
}

void
ConflictChainTerm::addArcWeights(ArcWeights& weights) {

	for (const Chain& chain : _chains)
		for (const Arc& arc : chain.arcs)
			weights[arc] += chain.lambda;
}

double
ConflictChainTerm::constant() {

	double constant = 0;
	for (const Chain& chain : _chains)
		constant -= (static_cast<double>(chain.length())-1)*chain.lambda;

	return constant;
}

bool
ConflictChainTerm::gradient(
		const ArcSelection& mst,
		Lambdas::iterator   begin,
		Lambdas::iterator) {

	Lambdas::iterator i = begin;

	bool feasible = true;

	LOG_ALL(conflictchainlog) << "gradient is:" << std::endl;

	for (const Chain& chain : _chains) {

		double gradient = -static_cast<double>(chain.length()) + 1;
		for (const Arc& arc : chain.arcs)
			gradient += mst[arc];

		if (gradient < 0 && chain.lambda < Configuration::LambdaEpsilon) {

			LOG_ALL(conflictchainlog) << "\tfixed from " << gradient << " to 0, λ is " << chain.lambda << std::endl;
			gradient = 0;
		}

		feasible &= (gradient <= 0);

		LOG_ALL(conflictchainlog) << "\t" << _graph << chain.arcs << ":\t" << gradient << std::endl;

		*i = gradient;
		i++;
	}

	LOG_ALL(conflictchainlog) << std::endl;

	return feasible;
}

void
ConflictChainTerm::findChains() {

	if (_maxChainLength == 0)
		return;

	std::vector<Arc> arcs;

	for (ArcIt arc(_graph); arc != lemon::INVALID; ++arc)
		if (_arcTypes[arc] == Conflict)
			findChains(arcs, arc);
}

void
ConflictChainTerm::findChains(
		std::vector<Arc>& arcs,
		const Arc&        nextArc) {

	// no cycles
	for (const Arc& arc : arcs)
		if (_graph.source(arc) == _graph.target(nextArc))
			return;

	arcs.push_back(nextArc);

	if (arcs.size() == _maxChainLength) {

		_chains.push_back(Chain{arcs,0});

	} else {

		const Arc& lastArc = *(arcs.end() - 1);

		for (OutArcIt out(_graph, _graph.target(lastArc)); out != lemon::INVALID; ++out)
			if (_arcTypes[out] == Conflict)
				findChains(arcs, out);
	}

	arcs.erase(arcs.end() - 1);
}

} // namespace host
