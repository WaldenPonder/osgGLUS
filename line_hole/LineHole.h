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
#include <queue>
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
extern bool g_always_dont_connected;
extern bool g_always_intersection;
extern bool g_is_daoxian_file;
extern std::queue<int> g_rang_i;
extern int g_current_range;
extern bool g_is_need_recalculate_range;

extern osg::ref_ptr<osg::Camera> g_hudCamera;

struct TextureFrameByFrame
{
	osg::ref_ptr<osg::Texture2D> texture;
	osg::ref_ptr<osg::Camera> camera;
	osg::ref_ptr<osg::Geode> geode;
};

struct RenderPass
{
	enum {
		LINE_PASS,
		BACKGROUND_PASS,
		CABLE_PASS
	} type;
	osg::Texture2D* baseColorTexture = nullptr;
	osg::Texture2D* depthTexture = nullptr;
	osg::Texture2D* idTexture = nullptr;
	osg::Texture2D* linePtTexture = nullptr;
	osg::Camera* rttCamera = nullptr;

	TextureFrameByFrame frameTexture1;
	TextureFrameByFrame frameTexture2;
};

extern RenderPass g_linePass;
extern RenderPass g_backgroundPass;
extern RenderPass g_cablePass;

#define NM_HUD (1 << 2)
#define NM_HIDDEN_LINE (1 << 3)
#define NM_FACE (1 << 4)
#define NM_OUT_LINE (1 << 5)
#define NM_LINE (1 << 6)
#define NM_HIDE_OBJECT (1 << 7)
#define NM_ALL (~0)
#define NM_LINE_PASS_QUAD (1 << 8)
#define NM_BACKGROUND_PASS_QUAD (1 << 9)
#define NM_CABLE_PASS_QUAD (1 << 10)

//导线 桥接  机电设备
#define NM_CABLE (1 << 11)
#define NM_QIAOJIA_JIDIANSHEBEI (1 << 12)

#define NM_NLL 0
#define NM_ALL (~0)

//越小越先画，默认0, 面要最先画,  实体线第二   虚线最后
namespace RenderPriority
{
	static const int FACE = 1; //面
	static const int DOT_LINE = 2; //虚线
	static const int LINE = 3; //实线
	static const int OUT_LINE = 4; //轮廓线

	//默认相机，视角沿z轴往下看，多张图，要先渲染远处的，才能得到正确结果
	static const int backgroundQuad = 3;
	static const int lineQuad = 5;
	static const int cableQuad = 6;
};

namespace QUAD_Z
{
	//面在下，先画
	static const float faceZ = -0.5;
	static const float lineZ = -0.3;
	static const float cableZ = -0.2;
}

union ColorID
{
	explicit ColorID(unsigned int id_) : id(id_) {}
	ColorID(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_)
		: r(r_), g(g_), b(b_), a(a_)
	{
	}

	struct {
		unsigned char r, g, b, a;
	};
	unsigned int id;
};

//------------------------------------------------------------------------------------------LineHole
class LineHole
{
public:
	static void createRttCamera(osgViewer::Viewer* viewer, RenderPass& pass);

	static osg::ref_ptr<osg::TextureBuffer> create_tbo(const vector<int>& data);

	//hud camera, 最终显示的贴图都在它下
	static void createHudCamera(osgViewer::Viewer* viewer);

	struct TextureQuadParam
	{
		RenderPass pass;
		osg::Camera* camera;
		osg::Geometry* geom;
		osg::Texture2D* inputTexture;
		osg::ref_ptr<osg::Program> program;
		int priority;
		int nodeMask;
	};
	static osg::Geode* createTextureQuad(const TextureQuadParam& param);

	static void processTexture(const RenderPass& pass, TextureFrameByFrame& frameTexture, int nodeMask, int priority, osg::Texture2D* inputTexture);

	static osg::Geode* displayTextureInHudCamera(const osg::ref_ptr<osg::Texture2D>& inputTexture, const osg::Vec3& pos, int priority, int mask = NM_ALL);

	//虚线
	static void setUpHiddenLineStateset(osg::StateSet* ss, osg::Camera* camera);

	static void setUpStateset(osg::StateSet* ss, osg::Camera* camera, bool isLine = true);

	static osg::Geometry* createHudTextureQuad(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner, const osg::Vec3& widthVec,
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

		osg::Matrixd mvp = mat * mCamera->getProjectionMatrix() * mCamera->getViewport()->computeWindowMatrix();
		float sz = 10;
		osg::Vec3d p1(-sz, 0, 1);
		osg::Vec3d p2(sz, 0, 1);

		float range = (p1 * mvp - p2 * mvp).length();
		//if (g_is_need_recalculate_range)
		//	std::cout << range << "\n";
		if (range > 300) range = 300;
		g_current_range = range;

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
		if (range > 300) range = 300;
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

//------------------------------------------------------------------------------------------RangeICallback
class RangeICallback : public osg::Uniform::Callback
{
public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		if (g_rang_i.size())
		{
			int r = g_rang_i.front();

			uniform->set(r);
			g_rang_i.pop();

			cout << "range i : " << r << "  g_current_range:  " << g_current_range << "\n";
		}
		else
		{
			uniform->set(1);
			cout << "range i : " << 1 << "  g_current_range:  " << g_current_range << "\n";
		}
	}
};

//------------------------------------------------------------------------------------------AlwaysDontConnectedCallback
class AlwaysDontConnectedCallback : public osg::Uniform::Callback
{
public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(g_always_dont_connected);
	}
};

//------------------------------------------------------------------------------------------AlwaysDontConnectedCallback
class AlwaysIntersectionCallback : public osg::Uniform::Callback
{
public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(g_always_intersection);
	}
};