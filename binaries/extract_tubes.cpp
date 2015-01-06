#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <imageprocessing/io/ImageStackDirectoryReader.h>
#include <features/TubeFeatures.h>

util::ProgramOption optionIntensities(
		util::_long_name        = "intensities",
		util::_description_text = "A directory containing the intensity volume.",
		util::_default_value    = "intensities");

util::ProgramOption optionLabels(
		util::_long_name        = "labels",
		util::_description_text = "A directory containing the labeled volume.",
		util::_default_value    = "volume");

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to store the extracted tubes.",
		util::_default_value    = "project.hdf");

int main(int argc, char** argv) {

	util::ProgramOptions::init(argc, argv);
	logger::LogManager::init();

	pipeline::Process<ImageStackDirectoryReader> intensityReader(optionIntensities.as<std::string>());
	pipeline::Process<ImageStackDirectoryReader> labelReader(optionLabels.as<std::string>());

	pipeline::Value<ImageStack> intensityStack = intensityReader->getOutput();
	pipeline::Value<ImageStack> labelStack = labelReader->getOutput();

	TubeFeatures tubeFeatures;
	tubeFeatures.computeFeatures(*intensityStack, *labelStack);
}
