#ifndef HOST_VOLUMES_IO_HDF5_VOLUME_WRITER_H__
#define HOST_VOLUMES_IO_HDF5_VOLUME_WRITER_H__

#include <string>
#include <vigra/hdf5impex.hxx>
#include <imageprocessing/ExplicitVolume.h>

class Hdf5VolumeWriter {

public:

	Hdf5VolumeWriter(vigra::HDF5File& hdfFile) :
		_hdfFile(hdfFile) {}

protected:

	template <typename ValueType>
	void writeVolume(const ExplicitVolume<ValueType>& volume, std::string dataset) {

		// the volume
		_hdfFile.write(
				dataset,
				volume.data());

		vigra::MultiArray<1, float> bb(6);
		vigra::MultiArray<1, float> res(3);

		// bounding-box
		bb[0] = volume.getBoundingBox().getMinX();
		bb[1] = volume.getBoundingBox().getMinY();
		bb[2] = volume.getBoundingBox().getMinZ();
		bb[3] = volume.getBoundingBox().getMaxX();
		bb[4] = volume.getBoundingBox().getMaxY();
		bb[5] = volume.getBoundingBox().getMaxZ();
		_hdfFile.writeAttribute(
				dataset,
				"bounding box",
				bb);

		// resolution
		res[0] = volume.getResolutionX();
		res[1] = volume.getResolutionY();
		res[2] = volume.getResolutionZ();
		_hdfFile.writeAttribute(
				dataset,
				"resolution",
				res);
	}

private:

	vigra::HDF5File& _hdfFile;
};

#endif // HOST_VOLUMES_IO_HDF5_VOLUME_WRITER_H__

