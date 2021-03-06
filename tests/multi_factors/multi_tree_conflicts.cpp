#include <tests.h>
#include <io/WeightedGraphReader.h>
#include <io/MultiEdgeFactorReader.h>
#include <inference/HostSearch.h>
#include <inference/ExplicitWeightTerm.h>
#include <inference/CandidateConflictTerm.h>
#include <inference/MultiEdgeFactorTerm.h>

void multi_tree_conflicts() {

	boost::filesystem::path dataDir = dir_of(__FILE__);
	boost::filesystem::path graphfile = dataDir/"tree_conflicts.dat";
	boost::filesystem::path factorsfile = dataDir/"tree_conflicts_factors.dat";

	host::Graph            graph;
	host::ArcWeights       weights(graph);
	host::ArcLabels        labels(graph);
	host::ArcTypes         types(graph);

	host::WeightedGraphReader reader(graphfile.native());
	reader.fill(graph, weights, labels, types);

	// check if we have the conflict edges
	for (host::ArcIt arc(graph); arc != lemon::INVALID; ++arc) {

		if ((graph.id(graph.source(arc)) == 1 && (graph.id(graph.target(arc)) == 2 || graph.id(graph.target(arc)) == 3)) ||
		    (graph.id(graph.target(arc)) == 1 && (graph.id(graph.source(arc)) == 2 || graph.id(graph.source(arc)) == 3)))
			BOOST_CHECK_EQUAL(types[arc], host::Conflict);
		else
			BOOST_CHECK_EQUAL(types[arc], host::Link);
	}

	host::MultiEdgeFactors multiEdgeFactors;
	host::MultiEdgeFactorReader factorReader(factorsfile.native());
	factorReader.fill(graph, labels, multiEdgeFactors);

	host::ExplicitWeightTerm    edgeWeightsTerm(graph, weights);
	host::CandidateConflictTerm conflictsTerm(graph, types);
	host::MultiEdgeFactorTerm   multiFactorsTerm(graph, multiEdgeFactors);

	host::HostSearch search(graph);
	search.addTerm(&edgeWeightsTerm);
	search.addTerm(&conflictsTerm);
	search.addTerm(&multiFactorsTerm);

	host::ArcSelection mst(graph);
	double value;
	bool optimal = search.find(mst, value);

	// check if the correct solution was found
	BOOST_CHECK(optimal);
	BOOST_CHECK_CLOSE(value, -0.25, 1e-6);
}
