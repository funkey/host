#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>

#include <graphs/Graph.h>
#include <graphs/RandomWeightedGraphGenerator.h>
#include <graphs/WeightedGraphReader.h>
#include <graphs/WeightedGraphWriter.h>
#include <inference/LeafConstrainedMstSearch.h>

util::ProgramOption optionGraphFile(
		util::_long_name        = "graph",
		util::_short_name       = "g",
		util::_description_text = "Read the graph from the given file.");

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
		util::_description_text = "Create a random graph with edge weights at lest this value.",
		util::_default_value    = 0.0);

util::ProgramOption optionRandomGraphMaxWeight(
		util::_long_name        = "randomGraphMaxWeight",
		util::_description_text = "Create a random graph with edge weights at most this value.",
		util::_default_value    = 1.0);

util::ProgramOption optionWriteResult(
		util::_long_name        = "writeResult",
		util::_description_text = "Write the resulting MST as a graph to the given file.");

int main(int argc, char** argv) {

	util::ProgramOptions::init(argc, argv);
	logger::LogManager::init();

	host::Graph       graph;
	host::EdgeWeights weights(graph);

	if (optionGraphFile) {

		WeightedGraphReader graphReader(optionGraphFile.as<std::string>());
		graphReader.fill(graph, weights);

	} else {

		RandomWeightedGraphGenerator randomWeightedGraphGenerator(
				optionRandomGraphNodes,
				optionRandomGraphEdges,
				optionRandomGraphMinWeight,
				optionRandomGraphMaxWeight);

		randomWeightedGraphGenerator.fill(graph, weights);

		std::cout
				<< "generated a random graph with "
				<< lemon::countNodes(graph)
				<< " nodes" << std::endl;
	}

	if (lemon::countEdges(graph) <= 100) {

		for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
			std::cout
					<< graph.id(graph.u(edge)) << " - "
					<< graph.id(graph.v(edge)) << ": "
					<< weights[edge] << std::endl;
	}

	// the minimal spanning tree
	host::Graph::EdgeMap<bool> mst(graph);

	// a selection of leaf nodes
	host::NodeSelection leaves(graph);

	// add only the first two nodes
	host::Graph::NodeIt node(graph);
	leaves[node] = true;
	++node;
	leaves[node] = true;

	for (host::Graph::NodeIt node(graph); node != lemon::INVALID; ++node)
		std::cout
				<< "node " << graph.id(node)
				<< " is supposed to be "
				<< (leaves[node] ? "a " : "no")
				<< " leaf" << std::endl;

	// search the minimal spanning tree that has the given nodes as leaves
	LeafConstrainedMstSearch lcmstSearch(graph);
	double length;
	bool constraintsFulfilled = lcmstSearch.find(weights, leaves, mst, length);

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
		graphWriter.write(graph, weights, mst);

		std::cout << "wrote result to " << optionWriteResult.as<std::string>() << std::endl;
	}

	return 0;
}
