// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include "CustomDrawable.h"

int main()
{
	osgViewer::Viewer view;

	osg::Group* root = new osg::Group;
	//root->addChild(osgDB::readNodeFile("cow.osg"));

	osg::Geode* geo = new osg::Geode;
	root->addChild(geo);
	geo->addDrawable(new CustomDrawable);
	
	view.setSceneData(root);
	add_event_handler(view);

	return view.run();
}

