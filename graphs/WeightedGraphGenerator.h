#ifndef HOST_WEIGHTED_GRAPH_GENERATOR_H__
#define HOST_WEIGHTED_GRAPH_GENERATOR_H__

#include "Graph.h"

class WeightedGraphGenerator {

public:

	/**
	 * Create a weighted graph.
	 */
	virtual void fill(host::Graph& graph, host::EdgeWeights& weights, host::EdgeTypes& types) = 0;
};

#endif // HOST_GRAPH_GENERATOR_H__

