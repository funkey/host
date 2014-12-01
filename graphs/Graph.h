#ifndef HOST_GRAPHS_GRAPH_H__
#define HOST_GRAPHS_GRAPH_H__

#include <lemon/list_graph.h>

namespace host {

enum ArcType {

	Link     = 0,
	Conflict = 1
};

typedef lemon::ListDigraph         Graph;
typedef Graph::Node                Node;
typedef Graph::NodeIt              NodeIt;
typedef Graph::NodeMap<double>     NodeWeights;
typedef Graph::NodeMap<bool>       NodeSelection;
typedef Graph::Arc                 Arc;
typedef Graph::ArcIt               ArcIt;
typedef Graph::OutArcIt            OutArcIt;
typedef Graph::InArcIt             InArcIt;
typedef Graph::ArcMap<double>      ArcWeights;
typedef Graph::ArcMap<std::string> ArcLabels;
typedef Graph::ArcMap<ArcType>     ArcTypes;
typedef Graph::ArcMap<bool>        ArcSelection;

} // namespace host

#endif // HOST_GRAPHS_GRAPH_H__

