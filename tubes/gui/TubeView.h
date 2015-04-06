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

	void setLabelsVolume(std::shared_ptr<ExplicitVolume<float>> volume);

	void onSignal(sg_gui::KeyDown& signal);

private:

	/**
	 * Scope preventing change alpha signals to get to raw images.
	 */
	class RawScope : public sg::Scope<
			RawScope,
			sg::FiltersDown<
					sg_gui::ChangeAlpha
			>,
			sg::PassesUp<
					sg_gui::ContentChanged
			>
	> {

	public:

		bool filterDown(sg_gui::ChangeAlpha&) { return false; }
		void unfilterDown(sg_gui::ChangeAlpha&) {}
	};

	/**
	 * Scope preventing change alpha signals to get to label images, also 
	 * ignores depth buffer for drawing on top of raw images.
	 */
	class LabelsScope : public sg::Scope<
			LabelsScope,
			sg::FiltersDown<
					sg_gui::DrawTranslucent,
					sg_gui::ChangeAlpha
			>,
			sg::AcceptsInner<
					sg::AgentAdded
			>,
			sg::ProvidesInner<
					sg_gui::ChangeAlpha
			>,
			sg::PassesUp<
					sg_gui::ContentChanged
			>
	> {

	public:

		void onInnerSignal(sg::AgentAdded&) {

			sendInner<sg_gui::ChangeAlpha>(0.5);
		}

		bool filterDown(sg_gui::DrawTranslucent&) {

			glDisable(GL_DEPTH_TEST);

			return true;
		}

		void unfilterDown(sg_gui::DrawTranslucent&) {

			glEnable(GL_DEPTH_TEST);
		}

		bool filterDown(sg_gui::ChangeAlpha&) { return false; }
		void unfilterDown(sg_gui::ChangeAlpha&) {}
	};


	std::shared_ptr<SkeletonView>       _skeletonView;
	std::shared_ptr<NormalsView>        _normalsView;
	std::shared_ptr<sg_gui::MeshView>   _meshView;
	std::shared_ptr<sg_gui::VolumeView> _rawView;
	std::shared_ptr<sg_gui::VolumeView> _labelsView;

	double _alpha;
};

#endif // HOST_TUBES_GUI_TUBE_VIEW_H__

