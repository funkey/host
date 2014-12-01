#ifndef HOST_INFERENCE_MULTI_EDGE_FACTORS_H__
#define HOST_INFERENCE_MULTI_EDGE_FACTORS_H__

#include <map>
#include <vector>
#include <graphs/Graph.h>

namespace host {

class MultiArcFactors {

public:

	typedef std::vector<host::Arc> Arcs;
	typedef std::map<Arcs,double>  Factors;

	typedef Factors::iterator       iterator;
	typedef Factors::const_iterator const_iterator;

	inline Factors::iterator begin() { return _factors.begin(); }
	inline Factors::iterator end()   { return _factors.end(); }
	inline Factors::const_iterator begin() const { return _factors.begin(); }
	inline Factors::const_iterator end()   const { return _factors.end(); }

	inline double& operator[](const Arcs& arcs) { return _factors[arcs]; }

	inline size_t size() const { return _factors.size(); }

private:

	Factors _factors;
};

} // namespace host

#endif // HOST_INFERENCE_MULTI_EDGE_FACTORS_H__

