/**
 * This program reads a stack of images (a volume) that contains labels for 
 * tubes (i.e., pixels that are supposed to belong to the same neural process 
 * have the same label) and an intensity volume. For each tube, features are 
 * computed, like:
 *
 *   • statistical region features (size, mean intensity, histogram, ...)
 *   • shape descriptors (TODO)
 *   • bounding box (TODO)
 *   • skeleton (TODO)
 *   • loose ends (TODO)
 *
 * The features are stored in a HDF-file for further processing by other tools.
 */

#include <iostream>

#include <util/Logger.h>
#include <util/ProgramOptions.h>
#include <util/exceptions.h>
#include <region_features/RegionFeatures.h>
#include <vigra/hdf5impex.hxx>

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

		vigra::HDF5File project(
				optionProjectFile.as<std::string>(),
				vigra::HDF5File::OpenMode::ReadWrite);

		vigra::MultiArray<3, int>   labels;
		vigra::MultiArray<3, float> intensities;

		project.cd("volume");
		project.readAndResize("labels", labels);
		project.readAndResize("intensities", intensities);

		// extract tube features

		RegionFeatures<3, float, int> regionFeatures;
		regionFeatures.computeFeatures(intensities, labels);

		// save them in the project file

		project.root();
		project.cd_mk("feature_names");
		for (const std::string& name : regionFeatures.getFeatureNames())
			project.mkdir(name);

		project.root();
		project.cd_mk("features");

		for (auto& lf : regionFeatures.getFeatureMap()) {

			int label = lf.first;
			const std::vector<double>& features = lf.second;
			double* fp = const_cast<double*>(&features[0]);

			project.write(
					boost::lexical_cast<std::string>(label),
					vigra::ArrayVectorView<double>(features.size(), fp));
		}

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
