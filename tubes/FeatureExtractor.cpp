#include <region_features/RegionFeatures.h>
#include "FeatureExtractor.h"

void
FeatureExtractor::extractFrom(
		vigra::MultiArrayView<3, float> intensities,
		vigra::MultiArrayView<3, int>   labels) {

	UTIL_ASSERT_REL(intensities.shape(), ==, labels.shape());

	Features features;
	RegionFeatures<3, float, int> regionFeatures(intensities, labels);

	// Here we assume that the values in the labels volume match the region 
	// ids (which is true, for now).
	regionFeatures.fill(features);

	_store->saveFeatures(features);
	_store->saveFeatureNames(regionFeatures.getFeatureNames());
}

void
FeatureExtractor::extractFrom(
		vigra::MultiArrayView<3, float> /*intensities*/,
		const Volumes&                  /*volumes*/) {

	UTIL_THROW_EXCEPTION(
			NotYetImplemented,
			"");
}
