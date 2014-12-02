#ifndef HOST_CANDIDATE_CONFLICT_TERM_H__
#define HOST_CANDIDATE_CONFLICT_TERM_H__

#include <graphs/Graph.h>
#include "HigherOrderArcTerm.h"

namespace host {

class CandidateConflictTerm : public HigherOrderArcTerm {

public:

	/**
	 * Construct a candidate conflict term for the given graph.
	 */
	CandidateConflictTerm(const Graph& graph, const ArcTypes& arcTypes);

	/**
	 * Get the number of lambda parameters of this higher order term.
	 */
	size_t numLambdas() { return _exclusiveEdges.size() + _conflictArcs.size(); }

	/**
	 * Store the upper and lower bounds for each lambda in the the given ranges.
	 */
	void lambdaBounds(
			Lambdas::iterator beginLower,
			Lambdas::iterator endLower,
			Lambdas::iterator beginUpper,
			Lambdas::iterator endUpper);

	/**
	 * Set the lambda parameters.
	 */
	void setLambdas(Lambdas::const_iterator begin, Lambdas::const_iterator end);

	/**
	 * Add the lambda contributions of this higher order term to the given arc 
	 * weights.
	 */
	void addArcWeights(ArcWeights& weights);

	/**
	 * Get the constant contribution of this higher order term to the objective.
	 */
	double constant();

	/**
	 * For the given MST (represented as boolean flags on arcs), compute the 
	 * gradient for each lambda and store it in the range pointed to with the 
	 * given iterator.
	 */
	void gradient(
			const ArcSelection& mst,
			Lambdas::iterator          begin,
			Lambdas::iterator          end);

private:

	typedef std::vector<Arc>  Arcs;

	// an edge consists of all arcs between two nodes (i.e., one or two)
	class Edge {

	public:

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
	typedef std::vector<Edge> Edges;

	// list of exclusive arcs (not all of them can be selected at the same time) 
	// and the lambda value assoticated with them
	struct ExclusiveEdgesLambda {

		Edges  edges;
		double lambda;
	};

	// arc and a list of conflict arcs (no conflict arc can be selected, if arc 
	// is selected) and the lambda value associated with them
	struct ConflictArcsLambda {

		Arc    arc;
		Arcs   conflictArcs;
		double lambda;
	};

	// find pairs of mutual exclusive arcs
	void findExclusiveEdges(const ArcTypes& arcTypes);

	// find all link edges of a node
	Edges findLinkEdges(const Node& node, const ArcTypes& arcTypes);

	// find conflict nodes and add an ConflictArcsLambda for each incoming edge
	void findConflictArcs(const ArcTypes& arcTypes);

	inline std::string toString(const Arc& arc);
	inline std::string toString(const Arcs& arcs);
	inline std::string toString(const Edge& edge);
	inline std::string toString(const Edges& edges);

	// list of conflict nodes and their lambda values
	std::vector<ExclusiveEdgesLambda> _exclusiveEdges;
	std::vector<ConflictArcsLambda>   _conflictArcs;

	const Graph& _graph;
};

} // namespace host

#endif // HOST_CANDIDATE_CONFLICT_TERM_H__

