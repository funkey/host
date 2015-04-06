/**
 * This programs visualizes a tube stored in an hdf5 file.
 */

#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <tubes/gui/TubeView.h>
#include <tubes/io/Hdf5TubeStore.h>
#include <volumes/io/Hdf5VolumeStore.h>
#include <sg_gui/MarchingCubes.h>
#include <sg_gui/Mesh.h>
#include <sg_gui/RotateView.h>
#include <sg_gui/ZoomView.h>
#include <sg_gui/Window.h>

using namespace sg_gui;

util::ProgramOption optionProjectFile(
		util::_long_name        = "projectFile",
		util::_short_name       = "p",
		util::_description_text = "The project file to read the tube from.",
		util::_default_value    = "project.hdf");

util::ProgramOption optionTubeId(
		util::_long_name        = "id",
		util::_short_name       = "i",
		util::_description_text = "The ids of the tubes to show (separated by a single non-decimal character). If not given, all tubes are shown.");

util::ProgramOption optionCubeSize(
		util::_long_name        = "cubeSize",
		util::_description_text = "The size of a cube for the marching cubes visualization.",
		util::_default_value    = 10);

template <typename EV>
class ExplicitVolumeAdaptor {

public:

	typedef typename EV::value_type value_type;

	ExplicitVolumeAdaptor(const EV& ev) :
		_ev(ev) {}

	const util::box<float,3>& getBoundingBox() const { return _ev.getBoundingBox(); }

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

class RayView :
		public sg::Agent<
			RayView,
			sg::Accepts<
					sg_gui::MouseDown,
					sg_gui::Draw
			>,
			sg::Provides<
					sg_gui::ContentChanged
			>> {

public:

	void onSignal(sg_gui::MouseDown& signal) {

		if (signal.button != sg_gui::buttons::Right)
			return;

		_ray = signal.ray;
		send<ContentChanged>();

		std::cout << "ray at " << _ray.position() << std::endl;
	}

	void onSignal(sg_gui::Draw&) {

		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(
				_ray.position().x(),
				_ray.position().y(),
				_ray.position().z());
		glVertex3f(
				_ray.position().x() + 100*_ray.direction().x(),
				_ray.position().y() + 100*_ray.direction().y(),
				_ray.position().z() + 100*_ray.direction().z());
		glEnd();
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_LINES);
		glVertex3f(
				_ray.position().x() + 100*_ray.direction().x(),
				_ray.position().y() + 100*_ray.direction().y(),
				_ray.position().z() + 100*_ray.direction().z());
		glVertex3f(
				_ray.position().x() + 1000*_ray.direction().x(),
				_ray.position().y() + 1000*_ray.direction().y(),
				_ray.position().z() + 1000*_ray.direction().z());
		glEnd();
	}

private:

	util::ray<float,3> _ray;
};

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// create hdf5 stores

		Hdf5TubeStore   tubeStore(optionProjectFile.as<std::string>());
		Hdf5VolumeStore volumeStore(optionProjectFile.as<std::string>());

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

		// get tube volumes

		Volumes volumes;
		tubeStore.retrieveVolumes(ids, volumes);

		// get skeletons

		auto skeletons = std::make_shared<Skeletons>();
		tubeStore.retrieveSkeletons(ids, *skeletons);

		// volumes -> meshes

		auto meshes = std::make_shared<Meshes>();
		for (auto& p : volumes) {

			TubeId id                             = p.first;
			ExplicitVolume<unsigned char>& volume = p.second;

			typedef ExplicitVolumeAdaptor<ExplicitVolume<unsigned char>> Adaptor;
			Adaptor adaptor(volume);

			MarchingCubes<Adaptor> marchingCubes;
			std::shared_ptr<Mesh> mesh = marchingCubes.generateSurface(
					adaptor,
					MarchingCubes<Adaptor>::AcceptAbove(0.5),
					optionCubeSize,
					optionCubeSize,
					optionCubeSize);
			meshes->add(id, mesh);
		}

		// get intensities and labels

		auto intensities = std::make_shared<ExplicitVolume<float>>();
		volumeStore.retrieveIntensities(*intensities);
		ExplicitVolume<int> l;
		volumeStore.retrieveLabels(l);
		auto labels = std::make_shared<ExplicitVolume<float>>(l);

		// visualize

		auto tubeView     = std::make_shared<TubeView>();
		auto rotateView   = std::make_shared<RotateView>();
		auto zoomView     = std::make_shared<ZoomView>(true);
		auto window       = std::make_shared<sg_gui::Window>("tube viewer");
		auto rayView      = std::make_shared<RayView>();

		window->add(zoomView);
		zoomView->add(rotateView);
		rotateView->add(tubeView);
		rotateView->add(rayView);

		tubeView->setTubeMeshes(meshes);
		tubeView->setTubeSkeletons(skeletons);
		tubeView->setRawVolume(intensities);
		tubeView->setLabelsVolume(labels);

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}
