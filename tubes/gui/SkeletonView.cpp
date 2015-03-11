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

		Skeleton::Position prev;
		bool first = true;

		for (const Skeleton::Position& p : skeleton.segments()[e]) {

			if (first) {

				first = false;

			} else {

				glBegin(GL_LINES);
				glVertex3d(prev[0], prev[1], prev[2]);
				glVertex3d(p[0], p[1], p[2]);
				glEnd();
			}

			prev  = p;

			glBegin(GL_QUADS);

			glVertex3d(p[0]-5, p[1]-5, p[2]+5);
			glVertex3d(p[0]+5, p[1]-5, p[2]+5);
			glVertex3d(p[0]+5, p[1]+5, p[2]+5);
			glVertex3d(p[0]-5, p[1]+5, p[2]+5);

			glVertex3d(p[0]-5, p[1]-5, p[2]-5);
			glVertex3d(p[0]+5, p[1]-5, p[2]-5);
			glVertex3d(p[0]+5, p[1]+5, p[2]-5);
			glVertex3d(p[0]-5, p[1]+5, p[2]-5);

			glVertex3d(p[0]-5, p[1]+5, p[2]-5);
			glVertex3d(p[0]+5, p[1]+5, p[2]-5);
			glVertex3d(p[0]+5, p[1]+5, p[2]+5);
			glVertex3d(p[0]-5, p[1]+5, p[2]+5);

			glVertex3d(p[0]-5, p[1]-5, p[2]-5);
			glVertex3d(p[0]+5, p[1]-5, p[2]-5);
			glVertex3d(p[0]+5, p[1]-5, p[2]+5);
			glVertex3d(p[0]-5, p[1]-5, p[2]+5);

			glVertex3d(p[0]+5 ,p[1]-5, p[2]-5);
			glVertex3d(p[0]+5 ,p[1]+5, p[2]-5);
			glVertex3d(p[0]+5 ,p[1]+5, p[2]+5);
			glVertex3d(p[0]+5 ,p[1]-5, p[2]+5);

			glVertex3d(p[0]-5 ,p[1]-5, p[2]-5);
			glVertex3d(p[0]-5 ,p[1]+5, p[2]-5);
			glVertex3d(p[0]-5 ,p[1]+5, p[2]+5);
			glVertex3d(p[0]-5 ,p[1]-5, p[2]+5);

			glEnd();
		}
	}
}
