/**
 * This program reads tubes from an HDF5 file, computes their features, and 
 * stores the result in the same file.
 */

#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <tubes/io/Hdf5TubeStore.h>

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the label and intensity volume and store the features for each tube.",
		util::_default_value    = "project.hdf");

util::ProgramOption optionTubeId(
		util::_long_name        = "id",
		util::_short_name       = "i",
		util::_description_text = "The ids of the tubes of which to export skeletons. If set to 'all', all skeletons are exported.");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// create an hdf5 tube store

		Hdf5TubeStore tubeStore(optionProjectFile.as<std::string>());

		TubeIds ids;

		if (optionTubeId && optionTubeId.as<std::string>() == "all") {

			ids = tubeStore.getTubeIds();

		} else if (optionTubeId) {

			std::stringstream ss(optionTubeId.as<std::string>());

			while (ss.good()) {

				TubeId id;
				ss >> id;
				ids.add(id);

				char sep;
				if (ss.good())
					ss >> sep;
			}
		}

		Skeletons skeletons;
		tubeStore.retrieveSkeletons(ids, skeletons);

		for (TubeId id : ids) {

			std::ofstream skeletonfile(std::string("skeleton_") + boost::lexical_cast<std::string>(id) + ".txt");

			const Skeleton& skeleton = skeletons[id];

			int numNodes = 0;
			for (Skeleton::Graph::NodeIt node(skeleton.graph()); node != lemon::INVALID; ++node)
				numNodes++;

			// nodes (number of)

			skeletonfile << numNodes << std::endl;

			// node positions (node x y z)
			for (Skeleton::Graph::NodeIt node(skeleton.graph()); node != lemon::INVALID; ++node)
				skeletonfile
						<< skeleton.graph().id(node) << "\t"
						<< skeleton.positions()[node].x() << "\t"
						<< skeleton.positions()[node].y() << "\t"
						<< skeleton.positions()[node].z() << std::endl;

			// node diameters (node d)
			for (Skeleton::Graph::NodeIt node(skeleton.graph()); node != lemon::INVALID; ++node)
				skeletonfile
						<< skeleton.graph().id(node) << "\t"
						<< skeleton.diameters()[node] << std::endl;

			// edges (pairs of nodes)

			for (Skeleton::Graph::EdgeIt edge(skeleton.graph()); edge != lemon::INVALID; ++edge)
				skeletonfile
						<< skeleton.graph().id(skeleton.graph().u(edge)) << "\t"
						<< skeleton.graph().id(skeleton.graph().v(edge)) << std::endl;
		}

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

