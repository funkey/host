/*
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

util::ProgramOption optionFormat(
		util::_long_name        = "format",
		util::_short_name       = "f",
		util::_description_text = "The format of the file to write: 'vtk' (vtk ASCII graph format), 'txt (simple text file). Default is 'txt'.",
		util::_default_value    = "txt");

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

		bool vtk = (optionFormat.as<std::string>() == "vtk");

		Skeletons skeletons;
		tubeStore.retrieveSkeletons(ids, skeletons);

		for (TubeId id : ids) {

			std::ofstream skeletonfile(std::string("skeleton_") + boost::lexical_cast<std::string>(id) + (vtk ? ".vtk" : ".txt"));

			if (vtk) {

				skeletonfile << "# vtk DataFile Version 3.0" << std::endl;
				skeletonfile << "vtk output" << std::endl;
				skeletonfile << "ASCII" << std::endl;
				skeletonfile << "DATASET UNDIRECTED_GRAPH" << std::endl;
			}

			const Skeleton& skeleton = skeletons[id];

			int numNodes = 0;
			int numEdges = 0;
			for (Skeleton::Graph::NodeIt node(skeleton.graph()); node != lemon::INVALID; ++node)
				numNodes++;
			for (Skeleton::Graph::EdgeIt edge(skeleton.graph()); edge != lemon::INVALID; ++edge)
				numEdges++;

			// nodes (number of)

			if (vtk)
				skeletonfile << "POINT " << numNodes << " float" << std::endl;
			else
				skeletonfile << numNodes << std::endl;

			// node positions (node x y z)
			for (int nodeId = 0; nodeId < numNodes; nodeId++) {

				Skeleton::Graph::Node node = skeleton.graph().nodeFromId(nodeId);

				if (!vtk)
					skeletonfile
							<< skeleton.graph().id(node) << "\t";

				skeletonfile
						<< skeleton.positions()[node].x() << "\t"
						<< skeleton.positions()[node].y() << "\t"
						<< skeleton.positions()[node].z() << std::endl;
			}

			if (vtk) {

				skeletonfile << "VERTICES " << numNodes << std::endl;
				skeletonfile << "EDGES " << numEdges << std::endl;
			}

			// edges (pairs of nodes)

			for (Skeleton::Graph::EdgeIt edge(skeleton.graph()); edge != lemon::INVALID; ++edge)
				skeletonfile
						<< skeleton.graph().id(skeleton.graph().u(edge)) << "\t"
						<< skeleton.graph().id(skeleton.graph().v(edge)) << std::endl;

			// node diameters (node d)

			if (vtk) {

				skeletonfile << "VERTEX_DATA " << numNodes << std::endl;
				skeletonfile << "FIELD FieldData 1" << std::endl;
				skeletonfile << "diameters 1 " << numNodes << " double" << std::endl;
			}
			for (int nodeId = 0; nodeId < numNodes; nodeId++) {

				Skeleton::Graph::Node node = skeleton.graph().nodeFromId(nodeId);
				skeletonfile
						<< skeleton.graph().id(node) << "\t"
						<< skeleton.diameters()[node] << std::endl;
			}
		}

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

