#ifndef HOST_FEATURES_TUBE_FEATURES_H__
#define HOST_FEATURES_TUBE_FEATURES_H__

#include <imageprocessing/ImageStack.h>
#include "RegionFeatures.h"
#include "LooseEnds.h"

class TubeFeatures {

public:

	void computeFeatures(
		const ImageStack& intensityStack,
		const ImageStack& labelStack);

	const std::vector<double>& getRegionFeatures(float tubeLabel) { return _regionFeatures.getFeatures(tubeLabel); }

	const std::vector<LooseEnd>& getLooseEnds(float tubeLabel) { return _looseEnds.getLooseEnds(tubeLabel); }

private:

	RegionFeatures<3, float, int> _regionFeatures;

	LooseEnds _looseEnds;
};

#endif // HOST_FEATURES_TUBE_FEATURES_H__

