#include "TubeFeatures.h"

void
TubeFeatures::computeFeatures(
		const ImageStack& intensityStack,
		const ImageStack& labelStack) {

	unsigned int width  = labelStack.width();
	unsigned int height = labelStack.height();
	unsigned int depth  = labelStack.size();

	vigra::MultiArray<3, float> intensityVolume(vigra::Shape3(width, height, depth));
	vigra::MultiArray<3, int>   labelVolume(vigra::Shape3(width, height, depth));

	// fill volumes image by image
	for (unsigned int i = 0; i < depth; i++) {

		vigra::copyMultiArray(
				*intensityStack[i],
				intensityVolume.bind<2>(i));
		vigra::copyMultiArray(
				*labelStack[i],
				labelVolume.bind<2>(i));
	}

	_regionFeatures.computeFeatures(intensityVolume, labelVolume);
	_looseEnds.findLooseEnds(labelVolume);
}
