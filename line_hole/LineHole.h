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

extern osg::Texture2D* g_texture;
extern osg::Texture2D* g_depthTexture;
extern osg::Texture2D* g_idTexture;
extern osg::Texture2D* g_linePtTexture;
extern osg::Camera* g_rttCamera;
extern osg::ref_ptr <osg::Node> g_sceneNode;
extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;
extern osgViewer::Viewer* g_viewer;
#define TEXTURE_SIZE1 1024
#define TEXTURE_SIZE2 1024

extern osg::Group* g_root;
extern osg::BoundingBox g_line_bbox;
extern osg::Vec4 CLEAR_COLOR;// (204 / 255, 213 / 255, 240 / 255, 1);

extern osg::Geode* g_hidden_line_geode;
extern bool g_is_orth_camera;
extern osg::PositionAttitudeTransform* g_mouseBoxPat;

#define NM_HUD (1 << 2)

//------------------------------------------------------------------------------------------LineHole
class LineHole
{
public:
	static std::vector<osg::Texture2D*> createRttCamera(osgViewer::Viewer* viewer);
	static osg::ref_ptr<osg::TextureBuffer> create_tbo(const vector<int>& data);

	//最终显示的贴图
	static osg::Camera* createHudCamera(osgViewer::Viewer* viewer, std::vector<osg::Texture2D*> TEXTURES);

	//虚线
	static void setUpHiddenLineStateset(osg::StateSet* ss, osg::Camera* camera);

	static void setUpStateset(osg::StateSet* ss, osg::Camera* camera);

	static osg::Geometry* createFinalHudTextureQuad(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner, const osg::Vec3& widthVec,
		const osg::Vec3& heightVec, float l = 0, float b = 0, float r = 1, float t = 1);

	static osg::Geometry* myCreateTexturedQuadGeometry2(osg::Camera* camera, int id, const osg::Vec3& corner, const osg::Vec3& widthVec,
		const osg::Vec3& heightVec, float l = 0, float b = 0, float r = 1, float t = 1);

	static osg::Geometry* createTriangles(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
		const std::vector<int>& ids, osg::Camera* camera);

	static osg::Geometry* createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
		const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode = osg::PrimitiveSet::LINE_LOOP);

	//虚线 区分内外
	static osg::Node* create_lines0(osgViewer::Viewer& view);

	//打断  连接关系  不相交不打断
	static osg::Node* create_lines1(osgViewer::Viewer& view);

	//大场景
	static osg::Node* create_lines2(osgViewer::Viewer& view);

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

			//osg::ref_ptr<osg::RefMatrix> proMat = new osg::RefMatrix(rttCamera->getProjectionMatrix());
			//renderInfo.getState()->applyProjectionMatrix(proMat);
			//renderInfo.getState()->applyModelViewMatrix(rttCamera->getViewMatrix());
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
		float sz = 12;
		osg::Vec3d p1(-sz, 0, 1);
		osg::Vec3d p2(sz, 0, 1);

		float range = (p1 * mvp - p2 * mvp).length();
		//std::cout << range << "\n";
		if (range > 50) range = 50;
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
		if (range > 50) range = 50;
		uniform->set(range);
	}

private:
	osg::Camera* mCamera;
};