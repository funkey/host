#ifndef HOST_TUBES_GUI_SKELETON_PAINTER_H__
#define HOST_TUBES_GUI_SKELETON_PAINTER_H__

#include <gui/Painter.h>
#include <tubes/Skeletons.h>

class SkeletonPainter : public gui::Painter {

public:

	bool draw(
			const util::rect<double>&  roi,
			const util::point<double>& resolution);

	void setSkeletons(boost::shared_ptr<Skeletons> skeletons);

private:

	void drawSkeleton(const Skeleton& skeleton);

	boost::shared_ptr<Skeletons> _skeletons;
};

#endif // HOST_TUBES_GUI_SKELETON_PAINTER_H__

