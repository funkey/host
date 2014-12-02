#include <fstream>
#include "MultiArcFactorReader.h"

namespace host {

void
MultiArcFactorReader::fill(
		const Graph&      graph,
		const ArcLabels& labels,
		MultiArcFactors& factors) {

	std::ifstream in(_filename);

	// create a reverse look-up from labels to arcs
	std::map<std::string, Arc> labelArcMap;
	for (host::ArcIt arc(graph); arc != lemon::INVALID; ++arc)
		labelArcMap[labels[arc]] = arc;

	std::string line;
	while (std::getline(in, line)) {

		double value;
		MultiArcFactors::Edges arcs;

		std::stringstream ss(line);

		ss >> value;
		while (ss.good()) {

			std::string label;
			ss >> label;
			arcs.push_back(labelArcMap[label]);
		}

		factors[arcs] = value;
	}
}

} // namespace host
