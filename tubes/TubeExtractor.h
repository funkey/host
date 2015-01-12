#ifndef HOST_TUBES_TUBE_EXTRACTOR_H__
#define HOST_TUBES_TUBE_EXTRACTOR_H__

#include <tubes/io/TubeStore.h>

/**
 * Extracts tubes from a source like a label image and saves their volumes with 
 * metadata (bounding box, resolution) using the given store.
 */
class TubeExtractor {

public:

	TubeExtractor(TubeStore* store) :
		_store(store) {}

	/**
	 * Extract tubes from a label image. The values of the label image are 
	 * interpreted as tube ids.
	 */
	void extractFrom(vigra::MultiArray<3, int> labels);

private:

	TubeStore* _store;
};

#endif // HOST_TUBES_TUBE_EXTRACTOR_H__

