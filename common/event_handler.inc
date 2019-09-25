#pragma once

void add_event_handler(osgViewer::Viewer&   v);

osg::Geometry* createLine(const std::vector<osg::Vec3d>& allPTs, osg::Vec4 color, osg::PrimitiveSet::Mode model, float nWidth);

class RemovePloyVisitor : public osg::NodeVisitor
{
 public:
	RemovePloyVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) {}

	virtual void apply(osg::Geometry& geometry)
	{
		for (osg::ref_ptr<osg::PrimitiveSet>& ps : geometry.getPrimitiveSetList())
		{
			// ps->setMode(mode_ % (osg::PrimitiveSet::PATCHES + 1));
		}

		geometry.dirtyGLObjects();
	}

	int mode_;
};

void add_event_handler(osgViewer::Viewer&   viewer)
{
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()));

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()));

	// add the thread model handler
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the LOD Scale handler
	viewer.addEventHandler(new osgViewer::LODScaleHandler);

	// add the screen capture handler
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);
}
