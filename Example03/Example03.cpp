// Example03.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//#undef  max
//#undef min

#include "pch.h"
#include "../common/common.h"
#include <random>

osg::Geode* createTriangle()
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<> urd(0., 1.);
	
	osg::Geode* geode = new osg::Geode;

	std::vector<osg::Vec3d> PTs;
	PTs.push_back(osg::Vec3());
	PTs.push_back(osg::Vec3(100, 0, 0));
	PTs.push_back(osg::Vec3(0, 100, 0));
	osg::Geometry* geom = createLine(PTs, osg::Vec4(1, 0, 0,1), osg::PrimitiveSet::LINE_LOOP);
	geode->addDrawable(geom);

	std::vector<osg::Vec3d> PTs2;
	for (int i = 0; i < 1000000; i++)
	{
		double r1 = urd(eng);
		double r2 = urd(eng);
		double sqrt_r1 = std::sqrt(r1);
		
		double u = 1 - sqrt_r1;
		double v = r2 * sqrt_r1;
		double w = 1 - u - v;

		osg::Vec3d p = PTs[0] * u + PTs[1] * v + PTs[2] * w;
		PTs2.push_back(p);
	}

	geom = createLine(PTs2, osg::Vec4(0, 1, 1, 1), osg::PrimitiveSet::POINTS);
	geode->addDrawable(geom);

	configureShaders(geode->getOrCreateStateSet());
	return geode;
}


int main(int argc, char** argv)
{
	// construct the viewer.
	osgViewer::Viewer viewer;

	osg::Group* root = new osg::Group;
	//root->addChild(osgDB::readNodeFile("cow.osg"));
	
	root->addChild(createTriangle());
	
	add_event_handler(viewer);
	viewer.setSceneData(root);
	viewer.realize();
	
	osg::setNotifyLevel(osg::NotifySeverity::WARN);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	return viewer.run();
}
