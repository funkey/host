#include <tests.h>
#include <vigra/multi_impex.hxx>
#include <imageprocessing/Skeletonize.h>

void skeletonization() {

	boost::filesystem::path dataDir = dir_of(__FILE__) / "data";

	vigra::VolumeImportInfo info(dataDir.native() + "/tube", ".tif");
	vigra::MultiArray<3, int> volume(info.shape());
	vigra::importVolume(info, volume);

	Skeletonize<int> skeletonize;
	skeletonize.skeletonize(volume);

	vigra::exportVolume(volume, "data/skeleton", ".tif");
}
