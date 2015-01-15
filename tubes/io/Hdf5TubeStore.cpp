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

		std::vector<int>           nodes;
		std::vector<vigra::Shape3> positions;
		std::vector<int>           edges; // stored in pairs

		for (Skeleton::Graph::NodeIt node(skeleton.graph()); node != lemon::INVALID; ++node) {

			nodes.push_back(skeleton.graph().id(node));
			positions.push_back(skeleton.positions()[node]);
		}

		for (Skeleton::Graph::EdgeIt edge(skeleton.graph()); edge != lemon::INVALID; ++edge) {

			edges.push_back(skeleton.graph().id(skeleton.graph().u(edge)));
			edges.push_back(skeleton.graph().id(skeleton.graph().v(edge)));
		}

		_hdfFile.cd_mk(name);

		_hdfFile.write(
				"nodes",
				vigra::ArrayVectorView<int>(nodes.size(), &nodes[0]));
		_hdfFile.write(
				"positions",
				vigra::ArrayVectorView<vigra::Shape3::value_type>(positions.size()*3, &positions[0][0]));

		if (edges.size() > 0) {

			_hdfFile.write(
					"edges",
					vigra::ArrayVectorView<int>(edges.size(), &edges[0]));
			// TODO: segments
		}

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
Hdf5TubeStore::retrieveSkeletons(const TubeIds&, Skeletons&) {

}
