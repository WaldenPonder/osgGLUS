#include "pch.h"
#include "TwoDimManipulator.h"
#include <osg/ComputeBoundsVisitor>
#include <iostream>
#include "LineHole.h"

extern osg::ref_ptr <osg::Node> g_sceneNode;

namespace util {
	osg::Vec3 screenToWorld(osg::Camera* camera, const osg::Vec3& screenPoint)
	{
		osg::Vec3d vec3;
		osg::Matrix mVPW = camera->getViewMatrix() * camera->getProjectionMatrix() * camera->getViewport()->computeWindowMatrix();
		osg::Matrix invertVPW;
		invertVPW.invert(mVPW);
		vec3 = screenPoint * invertVPW;
		return vec3;
	}

	float screenWidthInWorld(osg::Camera* camera)
	{
		float ret;
		double x = camera->getViewport()->x();
		double y = camera->getViewport()->y();
		double w = camera->getViewport()->width();
		double h = camera->getViewport()->height();

		osg::Vec3 leftBottom = util::screenToWorld(camera, osg::Vec3(x, y, 0));
		osg::Vec3 rightBottom = util::screenToWorld(camera, osg::Vec3(x + w, y, 0));

		osg::Vec3 leftUp = util::screenToWorld(camera, osg::Vec3(x, y + h, 0));
		osg::Vec3 rightUp = util::screenToWorld(camera, osg::Vec3(x + w, y + h, 0));

		double len = (leftBottom - rightBottom).length();

		//if (len < 1e-5 || len > 10e15) return s_preScreenWidthInWorld;

		ret = len;

		return ret;
	}
}

TwoDimManipulator::TwoDimManipulator(osgViewer::Viewer* viewer) : _viewer(viewer), _distance(1.0)
{
}

TwoDimManipulator::~TwoDimManipulator()
{
}

osg::Matrixd TwoDimManipulator::getMatrix() const
{
	osg::Matrixd matrix;
	matrix.makeTranslate(0.0f, 0.0f, _distance);
	matrix.postMultTranslate(_center);
	return matrix;
}

osg::Matrixd TwoDimManipulator::getInverseMatrix() const
{
	osg::Matrixd matrix;
	matrix.makeTranslate(0.0f, 0.0f, -_distance);
	matrix.preMultTranslate(-_center);

	float w = getCamera()->getViewport()->width() * _distance;
	float h = getCamera()->getViewport()->height() * _distance;
	getCamera()->setProjectionMatrixAsOrtho(-w / 2, w / 2, -h / 2, h / 2, 0, 1e5);
	return matrix;
}

void TwoDimManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	setByInverseMatrix(osg::Matrixd::inverse(matrix));
}

void TwoDimManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{
	osg::Vec3d eye, center, up;
	matrix.getLookAt(eye, center, up);

	_distance = abs((eye - center).length());
}

//屏幕居中
void TwoDimManipulator::home(double)
{
	if (!_viewer || !g_mouseBoxPat) return;

	focusNode(g_sceneNode);
}

bool TwoDimManipulator::handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	_preMousePt = osg::Vec2(ea.getX(), ea.getY());
	_buttonType = ea.getButton();

	return __super::handleMousePush(ea, us);
}

bool TwoDimManipulator::handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	_buttonType = -1;
	return __super::handleMouseRelease(ea, us);
}

bool TwoDimManipulator::handleMouseDrag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	//鼠标中键
	if (_buttonType == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)
	{
		osg::Vec2 deltaPt = osg::Vec2(ea.getX(), ea.getY()) - _preMousePt;

		float w = getCamera()->getViewport()->width();
		float h = getCamera()->getViewport()->height();

		double screenWidthInWorld = util::screenWidthInWorld(getCamera());
		_center -= osg::Vec3(deltaPt[0] / w * screenWidthInWorld, deltaPt[1] / h * (screenWidthInWorld * (h / w)), 0);

		_preMousePt = osg::Vec2(ea.getX(), ea.getY());
		us.requestRedraw();
	}

	return __super::handleMouseDrag(ea, us);
}

bool TwoDimManipulator::handleMouseWheel(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	const float factor = 0.1;

	bool bHandle = false;
	if (ea.getEventType() == osgGA::GUIEventAdapter::SCROLL)
	{
		osg::Vec3 mousePt = util::screenToWorld(_viewer->getCamera(), osg::Vec3(ea.getX(), ea.getY(), 0));

		if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
		{
			_distance *= (1.0 - factor);
			_center -= ((_center - mousePt) * factor);
			bHandle = true;
		}
		else if (ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
		{
			_distance *= (1.0 + factor);

			_center += ((_center - mousePt) * factor);
			bHandle = true;
		}
	}
	return false;
}

void TwoDimManipulator::focusNode(osg::Node* geo)
{
	if (!geo) return;
	osg::BoundingSphere boundingSphere;

	//获取包围球大小
	osg::ComputeBoundsVisitor cbVisitor;
	cbVisitor.setTraversalMask(~NM_HUD);
	geo->accept(cbVisitor);
	osg::BoundingBox& bb = cbVisitor.getBoundingBox();
	boundingSphere.expandBy(bb);
	_center.set(boundingSphere.center());
	_center += osg::Z_AXIS * boundingSphere.radius();

	double viewPortW = getCamera()->getViewport()->width();
	double viewPortH = getCamera()->getViewport()->height();

	_distance = (1.8f * boundingSphere.radius()) / min(viewPortW, viewPortH);
	if (_distance <= 0)
		_distance = 0.001;
}

osg::Camera* TwoDimManipulator::getCamera() const
{
	return _viewer->getCamera();
}
