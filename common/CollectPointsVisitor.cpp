
#include "CollectPointsVisitor.h"
#include <osg/Geometry>

class GetVertexArrayVisitor : public osg::ArrayVisitor
{
public:
	GetVertexArrayVisitor()
	{}

	virtual void apply(osg::Vec3Array& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			pts_.push_back(array.asVector()[i]);
		}
	}

	virtual void apply(osg::Vec4Array& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			osg::Vec4 v = array.asVector()[i];
			pts_.push_back(osg::Vec3(v.x(), v.y(), v.z()));
		}
	}

	virtual void apply(osg::Vec3dArray& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			pts_.push_back(array.asVector()[i]);
		}
	}

	virtual void apply(osg::Vec4dArray& array)
	{
		for (int i = 0; i < array.size(); i++)
		{
			osg::Vec4 v = array.asVector()[i];
			pts_.push_back(osg::Vec3(v.x(), v.y(), v.z()));
		}
	}

	std::vector<osg::Vec3> pts_;

protected:

	GetVertexArrayVisitor& operator = (const GetVertexArrayVisitor&) { return *this; }
};


CollectPointsVisitor::CollectPointsVisitor(bool bWorldSpace /*= true*/) 
	: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
	bWorldSpace_(bWorldSpace)
{
}

CollectPointsVisitor::~CollectPointsVisitor()
{
}

void CollectPointsVisitor::apply(osg::Geometry& geometry)
{
	if (!geometry.getVertexArray()) return;

	osg::Matrix mat = geometry.getWorldMatrices()[0];

	GetVertexArrayVisitor gav;
	geometry.getVertexArray()->accept(gav);

	if (bWorldSpace_)
	{
		for (auto& p : gav.pts_)
		{
			resultPts_.push_back(p * mat);
		}
	}
	else
	{
		for (auto& p : gav.pts_)
		{
			resultPts_.push_back(p);
		}
	}
}
