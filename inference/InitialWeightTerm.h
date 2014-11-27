#ifndef HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__
#define HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__

#include "EdgeTerm.h"

namespace host {

/**
 * An edge term contributing an initial set of edge weights.
 */
class InitialWeightTerm : public EdgeTerm {

public:

	InitialWeightTerm(
			const Graph& graph,
			const EdgeWeights& weights) :
		_graph(graph),
		_weights(weights) {}

	void addEdgeWeights(EdgeWeights& weights) {

		for (Graph::EdgeIt edge(_graph); edge != lemon::INVALID; ++edge)
			weights[edge] += _weights[edge];
	}

private:

	const Graph& _graph;

	const EdgeWeights& _weights;
};

} // namespace host

#endif // HOST_INFERENCE_INITIAL_WEIGHT_TERM_H__

