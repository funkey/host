#ifndef HOST_TUBES_IO_HDF5_GRAPH_READER_H__
#define HOST_TUBES_IO_HDF5_GRAPH_READER_H__

#include <vigra/hdf5impex.hxx>
#include <lemon/list_graph.h>

class Hdf5GraphReader {

public:

	typedef lemon::ListGraph Graph;

	template <typename ValueType>
	using NodeMap = Graph::NodeMap<ValueType>;
	template <typename ValueType>
	using EdgeMap = Graph::EdgeMap<ValueType>;

	Hdf5GraphReader(vigra::HDF5File& hdfFile) :
		_hdfFile(hdfFile) {}

	/**
	 * Read the graph structure in the current group with datasets "nodes" and 
	 * "edges".
	 */
	void readGraph(Graph& graph);

	/**
	 * Read a node map from a dataset with the given name. A converter object 
	 * needs to be provided to transform ArrayVectorView<T> objects into 
	 * ValueType. Converter has to define:
	 *
	 *   Converter::ArrayValueType
	 *
	 *     the expected type T of the ArrayVectorView<T>
	 *
	 *   Converter::ArraySize
	 *
	 *     the number of elements in the array
	 *
	 *   ValueType operator()(const ArrayVectorView<T>& a)
	 *
	 *     the conversion operator
	 */
	template <typename ValueType, typename Converter>
	void readNodeMap(
			const Graph&        graph,
			NodeMap<ValueType>& map,
			std::string         name,
			const Converter&    converter) {

		typedef vigra::ArrayVector<typename Converter::ArrayValueType> ArrayType;

		ArrayType values;

		if (_hdfFile.existsDataset(name))
			_hdfFile.readAndResize(
					name,
					values);

		std::size_t i = 0;
		for (Graph::NodeIt node(graph); node != lemon::INVALID; ++node) {

			map[node] = converter(values.subarray(i, i + Converter::ArraySize));
			i += Converter::ArraySize;
		}
	}

	/**
	 * Stores an edge map in a dataset with the given name.
	 */
	template <typename ValueType>
	void readEdgeMap(const EdgeMap<ValueType>& map, std::string name);
private:

	vigra::HDF5File& _hdfFile;
};

#endif // HOST_TUBES_IO_HDF5_GRAPH_READER_H__

