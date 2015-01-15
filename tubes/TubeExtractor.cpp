#include <vigra/functorexpression.hxx>
#include "TubeExtractor.h"

void
TubeExtractor::extractFrom(ExplicitVolume<int>& labels) {

	std::map<TubeId, BoundingBox> bbs;

	float resX = labels.getResolutionX();
	float resY = labels.getResolutionY();
	float resZ = labels.getResolutionZ();

	// get the bounding boxes of all tubes
	for (unsigned int z = 0; z < labels.depth();  z++)
	for (unsigned int y = 0; y < labels.height(); y++)
	for (unsigned int x = 0; x < labels.width();  x++) {

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

		// set volume properties
		volumes[id].setBoundingBox(bb*vigra::TinyVector<float, 3>(resX, resY, resZ));
		volumes[id].setResolution(
				labels.getResolutionX(),
				labels.getResolutionY(),
				labels.getResolutionZ());

		volumes[id].data() = vigra::MultiArray<3, char>(vigra::Shape3(bb.width(), bb.height(), bb.depth()));

		// copy data
		vigra::transformMultiArray(
				labels.data().subarray(
						vigra::Shape3(bb.getMinX(), bb.getMinY(), bb.getMinZ()),
						vigra::Shape3(bb.getMaxX(), bb.getMaxY(), bb.getMaxZ())),
				volumes[id].data(),
				(vigra::functor::Arg1() == vigra::functor::Param(id)));
	}

	// save them
	_store->saveVolumes(volumes);
}
