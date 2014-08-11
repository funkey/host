#include <iostream>
#include <lemon/list_graph.h>

int main(int argc, char** argv) {

	lemon::ListGraph graph;

	lemon::ListGraph::Node u = graph.addNode();
	lemon::ListGraph::Node v = graph.addNode();
	lemon::ListGraph::Edge e = graph.addEdge(u, v);

	std::cout << "build a graph with " << lemon::countNodes(graph) << " nodes" << std::endl;

	return 0;
}
