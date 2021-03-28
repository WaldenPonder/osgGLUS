﻿// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "LineHole.h"
#include "ReadJsonFile.h"
#include "TwoDimManipulator.h"
#include <unordered_map>
#include <assert.h>
#include "ConvexHullVisitor.h"

RenderPass g_linePass;
RenderPass g_facePass;

osgViewer::Viewer* g_viewer;
osg::ref_ptr <osg::MatrixTransform> g_sceneNode;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

osg::ref_ptr<osg::Camera> g_hudCamera;

bool g_is_orth_camera = false;
bool g_line_hole_enable = true;
bool g_always_dont_connected = false; //构件之间，不考虑连接关系
bool g_always_intersection = false; //不考虑直线相交的情况

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


//切换场景
class MyEventHandler : public osgGA::GUIEventHandler
{
public:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if (!viewer || !g_sceneNode) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == 'a')
			{
				static osg::Matrix s_projectionMatrix;
				auto* mani = g_viewer->getCameraManipulator();
				if (mani == g_trackballManipulator.get())
				{
					g_viewer->setCameraManipulator(g_twodimManipulator);
					s_projectionMatrix = g_viewer->getCamera()->getProjectionMatrix();

					g_is_orth_camera = true;
				}
				else
				{
					g_viewer->setCameraManipulator(g_trackballManipulator);
					g_viewer->getCamera()->setProjectionMatrix(s_projectionMatrix);
					g_is_orth_camera = false;
				}
			}
			else if (ea.getKey() == 'b')
			{
				g_is_top_view = !g_is_top_view;
				g_viewer->getCameraManipulator()->home(0);
			}
			else if (ea.getKey() == 'c')
			{
				g_always_dont_connected = !g_always_dont_connected;
			}
			else if (ea.getKey() == 'd')
			{
				osg::Camera* camera = g_linePass.rttCamera;
				if (camera->getCullMask() & NM_HIDDEN_LINE)
					camera->setCullMask(camera->getCullMask() & ~NM_HIDDEN_LINE);
				else
					camera->setCullMask(camera->getCullMask() | NM_HIDDEN_LINE);
			}
			else if (ea.getKey() == 'e')
			{
				g_line_hole_enable = !g_line_hole_enable;
			}
			else if(ea.getKey() == 'f')
			{
				unsigned mask = g_linePass.rttCamera->getCullMask();
				if(mask & NM_FACE)
					g_linePass.rttCamera->setCullMask(mask & ~NM_FACE);
				else 
					g_linePass.rttCamera->setCullMask(mask | NM_FACE);

				unsigned mask2 = g_facePass.rttCamera->getCullMask();
				if (mask2 & NM_FACE)
					g_facePass.rttCamera->setCullMask(mask2 & ~NM_FACE);
				else
					g_facePass.rttCamera->setCullMask(mask2 | NM_FACE);
			}
			else if (ea.getKey() == 'h')
			{
				if(g_convexRoot)
					g_convexRoot->removeChildren(0, g_convexRoot->getNumChildren());

				ConvexHullVisitor chv;
				chv.setTraversalMask(NM_FACE);
				g_sceneNode->accept(chv);

				g_sceneNode->addChild(g_convexRoot);
			}
			else if (ea.getKey() == 'i')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_FACE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_FACE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_FACE_PASS_QUAD);
			}
			else if (ea.getKey() == 'j')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_LINE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_LINE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_LINE_PASS_QUAD);
			}
			else if (ea.getKey() == 't')
			{
				g_always_intersection = !g_always_intersection;
			}
		}

		return __super::handle(ea, aa);
	}
};

int setUp(osgViewer::Viewer& view)
{
	osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
	traits->x = 420; traits->y = 10;
	traits->width = TEXTURE_SIZE1; traits->height = TEXTURE_SIZE2;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->readDISPLAY();
	traits->setUndefinedScreenDetailsToDefaultScreen();
	osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (!gc.valid())
	{
		osg::notify(osg::FATAL) << "Unable to create OpenGL v" << " context." << std::endl;
		return -1;
	}
	osg::Camera* cam = view.getCamera();
	cam->setGraphicsContext(gc.get());
	cam->setViewport(new osg::Viewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2));
	return 0;
}

void ReadFile()
{
	TEXTURE_SIZE1 = TEXTURE_SIZE2 = 1024;
	std::string file_name = "F:\\MEP_Json\\机电遮挡.json";

	bool bReadFileSuccess = true;
	do
	{
		ifstream IF(shader_dir() + "/line_hole/config.ini");

		if (!IF.is_open())
		{
			bReadFileSuccess = false;
			break;
		}

		string buf;
		getline(IF, buf);
		if (buf != "FILE_PATH:")
			bReadFileSuccess = false;
		getline(IF, buf);
		file_name = buf;
	} while (0);

	if (!bReadFileSuccess)
		cout << "读取配置文件失败" << endl;

	ReadJsonFile::read(file_name);
}

//对id的可视化， id转颜色
int main()
{
	ReadFile();

	osgViewer::Viewer viewer;
	osg::Group* root = new osg::Group;
	g_viewer = &viewer;
	g_root = root;

	viewer.setSceneData(root);

	setUp(viewer);

	g_linePass.rttCamera = LineHole::createLineRttCamera(&viewer);
	g_facePass.rttCamera = LineHole::createFaceRttCamera(&viewer);

	g_root->addChild(g_facePass.rttCamera);
	g_root->addChild(g_linePass.rttCamera);

	g_sceneNode = ReadJsonFile::createScene(g_elementRoot);

	g_hudCamera = LineHole::createHudCamera(&viewer);
	root->addChild(g_hudCamera);

	g_linePass.rttCamera->addChild(g_sceneNode);
	g_facePass.rttCamera->addChild(g_sceneNode);

	//没什么意义，不会显示，只是为了鼠标操作方便
	{
		float s = g_line_bbox.radius();
		g_mouseBoxPat = new osg::PositionAttitudeTransform;
		g_mouseBoxPat->setPosition(g_line_bbox.center());
		g_mouseBoxPat->setScale(osg::Vec3(s, s, s));
		g_mouseBoxPat->addChild(osgDB::readNodeFile(shader_dir() + "/model/cube.obj"));
		//g_mouseBoxPat->setNodeMask(NM_HIDE_OBJECT);
		//g_mouseBoxPat->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		g_mouseBoxPat->getOrCreateStateSet()->setAttributeAndModes(new osg::ColorMask(false, false, false, false));
		root->addChild(g_mouseBoxPat);
	}

	//view.getCamera()->setCullMask(~NM_HIDE_OBJECT);
	//view.getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES);
	//g_rttCamera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_USING_PRIMITIVES);

	viewer.addEventHandler(new osgViewer::StatsHandler);
	viewer.addEventHandler(new MyEventHandler);
	//osg::setNotifyLevel(osg::NotifySeverity::NOTICE);

	g_trackballManipulator = new osgGA::TrackballManipulator();
	g_twodimManipulator = new TwoDimManipulator(&viewer);
	viewer.setCameraManipulator(g_trackballManipulator);
	g_is_orth_camera = false;

	//// add the state manipulator
	//if (g_facePass.rttCamera)
	//	viewer.addEventHandler(new osgGA::StateSetManipulator(g_facePass.rttCamera->getOrCreateStateSet()));

	if (g_linePass.rttCamera)
		viewer.addEventHandler(new osgGA::StateSetManipulator(g_linePass.rttCamera->getOrCreateStateSet()));

	return viewer.run();
}
