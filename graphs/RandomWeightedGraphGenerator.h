#include "WeightedGraphGenerator.h"

class RandomWeightedGraphGenerator {

public:

	RandomWeightedGraphGenerator(
			unsigned int numVertices,
			unsigned int numEdges,
			double minEdgeWeight,
			double maxEdgeWeight) :
		_numVertices(numVertices),
		_numEdges(numEdges),
		_minEdgeWeight(minEdgeWeight),
		_maxEdgeWeight(maxEdgeWeight) {}

	void fill(host::Graph& graph, host::EdgeWeights& weights, host::EdgeTypes& types);

private:

	unsigned int _numVertices;
	unsigned int _numEdges;
	double _minEdgeWeight;
	double _maxEdgeWeight;
};
