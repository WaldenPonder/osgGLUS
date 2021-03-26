#pragma once
#include <osg/NodeVisitor>

extern osg::ref_ptr<osg::Group> g_convexRoot;

class ConvexHullVisitor : public osg::NodeVisitor
{
public:
	ConvexHullVisitor();

	virtual void apply(osg::Geode& node);
};

