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
		bb[0] = volume.getBoundingBox().min().x();
		bb[1] = volume.getBoundingBox().min().y();
		bb[2] = volume.getBoundingBox().min().z();
		bb[3] = volume.getBoundingBox().max().x();
		bb[4] = volume.getBoundingBox().max().y();
		bb[5] = volume.getBoundingBox().max().z();
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

