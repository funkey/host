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

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glColor3f(0, 0, 0);

	for (auto& p : *_skeletons)
		drawSkeleton(p.second);

	glEnable(GL_DEPTH_TEST);
}

void
SkeletonView::onSignal(sg_gui::QuerySize& signal) {

	if (!_skeletons)
		return;

	signal.setSize(
			util::rect<double>(
				_skeletons->getBoundingBox().getMinX(),
				_skeletons->getBoundingBox().getMinY(),
				_skeletons->getBoundingBox().getMaxX(),
				_skeletons->getBoundingBox().getMaxY()));
}

void
SkeletonView::drawSkeleton(const Skeleton& skeleton) {

	for (Skeleton::Graph::NodeIt n(skeleton.graph()); n != lemon::INVALID; ++n) {

		const Skeleton::Position& u = skeleton.positions()[n];

		glBegin(GL_QUADS);

		glVertex3d(u[0]-10, u[1]-10, u[2]+10);
		glVertex3d(u[0]+10, u[1]-10, u[2]+10);
		glVertex3d(u[0]+10, u[1]+10, u[2]+10);
		glVertex3d(u[0]-10, u[1]+10, u[2]+10);

		glVertex3d(u[0]-10, u[1]-10, u[2]-10);
		glVertex3d(u[0]+10, u[1]-10, u[2]-10);
		glVertex3d(u[0]+10, u[1]+10, u[2]-10);
		glVertex3d(u[0]-10, u[1]+10, u[2]-10);

		glVertex3d(u[0]-10, u[1]+10, u[2]-10);
		glVertex3d(u[0]+10, u[1]+10, u[2]-10);
		glVertex3d(u[0]+10, u[1]+10, u[2]+10);
		glVertex3d(u[0]-10, u[1]+10, u[2]+10);

		glVertex3d(u[0]-10, u[1]-10, u[2]-10);
		glVertex3d(u[0]+10, u[1]-10, u[2]-10);
		glVertex3d(u[0]+10, u[1]-10, u[2]+10);
		glVertex3d(u[0]-10, u[1]-10, u[2]+10);

		glVertex3d(u[0]+10 ,u[1]-10, u[2]-10);
		glVertex3d(u[0]+10 ,u[1]+10, u[2]-10);
		glVertex3d(u[0]+10 ,u[1]+10, u[2]+10);
		glVertex3d(u[0]+10 ,u[1]-10, u[2]+10);

		glVertex3d(u[0]-10 ,u[1]-10, u[2]-10);
		glVertex3d(u[0]-10 ,u[1]+10, u[2]-10);
		glVertex3d(u[0]-10 ,u[1]+10, u[2]+10);
		glVertex3d(u[0]-10 ,u[1]-10, u[2]+10);

		glEnd();
	}

	for (Skeleton::Graph::EdgeIt e(skeleton.graph()); e != lemon::INVALID; ++e) {

		const Skeleton::Position& u = skeleton.positions()[skeleton.graph().u(e)];
		const Skeleton::Position& v = skeleton.positions()[skeleton.graph().v(e)];

		glBegin(GL_LINES);
		glVertex3d(u[0], u[1], u[2]);
		glVertex3d(v[0], v[1], v[2]);
		glEnd();
	}
}
