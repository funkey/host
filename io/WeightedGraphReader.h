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

		lemon::digraphReader(graph, is).arcMap("weights", weights).arcMap("labels", labels).arcMap("types", types).run();
	}

private:

	std::string _filename;
};

#endif // HOST_GRAPHS_WEIGHTED_GRAPH_READER_H__

