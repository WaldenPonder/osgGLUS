// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "LineHole.h"
#include "ReadJsonFile.h"
#include "TwoDimManipulator.h"
#include <unordered_map>

osg::Texture2D* g_texture;
osg::Texture2D* g_depthTexture;
osg::Texture2D* g_idTexture;
osg::Texture2D* g_linePtTexture;
osgViewer::Viewer* g_viewer;
osg::ref_ptr <osg::Node> g_sceneNode;
osg::Camera* g_rttCamera = nullptr;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;
bool g_is_orth_camera = false;

#define TEXTURE_SIZE1 1024
#define TEXTURE_SIZE2 1024

osg::Group* g_root;
osg::BoundingBox g_line_bbox;
osg::Vec4 CLEAR_COLOR(0, 0, 0, 1);

osg::Geode* g_hidden_line_geode;
osg::PositionAttitudeTransform* g_mouseBoxPat = nullptr;

//切换场景
class MyEventHandler : public osgGA::GUIEventHandler
{
public:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if (!viewer || !g_rttCamera) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == 'c')
			{
				g_rttCamera->removeChildren(0, g_rttCamera->getNumChildren());

				static int index = 1;
				
				if (index % 3 == 0)
					g_rttCamera->addChild(LineHole::create_lines0(*viewer));
				else if (index % 3 == 1)
					g_rttCamera->addChild(LineHole::create_lines1(*viewer));
				else if (index % 3 == 2)
					g_rttCamera->addChild(LineHole::create_lines2(*viewer));

				index++;

				if (g_mouseBoxPat)
				{
					g_mouseBoxPat->setPosition(g_line_bbox.center());
					osg::BoundingSphere bs(g_line_bbox);
					float r = bs.radius();
					g_mouseBoxPat->setScale(osg::Vec3(r, r, r));
				}
			}
			else if (ea.getKey() == 'a')
			{
				g_is_orth_camera = true;
				auto mani = new TwoDimManipulator(g_viewer);
				g_viewer->setCameraManipulator(mani);
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

/*
为什么会消失?
*/
int main()
{
	//ReadJsonFile::read("F:\\MEP_Json\\piping-model\\model_Piping_fine.json");
	//ReadJsonFile::read("F:\\MEP_Json\\electrical-model\\model_Electrical_Coarse.json");
	ReadJsonFile::read("F:\\MEP_Json\\electrical-model\\model_Electrical_fine.json");

	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
	g_viewer = &view;
	g_root = root;

	view.setSceneData(root);
	
	setUp(view);

	std::vector<osg::Texture2D*> textures = LineHole::createRttCamera(&view);

	osg::Camera* hud_camera = LineHole::createHudCamera(&view, textures);
	root->addChild(hud_camera);

	//g_rttCamera->addChild(LineHole::create_lines0(view));
	g_rttCamera->addChild(ReadJsonFile::createScene(g_elementRoot));

	//没什么意义，不会显示，只是为了鼠标操作方便
	{
		float s = g_line_bbox.radius();
		g_mouseBoxPat = new osg::PositionAttitudeTransform;
		g_mouseBoxPat->setPosition(g_line_bbox.center());
		g_mouseBoxPat->setScale(osg::Vec3(s, s, s));
		g_mouseBoxPat->addChild(osgDB::readNodeFile(shader_dir() + "/model/cube.obj"));
		g_mouseBoxPat->setNodeMask(1);
		//root->addChild(g_mouseBoxPat);
	}

	//view.getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
	//g_rttCamera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

	add_event_handler(view);
	view.addEventHandler(new MyEventHandler);
	//osg::setNotifyLevel(osg::NotifySeverity::NOTICE);

	return view.run();
}