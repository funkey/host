#ifndef HOST_TUBES_IO_HDF_TUBE_STORE_H__
#define HOST_TUBES_IO_HDF_TUBE_STORE_H__

#include <vigra/hdf5impex.hxx>
#include "TubeStore.h"

class Hdf5TubeStore : public TubeStore {

public:

	Hdf5TubeStore(std::string projectFile) :
		_hdfFile(
				projectFile,
				vigra::HDF5File::OpenMode::ReadWrite) {}

	/**
	 * Store the given tube volumes.
	 */
	 void saveVolumes(const Volumes& volumes) override;

	/**
	 * Store the given tube features.
	 */
	 void saveFeatures(const Features& features) override;

	/**
	 * Store the names of the features.
	 */
	 void saveFeatureNames(const std::vector<std::string>& names) override;

	/**
	 * Store the given tube skeletons.
	 */
	void saveSkeletons(const Skeletons& skeletons) override;

	/**
	 * Get all tube ids that this store offers.
	 */
	 TubeIds getTubeIds() override;

	/**
	 * Get the volumes for the given tube ids and store them in the given 
	 * property map. If onlyGeometry is true, only the bounding boxes and voxel 
	 * resolutions of the volumes are retrieved.
	 */
	 void retrieveVolumes(const TubeIds& ids, Volumes& volumes, bool onlyGeometry = false) override;

	/**
	 * Get the features for the given tube ids and store them in the given 
	 * property map.
	 */
	 void retrieveFeatures(const TubeIds& ids, Features& features) override;

	/**
	 * Get the skeletons for the given tube ids and store them in the given 
	 * property map.
	 */
	void retrieveSkeletons(const TubeIds& ids, Skeletons& skeletons) override;

private:

	vigra::HDF5File _hdfFile;
};

#endif // HOST_TUBES_IO_HDF_TUBE_STORE_H__

