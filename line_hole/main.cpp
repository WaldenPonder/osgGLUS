#include "pch.h"
#include "LineHole.h"
#include "ReadJsonFile.h"
#include "TwoDimManipulator.h"
#include <unordered_map>
#include <assert.h>
#include <map>
#include "ConvexHullVisitor.h"

RenderPass g_linePass;
RenderPass g_backgroundPass;
RenderPass g_cablePass;

osgViewer::Viewer* g_viewer;
osg::ref_ptr <osg::MatrixTransform> g_sceneNode;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

osg::ref_ptr<osg::Camera> g_hudCamera;

bool g_is_need_recalculate_range = true;
bool g_is_orth_camera = false;
bool g_line_hole_enable = true;
std::queue<int> g_rang_i;
int g_current_range = 0;
bool g_always_dont_connected = false; //构件之间，不考虑连接关系
bool g_always_intersection = false; //不考虑直线相交的情况
bool g_is_daoxian_file = false;

int TEXTURE_SIZE1;
int TEXTURE_SIZE2;

osg::Group* g_root;
osg::BoundingBox g_line_bbox;
osg::Vec4 CLEAR_COLOR(0, 0, 0, 0);

osg::Geode* g_hidden_line_geode;
osg::PositionAttitudeTransform* g_mouseBoxPat = nullptr;

bool g_is_top_view = true;
osg::ref_ptr<osgGA::TrackballManipulator> g_trackballManipulator;
osg::ref_ptr<TwoDimManipulator> g_twodimManipulator;

bool flag_start = false;

#include "MyEventHandler.h"

/*
//三角面分摊查找的逻辑，移植到line hole shader内
//line hole shader能正常工作

现在在一个圆形区域内


超出边界的不处理


直线被屏幕截取的情况

机电设备和其它构建的遮挡关系正常
*/

int main()
{
	ReadFile();

	osgViewer::Viewer viewer;
	osg::Group* root = new osg::Group;
	g_viewer = &viewer;
	g_root = root;

	viewer.setSceneData(root);

	init(viewer);

	LineHole::processTexture(g_linePass, g_linePass.frameTexture1, NM_LINE_PASS_QUAD, 1, g_linePass.baseColorTexture);
	LineHole::processTexture(g_linePass, g_linePass.frameTexture2, NM_LINE_PASS_QUAD, 1, g_linePass.frameTexture1.texture);

	LineHole::processTexture(g_cablePass, g_cablePass.frameTexture1, NM_CABLE_PASS_QUAD, 1, g_cablePass.baseColorTexture);
	LineHole::processTexture(g_cablePass, g_cablePass.frameTexture2, NM_CABLE_PASS_QUAD, 1, g_cablePass.frameTexture1.texture);

	LineHole::displayTextureInHudCamera(g_linePass.frameTexture2.texture, osg::Vec3(0, 0, QUAD_Z::lineZ), RenderPriority::lineQuad, NM_LINE_PASS_QUAD);
	LineHole::displayTextureInHudCamera(g_cablePass.frameTexture2.texture, osg::Vec3(0, 0, QUAD_Z::cableZ), RenderPriority::cableQuad, NM_CABLE_PASS_QUAD);
	LineHole::displayTextureInHudCamera(g_backgroundPass.baseColorTexture, osg::Vec3(0, 0, QUAD_Z::faceZ), RenderPriority::backgroundQuad, NM_BACKGROUND_PASS_QUAD);

	//没什么意义，不会显示，只是为了鼠标操作方便
	{
		float s = g_line_bbox.radius();
		g_mouseBoxPat = new osg::PositionAttitudeTransform;
		g_mouseBoxPat->setPosition(g_line_bbox.center());
		g_mouseBoxPat->setScale(osg::Vec3(s, s, s));
		g_mouseBoxPat->addChild(osgDB::readNodeFile(shader_dir() + "/model/cube.obj"));
		g_mouseBoxPat->getOrCreateStateSet()->setAttributeAndModes(new osg::ColorMask(false, false, false, false));
		root->addChild(g_mouseBoxPat);
	}

	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new MyEventHandler);
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

	g_trackballManipulator = new osgGA::TrackballManipulator();
	g_twodimManipulator = new TwoDimManipulator(&viewer);
	viewer.setCameraManipulator(g_trackballManipulator);
	g_is_orth_camera = false;

	//add the state manipulator
	//if (g_cablePass.rttCamera)
	//	viewer.addEventHandler(new osgGA::StateSetManipulator(g_cablePass.rttCamera->getOrCreateStateSet()));

	if (g_linePass.rttCamera)
		viewer.addEventHandler(new osgGA::StateSetManipulator(g_linePass.rttCamera->getOrCreateStateSet()));

	return viewer.run();
}