#include "Logging.h"

namespace host {

	const Graph* LoggingState::_graph = 0;

} // namespace host

std::ostream&
operator<<(std::ostream& os, const host::Graph& graph) {

	host::LoggingState::setLoggingGraph(&graph);
	return os;
}

std::ostream&
operator<<(std::ostream& os, const host::Arc& arc) {

	const host::Graph& graph = *host::LoggingState::getLoggingGraph();

	os << "(" << graph.id(graph.source(arc)) << ", " << graph.id(graph.target(arc)) << ")";
	return os;
}

std::ostream&
operator<<(std::ostream& os, const std::vector<host::Arc>& arcs) {

	bool first = true;

	for (const auto& arc : arcs) {

		if (!first)
			os << "--";

		os << arc;
		first = false;
	}

	return os;
}

std::ostream&
operator<<(std::ostream& os, const host::Edge& edge) {

	const host::Graph& graph = *host::LoggingState::getLoggingGraph();

	os << "(" << graph.id(graph.source(edge)) << ", " << graph.id(graph.target(edge)) << ")";
	return os;
}

std::ostream&
operator<<(std::ostream& os, const std::vector<host::Edge>& edges) {

	bool first = true;

	for (const auto& edge : edges) {

		if (!first)
			os << "--";

		os << edge;
		first = false;
	}

	return os;
}
