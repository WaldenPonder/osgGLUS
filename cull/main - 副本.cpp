// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"

osg::Group* createScene()
{
	osg::ref_ptr<osg::Group> parent = new osg::Group;

	//地形
	osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(), 1000, 1000, 10);
	{
		osg::ref_ptr<osg::ShapeDrawable> plane = new osg::ShapeDrawable(box);
		plane->setColor(osg::Vec4(1, 0, 0, 1));
		parent->addChild(plane);

		osg::StateSet* ss = plane->getOrCreateStateSet();
		ss->setRenderBinDetails(10, "RenderBin");
	}

	//用于遮挡地形的区域
	osg::ref_ptr<osg::Box> box2 = new osg::Box(osg::Vec3(), 100, 100, 20);
	{
		osg::ref_ptr<osg::ShapeDrawable> plane = new osg::ShapeDrawable(box2);
		plane->setColor(osg::Vec4(0, 1, 0, 0));
		parent->addChild(plane);

		osg::StateSet* ss = plane->getOrCreateStateSet();
		ss->setRenderBinDetails(9, "RenderBin");
		ss->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_WRITEMASK, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);		
	}
	
	osg::Node* cow = osgDB::readNodeFile("cow.osg");
	{
		osg::MatrixTransform* tran = new osg::MatrixTransform;
		tran->setMatrix(osg::Matrix::translate(osg::Vec3(0, 0, 0)));
		tran->addChild(cow);
		parent->addChild(tran);
	}
	

	return parent.release();
}

int main()
{
	osgViewer::Viewer view;
	osg::Group*		  root = new osg::Group;
	root->addChild(createScene());

	view.setSceneData(root);
	add_event_handler(view);

	osg::Camera* camera = view.getCamera();
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
