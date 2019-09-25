#pragma once
#include <osg/Drawable>
#include <osg/Object>
#include <osg/Array>

class CustomDrawable : public osg::Drawable
{
public:
	CustomDrawable() { }
	CustomDrawable(const CustomDrawable& drawable, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
		: osg::Drawable(drawable, copyop)
	{
		
	}

	META_Object(ose, CustomDrawable);

	virtual void drawImplementation(osg::RenderInfo&) const;

	virtual osg::BoundingBox computeBoundingBox() const;

private:
	void complie(osg::RenderInfo&) const;

	virtual ~CustomDrawable() {}

	mutable osg::Vec3Array* arr_ = nullptr;
	mutable unsigned int vao_, vbo_;
	mutable int program_;
	mutable bool dirty_ = true;
};

