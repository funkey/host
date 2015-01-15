/**
 * This program reads a volume that contains labels for tubes (i.e., pixels that 
 * are supposed to belong to the same neural process have the same label) from 
 * an HDF5 file, and stores the tubes in the same file for further processing.
 */

#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <region_features/RegionFeatures.h>
#include <volumes/io/Hdf5VolumeStore.h>
#include <tubes/io/Hdf5TubeStore.h>
#include <tubes/TubeExtractor.h>

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the label volume and store the tubes.",
		util::_default_value    = "project.hdf");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read the label volume

		ExplicitVolume<int> labels;

		Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());
		volumeStore.retrieveLabels(labels);

		Hdf5TubeStore store(optionProjectFile.as<std::string>());
		TubeExtractor extractor(&store);
		extractor.extractFrom(labels);

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
