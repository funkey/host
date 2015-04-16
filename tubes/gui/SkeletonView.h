#ifndef HOST_TUBES_GUI_SKELETON_VIEW_H__
#define HOST_TUBES_GUI_SKELETON_VIEW_H__

#include <scopegraph/Agent.h>
#include <tubes/Skeletons.h>
#include <sg_gui/GuiSignals.h>

class SetSkeletons : public sg_gui::SetContent {

public:

	typedef sg_gui::SetContent parent_type;

	SetSkeletons(std::shared_ptr<Skeletons> skeletons) :
			_skeletons(skeletons) {}

	std::shared_ptr<Skeletons> getSkeletons() { return _skeletons; }

private:

	std::shared_ptr<Skeletons> _skeletons;
};

class SkeletonView :
		public sg::Agent<
				SkeletonView,
				sg::Accepts<
						sg_gui::Draw,
						sg_gui::QuerySize,
						SetSkeletons
				>,
				sg::Provides<
						sg_gui::ContentChanged
				>
		> {

public:

	void setSkeletons(std::shared_ptr<Skeletons> skeletons);

	void onSignal(sg_gui::Draw& draw);

	void onSignal(sg_gui::QuerySize& signal);

	void onSignal(SetSkeletons& signal);

private:

	void drawSkeleton(const Skeleton& skeleton);

	std::shared_ptr<Skeletons> _skeletons;
};

#endif // HOST_TUBES_GUI_SKELETON_VIEW_H__

