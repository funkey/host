#ifndef HOST_TUBES_FEATURE_EXTRACTOR_H__
#define HOST_TUBES_FEATURE_EXTRACTOR_H__

#include <tubes/io/TubeStore.h>

class FeatureExtractor {

public:

	FeatureExtractor(TubeStore* store) :
		_store(store) {}

	/**
	 * Extract features for a set of tubes given in one label image. The values 
	 * of the label image are interpreted as tube ids. This version is more 
	 * efficient then calling extractFrom() for a set of volumes.
	 */
	void extractFrom(
			vigra::MultiArrayView<3, float> intensities,
			vigra::MultiArrayView<3, int>   labels) {

		Features features;
		RegionFeatures<3, float, int> regionFeatures(intensities, labels);

		// Here we assume that the values in the labels volume match the region 
		// ids (which is true, for now).
		regionFeatures.fill(features);

		_store->saveFeatures(features);
		_store->saveFeatureNames(regionFeatures.getFeatureNames());
	}

	/**
	 * Extract features for a set of tubes given as individual volumes.
	 */
	void extractFrom(
			vigra::MultiArrayView<3, float> intensities,
			const Volumes&                  volumes) {

		// TODO
	}

private:

	TubeStore* _store;
};

#endif // HOST_TUBES_FEATURE_EXTRACTOR_H__

