#include <vigra/multi_impex.hxx>
#include <vigra/multi_resize.hxx>
#include <vigra/functorexpression.hxx>
#include <util/Logger.h>
#include "SkeletonExtractor.h"

logger::LogChannel skeletonextractorlog("skeletonextractorlog", "[SkeletonExtractor] ");

void
SkeletonExtractor::extract() {

	TubeIds ids = _store->getTubeIds();

	Volumes volumes;
	_store->retrieveVolumes(ids, volumes);

	Skeletons skeletons;

	for (TubeId id : ids) {

		LOG_DEBUG(skeletonextractorlog)
				<< "processing tube " << id << std::endl;

		if (id == 149) {

			vigra::MultiArray<3, float> tmp = volumes[id].data();
			vigra::exportVolume(
					tmp,
					vigra::VolumeExportInfo("debug/tube", ".png"));
		}

		// skeletonize an isotropic version of the tube volume
		ExplicitVolume<unsigned char> skeletonized = makeIsotropic(volumes[id]);

		if (id == 149) {

			vigra::MultiArray<3, float> tmp = skeletonized.data();
			vigra::exportVolume(
					tmp,
					vigra::VolumeExportInfo("debug/pre_skeleton", ".png"));
		}

		_skeletonize.skeletonize(skeletonized.data());

		if (id == 149) {

			vigra::MultiArray<3, float> tmp = skeletonized.data();
			vigra::exportVolume(
					tmp,
					vigra::VolumeExportInfo("debug/skeleton", ".png"));

			//break;
		}

		try {

			skeletons.insert(id, Skeleton(std::move(skeletonized)));

		} catch (Exception& e) {

			UTIL_RETHROW(e, " (tube id = " << id << ")");
		}
	}

	_store->saveSkeletons(skeletons);
}

ExplicitVolume<unsigned char>
SkeletonExtractor::makeIsotropic(const ExplicitVolume<unsigned char>& volume) {

	// x or y is too small to be resampled, just create a copy
	if (volume.width() < 4 || volume.height() < 4)
		return volume;

	float isoRes = std::max(std::max(volume.getResolutionX(), volume.getResolutionY()), volume.getResolutionZ());

	// find the closest discretization (at least 4x4x1) for the isotropic volume
	unsigned int isoWidth  = std::max(4, (int)std::round((volume.getResolutionX()/isoRes)*volume.width()));
	unsigned int isoHeight = std::max(4, (int)std::round((volume.getResolutionY()/isoRes)*volume.height()));
	unsigned int isoDepth;
	// if z-dimension is too small to be resampled, we resample each 2D xy image 
	// separately
	if (volume.depth() < 4)
		isoDepth = volume.depth();
	// otherwise, z has to be at least 4
	else
		isoDepth = std::max(4, (int)std::round((volume.getResolutionZ()/isoRes)*volume.depth()));

	// adjust the resolution and bounding box, which changed due to 
	// discretization artefacts
	float resX = volume.getResolutionX()*((float)volume.width()/isoWidth);
	float resY = volume.getResolutionY()*((float)volume.height()/isoHeight);
	float resZ = volume.getResolutionZ()*((float)volume.depth()/isoDepth);
	BoundingBox bb = volume.getBoundingBox();
	bb.setMax(
			bb.getMinX() + resX*isoWidth,
			bb.getMinY() + resY*isoHeight,
			bb.getMinZ() + resZ*isoDepth);

	ExplicitVolume<unsigned char> isotropic(isoWidth, isoHeight, isoDepth);
	isotropic.setBoundingBox(bb);
	isotropic.setResolution(resX, resY, resZ);

	if (volume.depth() < 4)
		// z-slice-wise resampling
		for (unsigned int i = 0; i < volume.depth(); i++)
			vigra::resizeMultiArraySplineInterpolation(
					volume.data().bind<2>(i),
					isotropic.data().bind<2>(i));
	else
		vigra::resizeMultiArraySplineInterpolation(
				volume.data(),
				isotropic.data());

	return isotropic;
}
