#pragma once
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osgDB/ReadFile"
#include <osg/TextureBuffer>
#include "osg/Depth"
#include "osg/LineStipple"
using namespace std;

extern osg::Texture2D* g_texture;
extern osg::Texture2D* g_depthTexture;
extern osg::Texture2D* g_idTexture;
extern osg::Texture2D* g_linePtTexture;

extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
extern osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

#define TEXTURE_SIZE1 1024
#define TEXTURE_SIZE2 1024

extern osg::Group* g_root;
extern osg::BoundingBox g_line_bbox;
extern osg::Vec4 CLEAR_COLOR;// (204 / 255, 213 / 255, 240 / 255, 1);

extern osg::Geode* g_hidden_line_geode;

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

	static osg::Geometry* createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
		const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode = osg::PrimitiveSet::LINE_LOOP);

	static osg::Node* create_lines(osgViewer::Viewer& view);
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
