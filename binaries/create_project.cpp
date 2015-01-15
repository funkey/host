/**
 * This program reads a label image stack and an intensity image stack and 
 * created an HDF5 project file that can be used by subsequent tools (like 
 * extract_tubes).
 */

#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <imageprocessing/io/ImageStackDirectoryReader.h>
#include <volumes/io/Hdf5VolumeStore.h>

util::ProgramOption optionIntensities(
		util::_long_name        = "intensities",
		util::_description_text = "A directory containing the intensity volume.",
		util::_default_value    = "intensities");

util::ProgramOption optionLabels(
		util::_long_name        = "labels",
		util::_description_text = "A directory containing the labeled volume.",
		util::_default_value    = "labels");

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to store the label and intensity volumes.",
		util::_default_value    = "project.hdf");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		pipeline::Process<ImageStackDirectoryReader> intensityReader(optionIntensities.as<std::string>());
		pipeline::Process<ImageStackDirectoryReader> labelReader(optionLabels.as<std::string>());

		pipeline::Value<ImageStack> intensityStack = intensityReader->getOutput();
		pipeline::Value<ImageStack> labelStack = labelReader->getOutput();

		unsigned int width  = labelStack->width();
		unsigned int height = labelStack->height();
		unsigned int depth  = labelStack->size();

		if (width  != intensityStack->width() ||
			height != intensityStack->height() ||
			depth  != intensityStack->size())
			UTIL_THROW_EXCEPTION(
					UsageError,
					"intensity and label stacks have different sizes");

		// create volumes from stacks

		ExplicitVolume<float> intensities(*intensityStack);
		ExplicitVolume<int>   labels(*labelStack);

		// store them in the project file

		Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());

		volumeStore.saveIntensities(intensities);
		volumeStore.saveLabels(labels);

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
