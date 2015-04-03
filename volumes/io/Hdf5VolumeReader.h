#ifndef HOST_VOLUMES_IO_HDF5_VOLUME_READER_H__
#define HOST_VOLUMES_IO_HDF5_VOLUME_READER_H__

#include <string>
#include <vigra/hdf5impex.hxx>
#include <imageprocessing/ExplicitVolume.h>

class Hdf5VolumeReader {

public:

	Hdf5VolumeReader(vigra::HDF5File& hdfFile) :
		_hdfFile(hdfFile) {}

protected:

	template <typename ValueType>
	void readVolume(ExplicitVolume<ValueType>& volume, std::string dataset, bool onlyGeometry = false) {

		// the volume
		if (!onlyGeometry)
			_hdfFile.readAndResize(dataset, volume.data());

		vigra::MultiArray<1, float> bb(6);
		vigra::MultiArray<1, float> res(3);

		// bounding-box
		_hdfFile.readAttribute(
				dataset,
				"bounding box",
				bb);
		volume.setBoundingBox(
				util::box<float,3>(
						bb[0], bb[1], bb[2],
						bb[3], bb[4], bb[5]));

		// resolution
		_hdfFile.readAttribute(
				dataset,
				"resolution",
				res);
		volume.setResolution(res[0], res[1], res[2]);
	}

private:

	vigra::HDF5File& _hdfFile;
};

#endif // HOST_VOLUMES_IO_HDF5_VOLUME_READER_H__

