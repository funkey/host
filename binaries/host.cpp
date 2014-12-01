#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>

#include <graphs/Graph.h>
#include <graphs/RandomWeightedGraphGenerator.h>
#include <graphs/WeightedGraphReader.h>
#include <graphs/WeightedGraphWriter.h>
#include <inference/HostSearch.h>
#include <inference/InitialWeightTerm.h>
#include <inference/CandidateConflictTerm.h>
#include <inference/MultiArcFactorTerm.h>
#include <io/MultiArcFactorReader.h>

util::ProgramOption optionGraphFile(
		util::_long_name        = "graph",
		util::_short_name       = "g",
		util::_description_text = "Read the graph from the given file.");

util::ProgramOption optionMultiArcFactorFile(
		util::_long_name        = "multiArcFactors",
		util::_short_name       = "m",
		util::_description_text = "Read the multi-arc factors from the given file.");

util::ProgramOption optionRandomGraphNodes(
		util::_long_name        = "randomGraphNodes",
		util::_description_text = "Create a random graph with this number of nodes.",
		util::_default_value    = 100);

util::ProgramOption optionRandomGraphArcs(
		util::_long_name        = "randomGraphArcs",
		util::_description_text = "Create a random graph with this number of arcs.",
		util::_default_value    = 1000);

util::ProgramOption optionRandomGraphMinWeight(
		util::_long_name        = "randomGraphMinWeight",
		util::_description_text = "Create a random graph with arc weights at least this value.",
		util::_default_value    = 0.0);

util::ProgramOption optionRandomGraphMaxWeight(
		util::_long_name        = "randomGraphMaxWeight",
		util::_description_text = "Create a random graph with arc weights at most this value.",
		util::_default_value    = 1.0);

util::ProgramOption optionWriteResult(
		util::_long_name        = "writeResult",
		util::_description_text = "Write the resulting MST as a graph to the given file.");

util::ProgramOption optionNumIterations(
		util::_long_name        = "numIterations",
		util::_description_text = "The maximal number of iterations for finding the HOST.",
		util::_default_value    = 100);

int main(int argc, char** argv) {

	util::ProgramOptions::init(argc, argv);
	logger::LogManager::init();

	host::Graph           graph;
	host::ArcWeights      arcWeights(graph);
	host::ArcLabels       arcLabels(graph);
	host::ArcTypes        arcTypes(graph);
	host::MultiArcFactors multiArcFactors;

	if (optionGraphFile) {

		WeightedGraphReader graphReader(optionGraphFile.as<std::string>());
		graphReader.fill(graph, arcWeights, arcLabels, arcTypes);

	} else {

		RandomWeightedGraphGenerator randomWeightedGraphGenerator(
				optionRandomGraphNodes,
				optionRandomGraphArcs,
				optionRandomGraphMinWeight,
				optionRandomGraphMaxWeight);

		randomWeightedGraphGenerator.fill(graph, arcWeights, arcLabels, arcTypes);

		std::cout
				<< "generated a random graph with "
				<< lemon::countNodes(graph)
				<< " nodes" << std::endl;
	}

	if (optionMultiArcFactorFile) {

		host::MultiArcFactorReader factorReader(optionMultiArcFactorFile.as<std::string>());
		factorReader.fill(graph, arcLabels, multiArcFactors);
	}

	if (lemon::countArcs(graph) <= 100) {

		for (host::Graph::ArcIt arc(graph); arc != lemon::INVALID; ++arc)
			std::cout
					<< graph.id(graph.source(arc)) << " - "
					<< graph.id(graph.target(arc)) << ": "
					<< arcWeights[arc] << std::endl;
	}

	// the minimal spanning tree
	host::Graph::ArcMap<bool> mst(graph);

	// search the minimal spanning tree under consideration of conflicting 
	// candidates
	HostSearch hostSearch(graph);

	host::InitialWeightTerm     weightTerm(graph, arcWeights);
	host::CandidateConflictTerm cctTerm(graph, arcTypes);
	host::MultiArcFactorTerm    mefTerm(graph, multiArcFactors);

	hostSearch.addTerm(&weightTerm);
	hostSearch.addTerm(&cctTerm);
	hostSearch.addTerm(&mefTerm);

	double length;
	bool constraintsFulfilled = hostSearch.find(mst, length, optionNumIterations.as<unsigned int>());

	if (constraintsFulfilled)
		std::cout << "found a minimal spanning tree that fulfills the constraints" << std::endl;

	if (lemon::countArcs(graph) <= 100) {

		std::cout << "minimal spanning tree is:" << std::endl;
		for (host::Graph::ArcIt arc(graph); arc != lemon::INVALID; ++arc)
			std::cout
					<< graph.id(graph.source(arc)) << " - "
					<< graph.id(graph.target(arc)) << ": "
					<< mst[arc] << std::endl;
	}

	std::cout << "length of minimal spanning tree is " << length << std::endl;

	if (optionWriteResult) {

		WeightedGraphWriter graphWriter(optionWriteResult.as<std::string>());
		graphWriter.write(graph, arcWeights, mst);

		std::cout << "wrote result to " << optionWriteResult.as<std::string>() << std::endl;
	}

	return 0;
}
