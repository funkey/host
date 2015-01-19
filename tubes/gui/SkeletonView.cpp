#include "SkeletonView.h"

SkeletonView::SkeletonView() {

	registerInput(_skeletons, "skeletons");
	registerOutput(_painter, "painter");
}

void
SkeletonView::updateOutputs() {

	if (!_painter)
		_painter = new SkeletonPainter();

	_painter->setSkeletons(_skeletons.getSharedPointer());
}
