#ifndef HOST_TUBES_GUI_SKELETON_VIEW_H__
#define HOST_TUBES_GUI_SKELETON_VIEW_H__

#include <pipeline/SimpleProcessNode.h>
#include <tubes/Skeletons.h>
#include "SkeletonPainter.h"

class SkeletonView : public pipeline::SimpleProcessNode<> {

public:

	SkeletonView();

private:

	void updateOutputs();

	pipeline::Input<Skeletons>        _skeletons;
	pipeline::Output<SkeletonPainter> _painter;
};

#endif // HOST_TUBES_GUI_SKELETON_VIEW_H__

