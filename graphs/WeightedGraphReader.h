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
			host::EdgeWeights& weights,
			host::EdgeLabels& labels,
			host::EdgeTypes& types) {

		std::ifstream is(_filename.c_str());

		lemon::graphReader(graph, is).edgeMap("weights", weights).edgeMap("labels", labels).edgeMap("types", types).run();
	}

private:

	std::string _filename;
};

#endif // HOST_GRAPHS_WEIGHTED_GRAPH_READER_H__

