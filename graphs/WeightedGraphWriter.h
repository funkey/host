#ifndef HOST_GRAPHS_WEIGHTED_GRAPH_WRITER_H__
#define HOST_GRAPHS_WEIGHTED_GRAPH_WRITER_H__

#include <lemon/lgf_writer.h>

class WeightedGraphWriter {

public:

	WeightedGraphWriter(const std::string& filename) :
		_filename(filename) {}

	void write(const host::Graph& graph, const host::EdgeWeights& weights) {

		host::EdgeSelection dummy(graph);
		write(graph, weights, dummy);
	}

	void write(const host::Graph& graph, const host::EdgeWeights& weights, const host::EdgeSelection& edgeSelection) {

		std::ofstream os(_filename.c_str());

		if (_filename.find(".lgf") != std::string::npos) {

			lemon::graphWriter(graph, os).edgeMap("weights", weights).edgeMap("mst", edgeSelection).run();

		} else { // write in GUESS format

			os << "nodedef>name VARCHAR" << std::endl;

			for (host::Graph::NodeIt node(graph); node != lemon::INVALID; ++node)
				os << graph.id(node) << std::endl;

			os << "edgedef>node1 VARCHAR,node2 VARCHAR,weight DOUBLE,mst BOOLEAN" << std::endl;

			for (host::Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
				os
						<< graph.id(graph.u(edge)) << ","
						<< graph.id(graph.v(edge)) << ","
						<< weights[edge] << ","
						<< edgeSelection[edge] << std::endl;
		}
	}

private:

	std::string _filename;
};

#endif // HOST_GRAPHS_WEIGHTED_GRAPH_WRITER_H__

