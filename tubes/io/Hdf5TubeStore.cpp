#include <boost/lexical_cast.hpp>
#include <util/Logger.h>
#include "Hdf5TubeStore.h"

logger::LogChannel hdf5storelog("hdf5storelog", "[Hdf5TubeStore] ");

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

	vigra::MultiArray<1, float> bb(6);
	vigra::MultiArray<1, float> res(3);

	for (TubeId id : ids) {

		std::string name = boost::lexical_cast<std::string>(id);

		ExplicitVolume<char> volume;

		// the volume
		if (!onlyGeometry)
			_hdfFile.readAndResize(name, volume.data());

		LOG_DEBUG(hdf5storelog) << "read volume of size " << volume.data().shape() << std::endl;

		// bounding-box
		_hdfFile.readAttribute(
				name,
				"bounding box",
				bb);
		volume.getBoundingBox().setMin(bb[0], bb[1], bb[2]);
		volume.getBoundingBox().setMax(bb[3], bb[4], bb[5]);

		// resolution
		_hdfFile.readAttribute(
				name,
				"resolution",
				res);
		volume.setResolution(res[0], res[1], res[2]);

		volumes.insert(id, std::move(volume));
	}
}

void
Hdf5TubeStore::retrieveFeatures(const TubeIds&, Features&) {

}

void
Hdf5TubeStore::retrieveSkeletons(const TubeIds&, Skeletons&) {

}
