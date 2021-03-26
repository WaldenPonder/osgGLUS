#pragma once
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osgDB/ReadFile"
#include <osg/TextureBuffer>
#include "osg/Depth"
#include "osg/LineStipple"
#include "osg/Math"
using namespace std;

namespace osgViewer
{
	class Viewer;
};

extern osg::ref_ptr <osg::MatrixTransform> g_sceneNode;
extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

extern osgViewer::Viewer* g_viewer;
extern bool g_is_top_view;
extern int TEXTURE_SIZE1;
extern int TEXTURE_SIZE2;

extern osg::Group* g_root;
extern osg::BoundingBox g_line_bbox;
extern osg::Vec4 CLEAR_COLOR;

extern osg::Geode* g_hidden_line_geode;
extern bool g_is_orth_camera;
extern osg::PositionAttitudeTransform* g_mouseBoxPat;
extern bool g_line_hole_enable;

struct RenderPass
{
	osg::Texture2D* baseColorTexture = nullptr;
	osg::Texture2D* depthTexture = nullptr;
	osg::Texture2D* idTexture = nullptr;
	osg::Texture2D* linePtTexture = nullptr;
	osg::Camera* rttCamera = nullptr;
};

extern RenderPass g_linePass;
extern RenderPass g_facePass;

#define NM_HUD (1 << 2)
#define NM_HIDDEN_LINE (1 << 3)
#define NM_FACE (1 << 4)
#define NM_OUT_LINE (1 << 5)
#define NM_LINE (1 << 6)
#define NM_HIDE_OBJECT (1 << 7)
#define NM_ALL (~0)

//越小越先画，默认0, 面要最先画,  实体线第二   虚线最后
namespace RenderPriority 
{
	static const int FACE = 1; //面
	static const int DOT_LINE = 2; //虚线
	static const int LINE = 3; //实线
	static const int OUT_LINE = 4; //轮廓线
};

//------------------------------------------------------------------------------------------LineHole
class LineHole
{
public:
	static osg::Camera* createLineRttCamera(osgViewer::Viewer* viewer);
	static osg::Camera* createFaceRttCamera(osgViewer::Viewer* viewer);

	static osg::ref_ptr<osg::TextureBuffer> create_tbo(const vector<int>& data);

	//最终显示的贴图
	static osg::Camera* createHudCamera(osgViewer::Viewer* viewer);

	//虚线
	static void setUpHiddenLineStateset(osg::StateSet* ss, osg::Camera* camera);

	static void setUpStateset(osg::StateSet* ss, osg::Camera* camera, bool isLine = true);

	static osg::Geometry* createFinalHudTextureQuad(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner, const osg::Vec3& widthVec,
		const osg::Vec3& heightVec, float l = 0, float b = 0, float r = 1, float t = 1);

	static osg::Geometry* myCreateTexturedQuadGeometry2(osg::Camera* camera, int id, const osg::Vec3& corner, const osg::Vec3& widthVec,
		const osg::Vec3& heightVec, float l = 0, float b = 0, float r = 1, float t = 1);

	static osg::Geometry* createTriangles(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
		const std::vector<int>& ids, osg::Camera* camera);

	static osg::Geometry* createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
		const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode = osg::PrimitiveSet::LINE_LOOP);

	static osg::Uniform* getOrCreateMVPUniform(osg::Camera* camera);
};

//------------------------------------------------------------------------------------------CameraPredrawCallback
class CameraPredrawCallback : public osg::Camera::DrawCallback
{
public:
	osg::observer_ptr<osg::Camera> rttCamera;
	osg::observer_ptr<osg::Camera> mainCamera;

	CameraPredrawCallback(osg::Camera* first, osg::Camera* main) : rttCamera(first), mainCamera(main) {}
	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		osg::Viewport* vp = mainCamera->getViewport();

		if (rttCamera.get())
		{
			rttCamera->setProjectionMatrix(mainCamera->getProjectionMatrix());
			rttCamera->setViewMatrix(mainCamera->getViewMatrix());

			osg::ref_ptr<osg::RefMatrix> proMat = new osg::RefMatrix(rttCamera->getProjectionMatrix());
			renderInfo.getState()->applyProjectionMatrix(proMat);
			renderInfo.getState()->applyModelViewMatrix(rttCamera->getViewMatrix());
		}
	}
};

//------------------------------------------------------------------------------------------MVPCallback
class MVPCallback : public osg::Uniform::Callback
{
public:
	MVPCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		osg::Matrixd modelView = mCamera->getViewMatrix();
		osg::Matrixd projectM = mCamera->getProjectionMatrix();
		osg::Matrixd  mvp = modelView * projectM;
		osg::Matrix mat = mvp;
		uniform->set(mat);
	}

private:
	osg::Camera* mCamera;
};

//------------------------------------------------------------------------------------------OutRangeCallback
class OutRangeCallback : public osg::Uniform::Callback
{
public:
	OutRangeCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		if (!mCamera->getViewport()) return;

		osg::Matrix mat;

		//透视如果不考虑view matrix, 会算出来一个很大的range, 崩溃在nv dll
		if (!g_is_orth_camera)
			mat = mCamera->getViewMatrix();

		osg::Matrixd  mvp = mat *  mCamera->getProjectionMatrix() * mCamera->getViewport()->computeWindowMatrix();
		float sz = 10;
		osg::Vec3d p1(-sz, 0, 1);
		osg::Vec3d p2(sz, 0, 1);

		float range = (p1 * mvp - p2 * mvp).length();
		//std::cout << range << "\n";
		if (range > 70) range = 70;
		uniform->set(range);
	}

private:
	osg::Camera* mCamera;
};

//------------------------------------------------------------------------------------------InnerRangeCallback
class InnerRangeCallback : public osg::Uniform::Callback
{
public:
	InnerRangeCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		if (!mCamera->getViewport()) return;

		osg::Matrix mat;
		if (!g_is_orth_camera)
			mat = mCamera->getViewMatrix();

		osg::Matrixd  mvp = mat * mCamera->getProjectionMatrix() * mCamera->getViewport()->computeWindowMatrix();

		float sz = 10;
		osg::Vec3d p1(-sz, 0, 1);
		osg::Vec3d p2(sz, 0, 1);

		float range = (p1 * mvp - p2 * mvp).length();
		//std::cout << range << "\n";
		if (range > 70) range = 70;
		uniform->set(range);
	}

private:
	osg::Camera* mCamera;
};

//------------------------------------------------------------------------------------------LineHoleCallback
class LineHoleCallback : public osg::Uniform::Callback
{
public:
	LineHoleCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(g_line_hole_enable);
	}

private:
	osg::Camera* mCamera;
};