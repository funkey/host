#ifndef HOST_GRAPHS_GRAPH_H__
#define HOST_GRAPHS_GRAPH_H__

#include <lemon/list_graph.h>

namespace host {

typedef lemon::ListGraph       Graph;
typedef Graph::Node            Node;
typedef Graph::NodeMap<double> NodeWeights;
typedef Graph::NodeMap<bool>   NodeSelection;
typedef Graph::Edge            Edge;
typedef Graph::EdgeMap<double> EdgeWeights;
typedef Graph::EdgeMap<bool>   EdgeSelection;

} // namespace host

#endif // HOST_GRAPHS_GRAPH_H__

