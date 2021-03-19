#pragma once
#include <osgGA/StandardManipulator>
#include <osgViewer/Viewer>

extern osg::Camera* g_rttCamera;

class TwoDimManipulator : public osgGA::StandardManipulator
{
 public:
	TwoDimManipulator(osgViewer::Viewer* viewer);

	virtual osg::Matrixd getMatrix() const;
	virtual osg::Matrixd getInverseMatrix() const;
	virtual void		 setByMatrix(const osg::Matrixd& matrix);
	virtual void		 setByInverseMatrix(const osg::Matrixd& matrix);

	virtual void setTransformation(const osg::Vec3d&, const osg::Quat&) {}
	virtual void setTransformation(const osg::Vec3d&, const osg::Vec3d&, const osg::Vec3d&) {}
	virtual void getTransformation(osg::Vec3d&, osg::Quat&) const {}
	virtual void getTransformation(osg::Vec3d&, osg::Vec3d&, osg::Vec3d&) const {}

	virtual void home(double);
	virtual void home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) { home(ea.getTime()); }

 protected:
	virtual ~TwoDimManipulator();

	virtual bool handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;
	virtual bool handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;
	virtual bool handleMouseDrag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;
	virtual bool handleMouseWheel(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;

	void focusNode(osg::Node* geo);

	osg::Vec3 _center;
	double	_distance;
	osgViewer::Viewer*	_viewer;

	//计算拖拽用到的数据
	osg::Vec2 _preMousePt;
	int _buttonType = -1;
};