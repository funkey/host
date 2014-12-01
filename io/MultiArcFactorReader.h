#ifndef HOST_IO_MULTI_EDGE_FACTOR_READER_H__
#define HOST_IO_MULTI_EDGE_FACTOR_READER_H__

#include <graphs/Graph.h>
#include <inference/MultiArcFactors.h>

namespace host {

class MultiArcFactorReader {

public:

	MultiArcFactorReader(std::string filename) :
		_filename(filename) {}

	void fill(
			const Graph&      graph,
			const ArcLabels& labels,
			MultiArcFactors& factors);

private:

	std::string _filename;
};

} // namesapce host

#endif // HOST_IO_MULTI_EDGE_FACTOR_READER_H__

