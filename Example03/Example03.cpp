// Example03.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

#include "../common/common.h"

int main(int argc, char** argv)
{

	// construct the viewer.
	osgViewer::Viewer viewer;

	osg::Group* root = new osg::Group;
	root->addChild(osgDB::readNodeFile("cow.osg"));

	add_event_handler(viewer);
	
	viewer.setSceneData(root);
	viewer.realize();
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	return viewer.run();
}
