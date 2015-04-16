#include "SkeletonView.h"
#include <sg_gui/OpenGl.h>

void
SkeletonView::setSkeletons(std::shared_ptr<Skeletons> skeletons) {

	_skeletons = skeletons;

	send<sg_gui::ContentChanged>();
}

void
SkeletonView::onSignal(sg_gui::Draw& /*draw*/) {

	if (!_skeletons)
		return;

	glColor3f(255, 53, 127);

	for (auto& p : *_skeletons)
		drawSkeleton(p.second);
}

void
SkeletonView::onSignal(sg_gui::QuerySize& signal) {

	if (!_skeletons)
		return;

	signal.setSize(_skeletons->getBoundingBox());
}

void
SkeletonView::onSignal(SetSkeletons& signal) {

	setSkeletons(signal.getSkeletons());
}

void
SkeletonView::drawSkeleton(const Skeleton& skeleton) {

	glLineWidth(2.0);
	glEnable(GL_LINE_SMOOTH);

	for (Skeleton::Graph::EdgeIt e(skeleton.graph()); e != lemon::INVALID; ++e) {

		Skeleton::Node u = skeleton.graph().u(e);
		Skeleton::Node v = skeleton.graph().v(e);

		Skeleton::Position pu = skeleton.positions()[u];
		Skeleton::Position pv = skeleton.positions()[v];

		util::point<float,3> ru;
		util::point<float,3> rv;
		skeleton.getRealLocation(pu, ru);
		skeleton.getRealLocation(pv, rv);

		glBegin(GL_LINES);
		glVertex3d(ru.x(), ru.y(), ru.z());
		glVertex3d(rv.x(), rv.y(), rv.z());
		glEnd();
	}
}
