/**
 * This programs visualizes a tube stored in an hdf5 file.
 */

#include <util/ProgramOptions.h>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <imageprocessing/ExplicitVolume.h>
#include <tubes/io/Hdf5TubeStore.h>
#include <gui/MarchingCubes.h>
#include <gui/Mesh.h>
#include <gui/MeshView.h>
#include <gui/RotateView.h>
#include <gui/ZoomView.h>
#include <gui/Window.h>

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the tube from.",
		util::_default_value    = "project.hdf");

util::ProgramOption optionTubeId(
		util::_long_name        = "id",
		util::_short_name       = "i",
		util::_description_text = "The id of the tube to show");


template <typename EV>
class ExplicitVolumeAdaptor {

public:

	typedef typename EV::value_type value_type;

	ExplicitVolumeAdaptor(const EV& ev) :
		_ev(ev) {}

	const BoundingBox& getBoundingBox() const { return _ev.getBoundingBox(); }

	float operator()(float x, float y, float z) const {

		if (!getBoundingBox().contains(x, y, z))
			return 0;

		unsigned int dx, dy, dz;

		_ev.getDiscreteCoordinates(x, y, z, dx, dy, dz);

		return _ev(dx, dy, dz);
	}

private:

	const EV& _ev;

};

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		if (!optionTubeId) {

			LOG_ERROR(logger::out) << "no tube id given" << std::endl;
			util::ProgramOptions::printUsage();
			return 1;
		}

		// create an hdf5 tube store

		Hdf5TubeStore tubeStore(optionProjectFile.as<std::string>());

		// get requested tube id

		TubeIds ids;
		ids.add(optionTubeId);

		// get volume

		Volumes volumes;
		tubeStore.retrieveVolumes(ids, volumes);

		ExplicitVolume<char>& volume = volumes[optionTubeId];

		LOG_USER(logger::out)
				<< "read volume with bb " << volume.getBoundingBox()
				<< " and resolution " << volume.getResolutionX()
				<< ", " << volume.getResolutionY()
				<< ", " << volume.getResolutionZ()
				<< std::endl;

		// get skeleton

		// volume -> mesh

		typedef ExplicitVolumeAdaptor<ExplicitVolume<char>> Adaptor;
		Adaptor adaptor(volume);

		MarchingCubes<Adaptor> marchingCubes;
		boost::shared_ptr<Mesh> mesh = marchingCubes.generateSurface(
				adaptor,
				MarchingCubes<Adaptor>::AcceptAbove(0.5),
				10.0,
				10.0,
				10.0);
		pipeline::Value<Meshes> meshes;
		meshes->add(1, mesh);

		// visualize

		pipeline::Process<MeshView>   meshView;
		pipeline::Process<gui::RotateView> rotateView;
		pipeline::Process<gui::ZoomView>   zoomView;
		pipeline::Process<gui::Window>     window("tube viewer");

		meshView->setInput(meshes);
		rotateView->setInput(meshView->getOutput());
		zoomView->setInput(rotateView->getOutput());
		window->setInput(zoomView->getOutput());

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
