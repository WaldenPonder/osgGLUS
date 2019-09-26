#pragma once
#include <osg/NodeVisitor>
#include <vector>

class CollectLinesVisitor : public osg::NodeVisitor
{
public:
	CollectLinesVisitor(bool bWorldSpace = true);
	~CollectLinesVisitor();

	virtual void apply(osg::Geometry& geometry);

	struct Line
	{
		osg::Vec3 start, end;
	};

	std::vector<Line> lines_;

private:
	bool bWorldSpace_ = true;
};
