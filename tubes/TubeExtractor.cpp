#include <vigra/functorexpression.hxx>
#include "TubeExtractor.h"

void
TubeExtractor::extractFrom(vigra::MultiArray<3, int> labels) {

	std::map<TubeId, BoundingBox> bbs;

	// get the bounding boxes of all tubes
	for (int z = 0; z < labels.shape(2); z++)
	for (int y = 0; y < labels.shape(1); y++)
	for (int x = 0; x < labels.shape(0); x++) {

		TubeId id = labels(x, y, z);

		if (id == 0)
			continue;

		bbs[id] += BoundingBox(x, y, z, x+1, y+1, z+1);
	}

	// extract the tube volumes
	Volumes volumes;

	for (auto& p : bbs) {

		TubeId       id = p.first;
		BoundingBox& bb = p.second;

		// TODO: set resolution
		volumes[id].setBoundingBox(bb);
		vigra::MultiArray<3, char> data(vigra::Shape3(bb.width(), bb.height(), bb.depth()));

		// copy
		vigra::transformMultiArray(
				labels.subarray(
						vigra::Shape3(bb.getMinX(), bb.getMinY(), bb.getMinZ()),
						vigra::Shape3(bb.getMaxX(), bb.getMaxY(), bb.getMaxZ())),
				data,
				(vigra::functor::Arg1() == vigra::functor::Param(id)));

		volumes[id].data() = std::move(data);
	}

	// save them
	_store->saveVolumes(volumes);
}
