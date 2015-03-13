#ifndef HOST_TUBES_SKELETON_EXTRACTOR_H__
#define HOST_TUBES_SKELETON_EXTRACTOR_H__

#include <tubes/io/TubeStore.h>
#include <imageprocessing/ExplicitVolume.h>
#include <imageprocessing/Skeletonize.h>
#include "Skeletons.h"

class SkeletonExtractor {

public:

	SkeletonExtractor(TubeStore* store) :
		_store(store) {}

	/**
	 * Extract the skeletons for all tubes in the given store, and store them in 
	 * the same store.
	 */
	void extract();

private:

	TubeStore*  _store;
};

#endif // HOST_TUBES_SKELETON_EXTRACTOR_H__

