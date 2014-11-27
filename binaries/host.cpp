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
#include <inference/MultiEdgeFactorTerm.h>
#include <io/MultiEdgeFactorReader.h>

util::ProgramOption optionGraphFile(
		util::_long_name        = "graph",
		util::_short_name       = "g",
		util::_description_text = "Read the graph from the given file.");

util::ProgramOption optionMultiEdgeFactorFile(
		util::_long_name        = "multiEdgeFactors",
		util::_short_name       = "m",
		util::_description_text = "Read the multi-edge factors from the given file.");

util::ProgramOption optionRandomGraphNodes(
		util::_long_name        = "randomGraphNodes",
		util::_description_text = "Create a random graph with this number of nodes.",
		util::_default_value    = 100);

util::ProgramOption optionRandomGraphEdges(
		util::_long_name        = "randomGraphEdges",
		util::_description_text = "Create a random graph with this number of edges.",
		util::_default_value    = 1000);

util::ProgramOption optionRandomGraphMinWeight(
		util::_long_name        = "randomGraphMinWeight",
		util::_description_text = "Create a random graph with edge weights at least this value.",
		util::_default_value    = 0.0);

util::ProgramOption optionRandomGraphMaxWeight(
		util::_long_name        = "randomGraphMaxWeight",
		util::_description_text = "Create a random graph with edge weights at most this value.",
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

	host::Graph            graph;
	host::EdgeWeights      edgeWeights(graph);
	host::EdgeLabels       edgeLabels(graph);
	host::EdgeTypes        edgeTypes(graph);
	host::MultiEdgeFactors multiEdgeFactors;

	if (optionGraphFile) {

		WeightedGraphReader graphReader(optionGraphFile.as<std::string>());
		graphReader.fill(graph, edgeWeights, edgeLabels, edgeTypes);

	} else {

		RandomWeightedGraphGenerator randomWeightedGraphGenerator(
				optionRandomGraphNodes,
				optionRandomGraphEdges,
				optionRandomGraphMinWeight,
				optionRandomGraphMaxWeight);

		randomWeightedGraphGenerator.fill(graph, edgeWeights, edgeLabels, edgeTypes);

		std::cout
				<< "generated a random graph with "
				<< lemon::countNodes(graph)
				<< " nodes" << std::endl;
	}

	if (optionMultiEdgeFactorFile) {

		host::MultiEdgeFactorReader factorReader(optionMultiEdgeFactorFile.as<std::string>());
		factorReader.fill(graph, edgeLabels, multiEdgeFactors);
	}

	if (lemon::countEdges(graph) <= 100) {

		for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
			std::cout
					<< graph.id(graph.u(edge)) << " - "
					<< graph.id(graph.v(edge)) << ": "
					<< edgeWeights[edge] << std::endl;
	}

	// the minimal spanning tree
	host::Graph::EdgeMap<bool> mst(graph);

	// search the minimal spanning tree under consideration of conflicting 
	// candidates
	HostSearch hostSearch(graph);

	host::InitialWeightTerm     weightTerm(graph, edgeWeights);
	host::CandidateConflictTerm cctTerm(graph, edgeTypes);
	host::MultiEdgeFactorTerm   mefTerm(graph, multiEdgeFactors);

	hostSearch.addTerm(&weightTerm);
	hostSearch.addTerm(&cctTerm);
	hostSearch.addTerm(&mefTerm);

	double length;
	bool constraintsFulfilled = hostSearch.find(mst, length, optionNumIterations.as<unsigned int>());

	if (constraintsFulfilled)
		std::cout << "found a minimal spanning tree that fulfills the constraints" << std::endl;

	if (lemon::countEdges(graph) <= 100) {

		std::cout << "minimal spanning tree is:" << std::endl;
		for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
			std::cout
					<< graph.id(graph.u(edge)) << " - "
					<< graph.id(graph.v(edge)) << ": "
					<< mst[edge] << std::endl;
	}

	std::cout << "length of minimal spanning tree is " << length << std::endl;

	if (optionWriteResult) {

		WeightedGraphWriter graphWriter(optionWriteResult.as<std::string>());
		graphWriter.write(graph, edgeWeights, mst);

		std::cout << "wrote result to " << optionWriteResult.as<std::string>() << std::endl;
	}

	return 0;
}
