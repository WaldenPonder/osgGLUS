// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "LineHole.h"
#include "ReadJsonFile.h"
#include "TwoDimManipulator.h"
#include <unordered_map>
#include <assert.h>
#include "ConvexHullVisitor.h"

RenderPass g_linePass;
RenderPass g_backgroundPass;
RenderPass g_cablePass;

osgViewer::Viewer* g_viewer;
osg::ref_ptr <osg::MatrixTransform> g_sceneNode;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;
osg::ref_ptr<osg::Texture2D> g_idTexture1; //512*512, 用于加速范围查询

osg::ref_ptr<osg::Camera> g_hudCamera;

bool g_is_orth_camera = false;
bool g_line_hole_enable = true;
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
				//osg::Camera* camera = g_linePass.rttCamera;
				//if (camera->getCullMask() & NM_HIDDEN_LINE)
				//	camera->setCullMask(camera->getCullMask() & ~NM_HIDDEN_LINE);
				//else
				//	camera->setCullMask(camera->getCullMask() | NM_HIDDEN_LINE);

				osg::Camera* camera = g_viewer->getCamera();
				if (camera->getCullMask() & NM_ID_PASS_QUAD)
					camera->setCullMask(camera->getCullMask() & ~NM_ID_PASS_QUAD);
				else
					camera->setCullMask(camera->getCullMask() | NM_ID_PASS_QUAD);
			}
			else if (ea.getKey() == 'e')
			{
				g_line_hole_enable = !g_line_hole_enable;
			}
			else if (ea.getKey() == 'f')
			{
				unsigned mask = g_linePass.rttCamera->getCullMask();
				if (mask & NM_FACE)
					g_linePass.rttCamera->setCullMask(mask & ~NM_FACE);
				else
					g_linePass.rttCamera->setCullMask(mask | NM_FACE);

				unsigned mask2 = g_backgroundPass.rttCamera->getCullMask();
				if (mask2 & NM_FACE)
					g_backgroundPass.rttCamera->setCullMask(mask2 & ~NM_FACE);
				else
					g_backgroundPass.rttCamera->setCullMask(mask2 | NM_FACE);
			}
			else if (ea.getKey() == 'h')
			{
				if (g_convexRoot)
					g_convexRoot->removeChildren(0, g_convexRoot->getNumChildren());

				ConvexHullVisitor chv;
				chv.setTraversalMask(NM_FACE | NM_QIAOJIA_JIDIANSHEBEI);
				g_sceneNode->accept(chv);

				g_sceneNode->addChild(g_convexRoot);
			}
			else if (ea.getKey() == 'i')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_BACKGROUND_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_BACKGROUND_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_BACKGROUND_PASS_QUAD);
			}
			else if (ea.getKey() == 'j')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_LINE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_LINE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_LINE_PASS_QUAD);
			}
			else if (ea.getKey() == 'k')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_CABLE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_CABLE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_CABLE_PASS_QUAD);
			}
			else if (ea.getKey() == 't')
			{
				g_always_intersection = !g_always_intersection;
			}
			else if (ea.getKey() == 'y')
			{
				system("cls");
				osg::Matrix mvp = g_viewer->getCamera()->getViewMatrix() * g_viewer->getCamera()->getProjectionMatrix();

				for (int i = 0; i < g_elementRoot.MEPElements.size(); i++)
				{
					auto& element = g_elementRoot.MEPElements[i];

					for (const LINE& line : element.Geometry.lines)
					{
						osg::Vec4 pt = osg::Vec4(line.StartPoint, 1)* mvp;
						bool b1 = abs(pt.x()) < 1 && abs(pt.y()) < 1;

						osg::Vec4 pt2 = osg::Vec4(line.EndPoint, 1) * mvp;
						bool b2 = abs(pt2.x()) < 1 && abs(pt2.y()) < 1;

						if(b1 && b2)
							cout << "  ---   " << pt  << "\t" << pt2 << "\n";
					}
				}			
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
	std::string file_name;

	bool bReadFileSuccess = true;
	ifstream IF(shader_dir() + "/line_hole/config.ini");

	if (!IF.is_open())
	{
		bReadFileSuccess = false;
		return;
	}

	do
	{
		string buf;
		getline(IF, buf);
		if (buf == "FILE_PATH:")
		{
			getline(IF, buf);
			if (!buf.empty())
			{
				g_is_daoxian_file = false;
				file_name = buf;
			}				
		}
		else if (buf == "DAOXIAN_FILE:")
		{
			getline(IF, buf);
			if (!buf.empty())
			{
				file_name = buf;
				g_is_daoxian_file = true;
			}
		}
	} while (!IF.fail());

	ReadJsonFile::read(file_name);
}


/*
3> 机电设备和其它构建的遮挡关系正常
*/

int main()
{
	ReadFile();

	osgViewer::Viewer viewer;
	osg::Group* root = new osg::Group;
	g_viewer = &viewer;
	g_root = root;

	viewer.setSceneData(root);

	setUp(viewer);

	g_linePass.type = RenderPass::LINE_PASS;
	g_backgroundPass.type = RenderPass::BACKGROUND_PASS;
	g_cablePass.type = RenderPass::CABLE_PASS;

	LineHole::createRttCamera(&viewer, g_linePass);
	LineHole::createRttCamera(&viewer, g_backgroundPass);
	LineHole::createRttCamera(&viewer, g_cablePass);

	g_root->addChild(LineHole::createIDPass());
	g_root->addChild(g_backgroundPass.rttCamera);
	g_root->addChild(g_linePass.rttCamera);
	g_root->addChild(g_cablePass.rttCamera);
	
	g_sceneNode = ReadJsonFile::createScene(g_elementRoot);

	g_hudCamera = LineHole::createHudCamera(&viewer);
	root->addChild(g_hudCamera);

	g_linePass.rttCamera->addChild(g_sceneNode);
	g_backgroundPass.rttCamera->addChild(g_sceneNode);
	g_cablePass.rttCamera->addChild(g_sceneNode);

	g_linePass.rttCamera->setCullMask(NM_LINE | NM_OUT_LINE | NM_HIDDEN_LINE | NM_FACE);
	//底图
	g_backgroundPass.rttCamera->setCullMask(NM_FACE);// | NM_QIAOJIA_JIDIANSHEBEI);
	//导线pass, 不需要绘制隐藏线
	g_cablePass.rttCamera->setCullMask(NM_CABLE | NM_QIAOJIA_JIDIANSHEBEI | NM_OUT_LINE);

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
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

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