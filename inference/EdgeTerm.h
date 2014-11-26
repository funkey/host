#ifndef HOST_INFERENCE_EDGE_TERM_H__
#define HOST_INFERENCE_EDGE_TERM_H__

#include <graphs/Graph.h>

class EdgeTerm {

public:

	/**
	 * Add the weights contributed by this edge term to the given edge weights.
	 */
	virtual void addEdgeWeights(host::EdgeWeights& weights) = 0;
};

#endif // HOST_INFERENCE_EDGE_TERM_H__

