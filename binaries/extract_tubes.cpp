#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>

util::ProgramOption optionVolume(
		util::_long_name        = "volume",
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
}
