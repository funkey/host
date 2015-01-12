#ifndef HOST_TUBES_IO_TUBE_STORE_H__
#define HOST_TUBES_IO_TUBE_STORE_H__

#include "Volumes.h"
#include "Features.h"

class TubeStore {

public:

	/**
	 * Store the given tube volumes.
	 */
	virtual void saveVolumes(const Volumes& volumes) = 0;

	/**
	 * Store the given tube features.
	 */
	virtual void saveFeatures(const Features& features) = 0;

	/**
	 * Store the names of the features.
	 */
	virtual void saveFeatureNames(const std::vector<std::string>& names) = 0;

	/**
	 * Get all tube ids that this store offers.
	 */
	virtual TubeIds getTubeIds() = 0;

	/**
	 * Get the volumes for the given tube ids and store them in the given 
	 * property map. If onlyGeometry is true, only the bounding boxes and voxel 
	 * resolutions of the volumes are retrieved.
	 */
	virtual void retrieveVolumes(const TubeIds& ids, Volumes& volumes, bool onlyGeometry = false) = 0;

	/**
	 * Get the features for the given tube ids and store them in the given 
	 * property map.
	 */
	virtual void retrieveFeatures(const TubeIds& ids, Features& features) = 0;
};

#endif // HOST_TUBES_IO_TUBE_STORE_H__

