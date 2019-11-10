#pragma once
#include <osg/NodeVisitor>
#include <vector>

class CollectPointsVisitor : public osg::NodeVisitor
{
public:
	CollectPointsVisitor(bool bWorldSpace = true);
	~CollectPointsVisitor();

	virtual void apply(osg::Geometry& geometry);

public:
	std::vector<osg::Vec3> resultPts_;

private:
	bool bWorldSpace_ = true;
};

