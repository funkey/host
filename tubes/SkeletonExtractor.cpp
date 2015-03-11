#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include "SkeletonExtractor.h"
#define WITH_LEMON
#include <vigra/multi_impex.hxx>
#include <vigra/multi_resize.hxx>
#include <vigra/multi_watersheds.hxx>
#include <vigra/functorexpression.hxx>

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

		Skeletonize skeletonize(volumes[id]);
		skeletons.insert(id, skeletonize.getSkeleton());
	}

	_store->saveSkeletons(skeletons);
}

