// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "LineHole.h"

osg::Texture2D* g_texture;
osg::Texture2D* g_depthTexture;
osg::Texture2D* g_idTexture;
osg::Texture2D* g_linePtTexture;

osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

#define TEXTURE_SIZE1 1024
#define TEXTURE_SIZE2 1024

osg::Group* g_root;
osg::BoundingBox g_line_bbox;
osg::Vec4 CLEAR_COLOR(0, 0, 0, 1);// (204 / 255, 213 / 255, 240 / 255, 1);

osg::Geode* g_hidden_line_geode;

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

int main()
{
	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
	g_hidden_line_geode = new osg::Geode;
	g_root = root;
	view.setSceneData(root);

	setUp(view);

	std::vector<osg::Texture2D*> textures = LineHole::createRttCamera(&view);
	osg::Camera* hud_camera = LineHole::createHudCamera(&view, textures);
	root->addChild(hud_camera);

	auto* ss = g_hidden_line_geode->getOrCreateStateSet();
	LineHole::setUpHiddenLineStateset(ss, view.getCamera());

	//没什么意义，不会显示，只是为了鼠标操作方便
	{
		float s = g_line_bbox.radius();
		osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
		pat->setPosition(g_line_bbox.center());
		pat->setScale(osg::Vec3(s, s, s));
		pat->addChild(osgDB::readNodeFile(shader_dir() + "/model/cube.obj"));
		pat->setNodeMask(1);
		root->addChild(pat);
	}

	add_event_handler(view);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);

	return view.run();
}