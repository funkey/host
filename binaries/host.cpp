#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>

#include <graphs/Graph.h>
#include <graphs/RandomWeightedGraphGenerator.h>
#include <inference/LeafConstrainedMstSearch.h>

int main(int argc, char** argv) {

	util::ProgramOptions::init(argc, argv);
	logger::LogManager::init();

	host::Graph       graph;
	host::EdgeWeights weights(graph);

	RandomWeightedGraphGenerator randomWeightedGraphGenerator(5, 8, 1, 1);
	randomWeightedGraphGenerator.fill(graph, weights);

	std::cout
			<< "generated a random graph with "
			<< lemon::countNodes(graph)
			<< " nodes" << std::endl;

	if (lemon::countEdges(graph) < 100) {

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
	bool constraintsFulfilled = lcmstSearch.find(weights, leaves, mst);

	if (constraintsFulfilled)
		std::cout << "found a minimal spanning tree that fulfills the constraints" << std::endl;

	if (lemon::countEdges(graph) < 100) {

		std::cout << "minimal spanning tree is:" << std::endl;
		for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
			std::cout
					<< graph.id(graph.u(edge)) << " - "
					<< graph.id(graph.v(edge)) << ": "
					<< mst[edge] << std::endl;
	}

	return 0;
}
