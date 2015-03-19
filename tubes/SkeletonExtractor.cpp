#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "SkeletonExtractor.h"
#define WITH_LEMON
#include <vigra/multi_impex.hxx>
#include <vigra/multi_resize.hxx>
#include <vigra/multi_watersheds.hxx>
#include <vigra/functorexpression.hxx>
#include <util/timing.h>

logger::LogChannel skeletonextractorlog("skeletonextractorlog", "[SkeletonExtractor] ");

void
SkeletonExtractor::extract() {

	TubeIds ids = _store->getTubeIds();
	//TubeIds ids;
	//ids.add(331);

	Volumes volumes;
	_store->retrieveVolumes(ids, volumes);

	Skeletons skeletons;

	for (TubeId id : ids) {

		LOG_DEBUG(skeletonextractorlog)
				<< "processing tube " << id << std::endl;

		try {

			Timer t("skeletonize volume");

			Skeletonize skeletonize(volumes[id]);
			skeletons.insert(id, skeletonize.getSkeleton());

		} catch (NoNodeFound& e) {

			LOG_USER(skeletonextractorlog)
					<< "tube " << id
					<< " could not be skeletonized (NoNodeFound)"
					<< std::endl;
		}
	}

	_store->saveSkeletons(skeletons);
}

