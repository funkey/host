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
	glColor3f(255, 53, 127);

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
				_skeletons->getBoundingBox().minX,
				_skeletons->getBoundingBox().minY,
				_skeletons->getBoundingBox().maxX,
				_skeletons->getBoundingBox().maxY));
}

void
SkeletonView::drawSkeleton(const Skeleton& skeleton) {

	glLineWidth(2.0);
	glEnable(GL_LINE_SMOOTH);

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
		}
	}
}
