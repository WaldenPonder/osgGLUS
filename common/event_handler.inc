#pragma once
#ifndef ADD_EVENT_HANDLERADD_EVENT_HANDLER
#define ADD_EVENT_HANDLERADD_EVENT_HANDLER

inline void add_event_handler(osgViewer::Viewer&   viewer)
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

#endif // !ADD_EVENT_HANDLERADD_EVENT_HANDLER
