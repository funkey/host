#include <fstream>
#include "MultiEdgeFactorReader.h"

namespace host {

void
MultiEdgeFactorReader::fill(
		const Graph&      graph,
		const EdgeLabels& labels,
		MultiEdgeFactors& factors) {

	std::ifstream in(_filename);

	// create a reverse look-up from labels to edges
	std::map<std::string, Edge> labelEdgeMap;
	for (Graph::EdgeIt edge(graph); edge != lemon::INVALID; ++edge)
		labelEdgeMap[labels[edge]] = edge;

	std::string line;
	while (std::getline(in, line)) {

		double value;
		MultiEdgeFactors::Edges edges;

		std::stringstream ss(line);

		ss >> value;
		while (ss.good()) {

			std::string label;
			ss >> label;
			edges.push_back(labelEdgeMap[label]);
		}

		factors[edges] = value;
	}
}

} // namespace host
