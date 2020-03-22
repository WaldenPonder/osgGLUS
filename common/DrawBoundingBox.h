#pragma once
#include "THEarthMacro.h"
#include <osg/BoundingBox>
#include <osg/Geode>

THE_BEGIN

//合并包围盒绘制
class THEARTH_API  DrawBoundingBox
{
public:
	DrawBoundingBox();
	~DrawBoundingBox();

	void add(const osg::BoundingBox& bb);
	osg::Geode* get();

private:
	osg::ref_ptr<osg::Geode> geode_;
	osg::ref_ptr<osg::Vec3Array> vert_ = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec4Array> color_ = new osg::Vec4Array;
};

THE_END