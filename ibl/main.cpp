// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>

//http://www.codinglabs.net/article_physically_based_rendering.aspx
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://metashapes.com/blog/realtime-image-based-lighting-using-spherical-harmonics/

int main()
{
	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
	
	osg::Node* cow = osgDB::readNodeFile("cow.osg");

	root->addChild(cow);

	view.setSceneData(root);
	add_event_handler(view);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
