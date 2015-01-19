/**
 * This programs visualizes a tube stored in an hdf5 file.
 */

#include <util/ProgramOptions.h>
#include <pipeline/Process.h>
#include <pipeline/Value.h>
#include <imageprocessing/ExplicitVolume.h>
#include <tubes/gui/SkeletonView.h>
#include <tubes/io/Hdf5TubeStore.h>
#include <gui/ContainerView.h>
#include <gui/MarchingCubes.h>
#include <gui/Mesh.h>
#include <gui/MeshView.h>
#include <gui/OverlayPlacing.h>
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
		util::_description_text = "The ids of the tubes to show (separated by a single non-decimal character). If not given, all tubes are shown.");


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

		// create an hdf5 tube store

		Hdf5TubeStore tubeStore(optionProjectFile.as<std::string>());

		// get requested tube ids

		TubeIds ids;

		if (!optionTubeId) {

			ids = tubeStore.getTubeIds();

		} else {

			std::stringstream ss(optionTubeId.as<std::string>());

			while (ss.good()) {

				TubeId id;
				ss >> id;
				ids.add(id);

				char sep;
				if (ss.good())
					ss >> sep;
			}
		}

		// get volumes

		Volumes volumes;
		tubeStore.retrieveVolumes(ids, volumes);

		// get skeletons

		pipeline::Value<Skeletons> skeletons;
		tubeStore.retrieveSkeletons(ids, *skeletons);

		// volumes -> meshes

		pipeline::Value<Meshes> meshes;
		for (auto& p : volumes) {

			TubeId id                             = p.first;
			ExplicitVolume<unsigned char>& volume = p.second;

			typedef ExplicitVolumeAdaptor<ExplicitVolume<unsigned char>> Adaptor;
			Adaptor adaptor(volume);

			MarchingCubes<Adaptor> marchingCubes;
			boost::shared_ptr<Mesh> mesh = marchingCubes.generateSurface(
					adaptor,
					MarchingCubes<Adaptor>::AcceptAbove(0.5),
					10.0,
					10.0,
					10.0);
			meshes->add(id, mesh);
		}

		// visualize

		pipeline::Process<SkeletonView>                            skeletonView;
		pipeline::Process<MeshView>                                meshView;
		pipeline::Process<gui::ContainerView<gui::OverlayPlacing>> overlay;
		pipeline::Process<gui::RotateView>                         rotateView;
		pipeline::Process<gui::ZoomView>                           zoomView;
		pipeline::Process<gui::Window>                             window("tube viewer");

		meshView->setInput(meshes);
		skeletonView->setInput(skeletons);
		overlay->addInput(meshView->getOutput());
		overlay->addInput(skeletonView->getOutput());
		rotateView->setInput(overlay->getOutput());
		zoomView->setInput(rotateView->getOutput());
		window->setInput(zoomView->getOutput());

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
