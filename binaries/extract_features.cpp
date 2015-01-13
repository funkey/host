/**
 * This program reads tubes from an HDF5 file, computes their features, and 
 * stores the result in the same file.
 */

#include <iostream>

#include <vigra/hdf5impex.hxx>
#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <region_features/RegionFeatures.h>
#include <tubes/io/Hdf5TubeStore.h>
#include <tubes/FeatureExtractor.h>
#include <tubes/SkeletonExtractor.h>

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the label and intensity volume and store the features for each tube.",
		util::_default_value    = "project.hdf");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read the label and intensity volumes

		vigra::MultiArray<3, int>   labels;
		vigra::MultiArray<3, float> intensities;

		{
			vigra::HDF5File project(
					optionProjectFile.as<std::string>(),
					vigra::HDF5File::OpenMode::ReadWrite);

			project.cd("volume");
			project.readAndResize("intensities", intensities);
			project.readAndResize("labels", labels);
		}

		// create an hdf5 tube store

		Hdf5TubeStore tubeStore(optionProjectFile.as<std::string>());

		// extract and save tube features

		LOG_USER(logger::out) << "extracting features..." << std::endl;

		FeatureExtractor featureExtractor(&tubeStore);
		featureExtractor.extractFrom(intensities, labels);

		// extract and save tube skeletons

		LOG_USER(logger::out) << "extracting skeletons..." << std::endl;

		SkeletonExtractor skeletonExtractor(&tubeStore);
		skeletonExtractor.extract();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

