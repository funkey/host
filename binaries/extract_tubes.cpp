/**
 * This program reads a volume that contains labels for tubes (i.e., pixels that 
 * are supposed to belong to the same neural process have the same label) from 
 * an HDF5 file, and stores the tubes in the same file for further processing.
 */

#include <iostream>

#include <vigra/hdf5impex.hxx>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <region_features/RegionFeatures.h>
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

		vigra::MultiArray<3, int>   labels;

		{
			vigra::HDF5File project(
					optionProjectFile.as<std::string>(),
					vigra::HDF5File::OpenMode::ReadWrite);

			project.cd("volume");
			project.readAndResize("labels", labels);
		}

		Hdf5TubeStore store(optionProjectFile.as<std::string>());
		TubeExtractor extractor(&store);
		extractor.extractFrom(labels);

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
