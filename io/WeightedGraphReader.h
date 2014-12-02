#ifndef HOST_GRAPHS_WEIGHTED_GRAPH_READER_H__
#define HOST_GRAPHS_WEIGHTED_GRAPH_READER_H__

#include <lemon/lgf_reader.h>
#include "WeightedGraphGenerator.h"

class WeightedGraphReader : public WeightedGraphGenerator {

public:

	WeightedGraphReader(const std::string& filename) :
		_filename(filename) {}

	void fill(
			host::Graph& graph,
			host::ArcWeights& weights,
			host::ArcLabels& labels,
			host::ArcTypes& types) {

		std::ifstream is(_filename.c_str());

		bool isUndirected = true;
		int  rootId = 0;

		lemon::digraphReader(graph, is).
				arcMap("weights", weights).
				arcMap("labels", labels).
				arcMap("types", types).
				attribute("undirected", isUndirected).
				attribute("root", rootId).
				run();

		graph.setUndirected(isUndirected);
		graph.setRoot(graph.nodeFromId(rootId));

		if (isUndirected) {

			// add arcs in opposite direction for each link arc
			for (host::ArcIt arc(graph); arc != lemon::INVALID; ++arc) {

				if (!types[arc] == host::Link)
					continue;

				addOppositeArc(graph, arc, weights, labels, types);
			}

		}

		// add arcs in opposite direction for each conflict arc
		for (host::ArcIt arc(graph); arc != lemon::INVALID; ++arc) {

			if (!types[arc] == host::Conflict)
				continue;

			addOppositeArc(graph, arc, weights, labels, types);
		}
	}

private:

	void addOppositeArc(
			host::Graph& graph,
			const host::Arc& arc,
			host::ArcWeights& weights,
			host::ArcLabels& labels,
			host::ArcTypes& types) {

		host::Arc opposite = graph.addArc(graph.target(arc), graph.source(arc));

		weights[opposite] = weights[arc];
		labels[opposite]  = labels[arc] + std::string("_opp");
		types[opposite]   = types[arc];
	}

	std::string _filename;
};

#endif // HOST_GRAPHS_WEIGHTED_GRAPH_READER_H__

