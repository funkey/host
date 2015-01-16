#include "Hdf5GraphWriter.h"

void
Hdf5GraphWriter::writeGraph(const Hdf5GraphWriter::Graph& graph) {

	int numNodes = 0;
	for (Graph::NodeIt node(graph); node != lemon::INVALID; ++node)
		numNodes++;

	vigra::ArrayVector<int> n(1);
	vigra::ArrayVector<int> edges; // stored in pairs

	n[0] = numNodes;
	_hdfFile.write("num_nodes", n);

	if (numNodes == 0)
		return;

	if (!nodeIdsConsequtive(graph)) {

		// create map from node ids to consecutive ids
		std::map<int, int> nodeMap = createNodeMap(graph);

		for (Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge) {

			edges.push_back(nodeMap[graph.id(graph.u(edge))]);
			edges.push_back(nodeMap[graph.id(graph.v(edge))]);
		}

	} else {

		for (Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge) {

			edges.push_back(graph.id(graph.u(edge)));
			edges.push_back(graph.id(graph.v(edge)));
		}
	}

	if (edges.size() > 0)
		_hdfFile.write("edges", edges);
}

bool
Hdf5GraphWriter::nodeIdsConsequtive(const Hdf5GraphWriter::Graph& graph) {

	int i = 0;
	for (Graph::NodeIt node(graph); node != lemon::INVALID; ++node) {

		if (graph.id(node) != i)
			return false;

		i++;
	}

	return true;
}

std::map<int, int>
Hdf5GraphWriter::createNodeMap(const Hdf5GraphWriter::Graph& graph) {

	std::map<int, int> nodeMap;
	int i = 0;
	for (Graph::NodeIt node(graph); node != lemon::INVALID; ++node) {

		nodeMap[graph.id(node)] = i;
		i++;
	}

	return nodeMap;
}
