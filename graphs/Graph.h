#ifndef HOST_GRAPHS_GRAPH_H__
#define HOST_GRAPHS_GRAPH_H__

#include <lemon/list_graph.h>

namespace host {

enum ArcType {

	Link     = 0,
	Conflict = 1
};

class Graph : public lemon::ListDigraph {

public:

	Graph() :
		_isUndirected(false) {}

	/**
	 * Returns true if this graph is undirected. Undirected graphs have 
	 * symmetric link arcs, i.e., whenever node A links to node B with cost c, 
	 * node B does also link to node A with the same costs c.
	 */
	bool isUndirected() { return _isUndirected; }

	/**
	 * Mark this graph as undirected. This does not change the graph and does 
	 * not test whether the arcs are really symmetric. Set this if you created 
	 * the graph and know whether it is directed or not.
	 */
	void setUndirected(bool isUndirected) { _isUndirected = isUndirected; }

	/**
	 * Set the root node for the branching search in directed graphs.
	 */
	void setRoot(Node root) { _root = root; }

	/**
	 * Get the root node.
	 */
	Node getRoot() const { return _root; }

private:

	bool _isUndirected;

	Node _root;
};

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

