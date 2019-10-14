// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "stdafx.h"
#include "../common/common.h"
#include "CustomDrawable.h"

int main()
{
	osgViewer::Viewer view;

	osg::Group* root = new osg::Group;

	osg::Geode* geo = new osg::Geode;
	root->addChild(geo);
	geo->addDrawable(new CustomDrawable);
	
	view.setSceneData(root);
	add_event_handler(view);

	const int width(800), height(450);
	const std::string version("3.3");
	osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
	traits->x = 20; traits->y = 30;
	traits->width = width; traits->height = height;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->glContextVersion = version;
	traits->readDISPLAY();
	traits->setUndefinedScreenDetailsToDefaultScreen();
	osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (!gc.valid())
	{
		osg::notify(osg::FATAL) << "Unable to create OpenGL v" << version << " context." << std::endl;
		return(1);
	}

	osg::Camera* cam = view.getCamera();
	cam->setGraphicsContext(gc.get());
	cam->setViewport(new osg::Viewport(0, 0, width, height));


	return view.run();
}

