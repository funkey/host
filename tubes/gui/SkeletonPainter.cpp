#include "SkeletonPainter.h"
#include <gui/OpenGl.h>

bool
SkeletonPainter::draw(
		const util::rect<double>&,
		const util::point<double>&) {

	for (auto& p : *_skeletons)
		drawSkeleton(p.second);

	return false;
}

void
SkeletonPainter::setSkeletons(boost::shared_ptr<Skeletons> skeletons) {

	_skeletons = skeletons;

	setSize(
			_skeletons->getBoundingBox().getMinX(),
			_skeletons->getBoundingBox().getMinY(),
			_skeletons->getBoundingBox().getMaxX(),
			_skeletons->getBoundingBox().getMaxY());
}

void
SkeletonPainter::drawSkeleton(const Skeleton& skeleton) {

	glDisable(GL_DEPTH_TEST);
	glColor3f(0, 0, 0);

	for (Skeleton::Graph::EdgeIt e(skeleton.graph()); e != lemon::INVALID; ++e) {

		const Skeleton::Position& u = skeleton.positions()[skeleton.graph().u(e)];
		const Skeleton::Position& v = skeleton.positions()[skeleton.graph().v(e)];

		glBegin(GL_LINES);
		glVertex3d(u[0], u[1], u[2]);
		glVertex3d(v[0], v[1], v[2]);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
}
