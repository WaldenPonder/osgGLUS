#include "stdafx.h"
#include "DrawBoundingBox.h"
#include <random>
#include <osg/PolygonMode>
#include <osg/Geometry>
#include <osg/LineWidth>

THE_BEGIN

std::default_random_engine			  eng(time(NULL));
std::uniform_real_distribution<float> urd(0, 1);

DrawBoundingBox::DrawBoundingBox()
{

}


DrawBoundingBox::~DrawBoundingBox()
{
}

void DrawBoundingBox::add(const osg::BoundingBox& bb)
{
	float xMin = bb.xMin(), xMax = bb.xMax();
	float yMin = bb.yMin(), yMax = bb.yMax();
	float zMin = bb.zMin(), zMax = bb.zMax();

	//OSG_WARN << "DELTA: " << (xMax - xMin) << "\t" << (yMax - yMin) << "\t" << (zMax - zMin) << "\n";
	
	float r = urd(eng), g = urd(eng), b = urd(eng);
	for (int j = 0; j < 24; j++)
	{
		color_->push_back(osg::Vec4(r, g, b, 1));
	}

	//front
	vert_->push_back(osg::Vec3(xMin, yMin, zMin));
	vert_->push_back(osg::Vec3(xMax, yMin, zMin));
	vert_->push_back(osg::Vec3(xMax, yMin, zMax));
	vert_->push_back(osg::Vec3(xMin, yMin, zMax));

	//back
	vert_->push_back(osg::Vec3(xMin, yMax, zMin));
	vert_->push_back(osg::Vec3(xMax, yMax, zMin));
	vert_->push_back(osg::Vec3(xMax, yMax, zMax));
	vert_->push_back(osg::Vec3(xMin, yMax, zMax));

	//left
	vert_->push_back(osg::Vec3(xMin, yMin, zMin));
	vert_->push_back(osg::Vec3(xMin, yMax, zMin));
	vert_->push_back(osg::Vec3(xMin, yMax, zMax));
	vert_->push_back(osg::Vec3(xMin, yMin, zMax));

	//right
	vert_->push_back(osg::Vec3(xMax, yMin, zMin));
	vert_->push_back(osg::Vec3(xMax, yMax, zMin));
	vert_->push_back(osg::Vec3(xMax, yMax, zMax));
	vert_->push_back(osg::Vec3(xMax, yMin, zMax));

	//top
	vert_->push_back(osg::Vec3(xMin, yMin, zMax));
	vert_->push_back(osg::Vec3(xMax, yMin, zMax));
	vert_->push_back(osg::Vec3(xMax, yMax, zMax));
	vert_->push_back(osg::Vec3(xMin, yMax, zMax));

	//bottom
	vert_->push_back(osg::Vec3(xMin, yMin, zMax));
	vert_->push_back(osg::Vec3(xMax, yMin, zMax));
	vert_->push_back(osg::Vec3(xMax, yMax, zMax));
	vert_->push_back(osg::Vec3(xMin, yMax, zMax));
}

osg::Geode* DrawBoundingBox::get()
{
	if (geode_) return geode_;

	geode_ = new osg::Geode;

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geode_->addDrawable(geometry);

	geometry->setVertexArray(vert_);
	geometry->setColorArray(color_, osg::Array::BIND_PER_VERTEX);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vert_->size()));

	osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	geode_->getOrCreateStateSet()->setAttributeAndModes(model);
	geode_->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(2));

	return geode_;
}

THE_END