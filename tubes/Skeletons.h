#ifndef HOST_TUBES_SKELETONS_H__
#define HOST_TUBES_SKELETONS_H__

#include <imageprocessing/Volume.h>
#include "TubePropertyMap.h"
#include "Skeleton.h"

class Skeletons : public TubePropertyMap<Skeleton>, public Volume {

protected:

	BoundingBox computeBoundingBox() const override {

		BoundingBox bb;

		for (auto& p : *this)
			bb += p.second.getBoundingBox();

		return bb;
	}
};

#endif // HOST_TUBES_SKELETONS_H__

