#ifndef HOST_GRAPHS_LOGGING_H__
#define HOST_GRAPHS_LOGGING_H__

#include <vector>
#include <set>
#include <iostream>
#include <util/exceptions.h>
#include "Graph.h"

std::ostream& operator<<(std::ostream& os, const host::Graph& graph);
std::ostream& operator<<(std::ostream& os, const host::Arc& arc);
std::ostream& operator<<(std::ostream& os, const std::vector<host::Arc>& arcs);
std::ostream& operator<<(std::ostream& os, const host::Edge& edge);
std::ostream& operator<<(std::ostream& os, const std::vector<host::Edge>& edges);
std::ostream& operator<<(std::ostream& os, const std::set<host::Edge>& edges);

namespace host {

class LoggingState {

public:

	static void setLoggingGraph(const host::Graph* graph) { _graph = graph; }

	static const host::Graph* getLoggingGraph() {
	
		if (_graph == 0)
			UTIL_THROW_EXCEPTION(
					UsageError,
					"no graph was set via operator<< for stream output");

		return _graph;
	}
private:

	static const host::Graph* _graph;
};

};

#endif // HOST_GRAPHS_LOGGING_H__

