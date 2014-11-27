#include <util/Logger.h>
#include "RandomWeightedGraphGenerator.h"

logger::LogChannel randomwggeneratorlog("randomwggeneratorlog", "[RandomWeightedGraphGenerator] ");

void
RandomWeightedGraphGenerator::fill(
		host::Graph& graph,
		host::EdgeWeights& weights,
		host::EdgeLabels& labels,
		host::EdgeTypes& types) {

	std::vector<lemon::ListGraph::Node> vertices;

	srand(23);

	for (unsigned int i = 0; i < _numVertices; i++)
		vertices.push_back(graph.addNode());

	for (unsigned int i = 0; i < _numEdges; i++) {

		lemon::ListGraph::Node u;
		lemon::ListGraph::Node v;

		for (;;) {

			bool validEdge = true;

			u = vertices[rand()%vertices.size()];
			v = vertices[rand()%vertices.size()];

			if (u == v)
				continue;

			for (host::Graph::IncEdgeIt edge(graph, u); edge != lemon::INVALID; ++edge) {

				if (graph.u(edge) == v || graph.v(edge) == v) {

					validEdge = false;
					break;
				}
			}

			if (validEdge)
				break;
		}

		LOG_ALL(randomwggeneratorlog)
				<< "adding edge " << graph.id(u) << " - " << graph.id(v) << std::endl;

		double weight = _minEdgeWeight + (static_cast<double>(rand())/RAND_MAX)*(_maxEdgeWeight - _minEdgeWeight);

		lemon::ListGraph::Edge e = graph.addEdge(u, v);
		weights[e] = weight;
		types[e] = host::Link;
	}
}
