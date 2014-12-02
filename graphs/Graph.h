#ifndef HOST_GRAPHS_GRAPH_H__
#define HOST_GRAPHS_GRAPH_H__

#include <lemon/list_graph.h>

namespace host {

enum ArcType {

	Link     = 0,
	Conflict = 1
};

typedef lemon::ListDigraph GraphBase;

typedef GraphBase::Node                Node;
typedef GraphBase::NodeIt              NodeIt;
typedef GraphBase::NodeMap<double>     NodeWeights;
typedef GraphBase::NodeMap<bool>       NodeSelection;
typedef GraphBase::Arc                 Arc;
typedef GraphBase::ArcIt               ArcIt;
typedef GraphBase::OutArcIt            OutArcIt;
typedef GraphBase::InArcIt             InArcIt;
typedef GraphBase::ArcMap<double>      ArcWeights;
typedef GraphBase::ArcMap<std::string> ArcLabels;
typedef GraphBase::ArcMap<ArcType>     ArcTypes;

/**
 * An edge consists of all arcs between two nodes (i.e., one or two).
 */
class Edge {

public:

	typedef std::vector<Arc>     Arcs;
	typedef Arcs::iterator       iterator;
	typedef Arcs::const_iterator const_iterator;

	void addArc(const Arc& arc) { _arcs.push_back(arc); std::sort(_arcs.begin(), _arcs.end()); }

	iterator begin() { return _arcs.begin(); }
	const_iterator begin() const { return _arcs.begin(); }
	iterator end() { return _arcs.end(); }
	const_iterator end() const { return _arcs.end(); }

	bool operator==(const Edge& other) const { return _arcs == other._arcs; }

	bool contains(const Arc& arc) { return std::find(_arcs.begin(), _arcs.end(), arc) != _arcs.end(); }

private:

	Arcs _arcs;
};

class Graph : public GraphBase {

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

	/**
	 * The "source" of an undirected edge, i.e., the lower of the two nodes.
	 */
	Node source(const Edge& edge) const { return source(*edge.begin()); }
	using GraphBase::source;

	/**
	 * The "target" of an undirected edge, i.e., the bigger of the two nodes.
	 */
	Node target(const Edge& edge) const { return target(*edge.begin()); }
	using GraphBase::target;

private:

	bool _isUndirected;

	Node _root;
};


/**
 * Selection of arcs, represented as bool attributes.
 */
class ArcSelection : public Graph::ArcMap<bool> {

public:

	ArcSelection(const Graph& graph) :
		Graph::ArcMap<bool>(graph) {}

	using Graph::ArcMap<bool>::operator[];

	/**
	 * Check whether any of the arcs of the given edge is contained in the 
	 * selection.
	 */
	bool operator[](const Edge& edge) const {

		for (const Arc& arc : edge)
			if ((*this)[arc])
				return true;

		return false;
	}
};

} // namespace host

#endif // HOST_GRAPHS_GRAPH_H__

