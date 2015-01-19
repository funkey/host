#include <boost/lexical_cast.hpp>
#include <util/Logger.h>
#include "Hdf5TubeStore.h"

logger::LogChannel hdf5storelog("hdf5storelog", "[Hdf5TubeStore] ");

void
Hdf5TubeStore::saveVolumes(const Volumes& volumes) {

	_hdfFile.root();
	_hdfFile.cd_mk("tubes");
	_hdfFile.cd_mk("volumes");

	for (auto& p : volumes) {

		TubeId                               id     = p.first;
		const ExplicitVolume<unsigned char>& volume = p.second;
		std::string                          name   = boost::lexical_cast<std::string>(id);

		writeVolume(volume, name);
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

void
Hdf5TubeStore::saveSkeletons(const Skeletons& skeletons) {

	_hdfFile.root();
	_hdfFile.cd_mk("tubes");
	_hdfFile.cd_mk("skeletons");

	for (auto& p : skeletons) {

		TubeId          id       = p.first;
		const Skeleton& skeleton = p.second;
		std::string     name     = boost::lexical_cast<std::string>(id);

		_hdfFile.cd_mk(name);

		PositionConverter positionConverter;

		writeGraph(skeleton.graph());
		writeNodeMap(skeleton.graph(), skeleton.positions(), "positions", positionConverter);
		// TODO:
		//writeEdgeMap(skeleton.segments(), "segments");

		_hdfFile.cd_up();
	}
}

TubeIds
Hdf5TubeStore::getTubeIds() {

	_hdfFile.cd("/tubes/volumes");

	TubeIds ids;
	for (std::string& idString : _hdfFile.ls())
		ids.add(boost::lexical_cast<TubeId>(idString));

	return ids;
}

void
Hdf5TubeStore::retrieveVolumes(const TubeIds& ids, Volumes& volumes, bool onlyGeometry) {

	_hdfFile.cd("/tubes/volumes");

	for (TubeId id : ids) {

		std::string name = boost::lexical_cast<std::string>(id);

		ExplicitVolume<unsigned char> volume;

		readVolume(volume, name, onlyGeometry);

		volumes.insert(id, std::move(volume));
	}
}

void
Hdf5TubeStore::retrieveFeatures(const TubeIds&, Features&) {

}

void
Hdf5TubeStore::retrieveSkeletons(const TubeIds& ids, Skeletons& skeletons) {

	_hdfFile.cd("/tubes/skeletons");

	for (TubeId id : ids) {

		std::string name = boost::lexical_cast<std::string>(id);

		_hdfFile.cd(name);

		Skeleton          skeleton;
		PositionConverter positionConverter;

		readGraph(skeleton.graph());
		readNodeMap(skeleton.graph(), skeleton.positions(), "positions", positionConverter);
		// TODO:
		//writeEdgeMap(skeleton.segments(), "segments");

		skeletons.insert(id, std::move(skeleton));

		_hdfFile.cd_up();
	}
}
