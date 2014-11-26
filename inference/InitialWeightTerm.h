#ifndef HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__
#define HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__

#include "EdgeTerm.h"

/**
 * An edge term contributing an initial set of edge weights.
 */
class InitialWeightTerm : public EdgeTerm {

public:

	InitialWeightTerm(
			const host::Graph& graph,
			const host::EdgeWeights& weights) :
		_graph(graph),
		_weights(weights) {}

	void addEdgeWeights(host::EdgeWeights& weights) {

		for (host::Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
			weights[edge] += _weights[edge];
	}

private:

	const host::Graph& _graph;

	const host::EdgeWeights& _weights;
};

#endif // HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__

