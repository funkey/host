#include <boost/lexical_cast.hpp>
#include "Hdf5TubeStore.h"

void
Hdf5TubeStore::saveVolumes(const Volumes& volumes) {

	_hdfFile.root();
	_hdfFile.cd_mk("tubes");
	_hdfFile.cd_mk("volumes");

	vigra::MultiArray<1, float> bb(6);
	vigra::MultiArray<1, float> res(3);

	for (auto& p : volumes) {

		TubeId                      id     = p.first;
		const ExplicitVolume<char>& volume = p.second;

		// the volume
		_hdfFile.write(
				boost::lexical_cast<std::string>(id),
				volume.data());

		// bounding-box
		bb[0] = volume.getBoundingBox().getMinX();
		bb[1] = volume.getBoundingBox().getMinY();
		bb[2] = volume.getBoundingBox().getMinZ();
		bb[3] = volume.getBoundingBox().getMaxX();
		bb[4] = volume.getBoundingBox().getMaxY();
		bb[5] = volume.getBoundingBox().getMaxZ();
		_hdfFile.writeAttribute(
				boost::lexical_cast<std::string>(id),
				"bounding box",
				bb);

		// resolution
		res[0] = volume.getResolutionX();
		res[1] = volume.getResolutionY();
		res[2] = volume.getResolutionZ();
		_hdfFile.writeAttribute(
				boost::lexical_cast<std::string>(id),
				"resolution",
				res);
	}
}

void
Hdf5TubeStore::saveFeatures(const Features& features) {

	_hdfFile.root();
	_hdfFile.cd_mk("tubes");
	_hdfFile.cd_mk("features");

	for (auto& p : features) {

		TubeId                     id = p.first;
		const std::vector<double>& f  = p.second;

		_hdfFile.write(
				boost::lexical_cast<std::string>(id),
				vigra::ArrayVectorView<double>(f.size(), const_cast<double*>(&f[0])));
	}
}

void
Hdf5TubeStore::saveFeatureNames(const std::vector<std::string>& names) {

	_hdfFile.root();
	_hdfFile.cd_mk("tubes");
	_hdfFile.cd_mk("feature_names");

	for (const std::string& name : names)
		_hdfFile.mkdir(name);
}

TubeIds
Hdf5TubeStore::getTubeIds() {

	_hdfFile.cd("/volumes");

	TubeIds ids;
	for (std::string& idString : _hdfFile.ls())
		ids.add(boost::lexical_cast<TubeId>(idString));

	return ids;
}

void
Hdf5TubeStore::retrieveVolumes(const TubeIds& ids, Volumes& volumes, bool onlyGeometry) {

}

void
Hdf5TubeStore::retrieveFeatures(const TubeIds& ids, Features& features) {

}
