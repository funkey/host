#include <vigra/multi_impex.hxx>
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

		ExplicitVolume<char> skeletonized = volumes[id];
		_skeletonize.skeletonize(skeletonized.data());

		if (id == 60) {

			vigra::MultiArray<3, float> tmp = skeletonized.data();
			vigra::exportVolume(
					tmp,
					vigra::VolumeExportInfo("debug/current_skeleton", ".tif"));
		}

		skeletons.insert(id, Skeleton(std::move(skeletonized)));
	}

	_store->saveSkeletons(skeletons);
}

