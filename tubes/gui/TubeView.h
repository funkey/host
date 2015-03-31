#ifndef HOST_TUBES_GUI_TUBE_VIEW_H__
#define HOST_TUBES_GUI_TUBE_VIEW_H__

#include <scopegraph/Scope.h>
#include "SkeletonView.h"
#include "NormalsView.h"
#include <sg_gui/MeshView.h>
#include <sg_gui/VolumeView.h>
#include <sg_gui/KeySignals.h>

class TubeView :
		public sg::Scope<
				TubeView,
				sg::Accepts<
						sg_gui::KeyDown
				>,
				sg::ProvidesInner<
						sg_gui::ChangeAlpha
				>,
				sg::PassesUp<
						sg_gui::ContentChanged
				>
		> {

public:

	TubeView();

	void setTubeMeshes(std::shared_ptr<sg_gui::Meshes> meshes);

	void setTubeSkeletons(std::shared_ptr<Skeletons> skeletons);

	void setRawVolume(std::shared_ptr<ExplicitVolume<float>> volume);

	void onSignal(sg_gui::KeyDown& signal);

private:

	std::shared_ptr<SkeletonView>       _skeletonView;
	std::shared_ptr<NormalsView>        _normalsView;
	std::shared_ptr<sg_gui::MeshView>   _meshView;
	std::shared_ptr<sg_gui::VolumeView> _volumeView;

	double _alpha;
};

#endif // HOST_TUBES_GUI_TUBE_VIEW_H__

