#include "stdafx.h"
#include "../common/common.h"

#include "osg/AutoTransform"
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <sstream>

#define PING_PONG_NUM 8

osg::Camera*	g_hudCamera  = new osg::Camera;
osg::Group*		g_root		 = new osg::Group;
osg::Camera*	g_first_fbo  = new osg::Camera;
osg::Group*		g_modelGroup = new osg::Group;
osg::Camera*	g_pingpong_fbo[PING_PONG_NUM] = {nullptr};
osg::Texture2D* g_pingpong_texture[PING_PONG_NUM];
osg::Texture2D* g_first_texture = new osg::Texture2D;
osg::Geode* g_geode_quat = nullptr;

osg::Geometry* g_screenQuat = nullptr;

void start(float w, float h, osgViewer::Viewer* view);

class camere_callback : public osg::Callback
{
 public:
	osg::Camera* c_;
	camere_callback(osg::Camera* c) : c_(c) {}
	virtual bool run(osg::Object* object, osg::Object* data)
	{
		osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
		string		 name   = camera->getName();

		camera->setProjectionMatrix(c_->getProjectionMatrix());
		camera->setViewMatrix(c_->getViewMatrix());
		// hudCamera->setViewport(c_->getViewport());

		return traverse(object, data);
	}
};

class PickHandler : public osgGA::GUIEventHandler
{

 public:
	osg::Camera* fbo_camera;
	PickHandler(osg::Camera* fbo) : fbo_camera(fbo) {}

	~PickHandler() {}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* view = dynamic_cast<osgViewer::Viewer*>(&aa);
		fbo_camera->setProjectionMatrix(view->getCamera()->getProjectionMatrix());
		fbo_camera->setViewMatrix(view->getCamera()->getViewMatrix());

			//for (int i = 0; i < PING_PONG_NUM; i++)
			//{
			//	if(!g_pingpong_fbo[i]) continue;
			//	g_pingpong_fbo[i]->setProjectionMatrix(view->getCamera()->getProjectionMatrix());
			//	g_pingpong_fbo[i]->setViewMatrix(view->getCamera()->getViewMatrix());
			//}
		
		switch (ea.getEventType())
		{
		case (osgGA::GUIEventAdapter::FRAME):
		{
		}
		case (osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_A)
			{
				osg::Viewport* vp = view->getCamera()->getViewport();
				start(vp->width(), vp->height(), view);
			}
			break;
		}
		}
		return false;
	}

	virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea) {}
};

void set_pingpong_texture()
{
	for (int i = 0; i < PING_PONG_NUM; i++)
	{
		osg::Texture2D* texture2D = new osg::Texture2D;
		g_pingpong_texture[i]	 = texture2D;
		texture2D->setTextureSize(1024, 1024);
		texture2D->setInternalFormat(GL_RGBA);
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	}
		
	g_first_texture->setTextureSize(1024, 1024);
	g_first_texture->setInternalFormat(GL_RGBA);
	g_first_texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	g_first_texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
}

void create_blur_fbo(float w, float h, osgViewer::Viewer* view)
{
	for (int i = 0; i < PING_PONG_NUM; i++)
	{
		g_pingpong_fbo[i] = new osg::Camera;
		osg::Camera* fbo  = g_pingpong_fbo[i];

		osg::Geometry* screenQuat = osg::createTexturedQuadGeometry(
			osg::Vec3(), osg::Vec3(w, 0, 0), osg::Vec3(0, h, 0));
		{
			fbo->setName("FBO_CAMERA");
			// set up the background color and clear mask.
			fbo->setClearColor(osg::Vec4());
			fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// set view
			fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			// set viewport
			fbo->setViewport(0, 0, 1024, 1024);

			// set the camera to render before the main camera.
			fbo->setRenderOrder(osg::Camera::PRE_RENDER, 10);

			// tell the camera to use OpenGL frame buffer object where supported.
			fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

			// attach the texture and use it as the color buffer.
			fbo->attach(osg::Camera::COLOR_BUFFER, g_pingpong_texture[i],
				0, 0, false,
				4, 4);

			fbo->setProjectionMatrixAsOrtho2D(0, w, 0, h);
			fbo->setViewMatrix(osg::Matrix::identity());

			
			osg::Geode* geode = new osg::Geode;
			fbo->addChild(geode);
			geode->addChild(screenQuat);
			geode->getOrCreateStateSet()->setRenderingHint(
				osg::StateSet::TRANSPARENT_BIN);
			geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

			screenQuat->setDataVariance(osg::Object::DYNAMIC);
			screenQuat->setSupportsDisplayList(false);

			osg::StateSet* ss = geode->getOrCreateStateSet();

			ss->setTextureAttributeAndModes(0, i == 0 ? g_first_texture : g_pingpong_texture[i-1], osg::StateAttribute::ON);

			osg::ref_ptr<osg::Program> program = new osg::Program;
			osg::Shader*			   vert	= osgDB::readShaderFile(osg::Shader::VERTEX, "blur.vert");
			osg::Shader*			   frag	= osgDB::readShaderFile(osg::Shader::FRAGMENT, "blur.frag");
			program->addShader(vert);
			program->addShader(frag);
			ss->addUniform(new osg::Uniform("baseTexture", 0));
			ss->addUniform(new osg::Uniform("u_screen_width", w));
			ss->addUniform(new osg::Uniform("u_screen_height", h));
			ss->addUniform(new osg::Uniform("u_is_horizontal", i % 2 == 0));
			ss->setAttributeAndModes(program, osg::StateAttribute::ON);
		}
	}
}

void create_hud_fbo(float w, float h, osg::Texture2D* texture)
{
	g_screenQuat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(w, 0, 0), osg::Vec3(0, h, 0));
	{
		g_hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		g_hudCamera->setProjectionMatrixAsOrtho2D(0, w, 0, h);
		g_hudCamera->setViewMatrix(osg::Matrix::identity());
		g_hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
		g_hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
		g_hudCamera->getOrCreateStateSet()->setMode(GL_LIGHTING,
													osg::StateAttribute::OFF);

		g_geode_quat = new osg::Geode;
		g_hudCamera->addChild(g_geode_quat);
		g_geode_quat->addChild(g_screenQuat);
		g_geode_quat->getOrCreateStateSet()->setRenderingHint(
			osg::StateSet::TRANSPARENT_BIN);
		g_geode_quat->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		g_screenQuat->setDataVariance(osg::Object::DYNAMIC);
		g_screenQuat->setSupportsDisplayList(false);

		osg::StateSet* ss = g_geode_quat->getOrCreateStateSet();
		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert	= osgDB::readShaderFile(osg::Shader::VERTEX, "outline.vert");
		osg::Shader*			   frag	= osgDB::readShaderFile(osg::Shader::FRAGMENT, "outline.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("u_screen_width", w));
		ss->addUniform(new osg::Uniform("u_screen_height", h));
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	}
}

void create_first_fbo(osg::Group* modelGroup, osgViewer::Viewer* view)
{
	// add subgraph to render
	g_first_fbo->addChild(modelGroup);

	//view->addSlave(fbo_camera, false);
	g_first_fbo->setName("FBO_CAMERA");
	//fbo_camera->addEventCallback(new camere_callback(viewer.getCamera()));
	// set up the background color and clear mask.
	g_first_fbo->setClearColor(osg::Vec4());
	g_first_fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set view
	g_first_fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	g_first_fbo->setViewport(0, 0, 1024, 1024);

	// set the camera to render before the main camera.
	g_first_fbo->setRenderOrder(osg::Camera::PRE_RENDER);

	// tell the camera to use OpenGL frame buffer object where supported.
	g_first_fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	// attach the texture and use it as the color buffer.
	g_first_fbo->attach(osg::Camera::COLOR_BUFFER, g_first_texture,
						0, 0, false,
						4, 4);

	g_first_fbo->setProjectionMatrix(view->getCamera()->getProjectionMatrix());
	g_first_fbo->setViewMatrix(view->getCamera()->getViewMatrix());
}

void read_model()
{
	std::vector<osg::Vec3d> allPTs;
	allPTs.push_back(osg::Vec3(0, 0, 0));
	allPTs.push_back(osg::Vec3(100, 0, 0));
	allPTs.push_back(osg::Vec3(100, 100, 0));

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	g_root->addChild(g_modelGroup);
	{
		//modelGroup->addChild(createLine(allPTs, osg::Vec4(0, 0, 1, 1), osg::PrimitiveSet::LINE_LOOP));

		//osg::PositionAttitudeTransform* pat1 = new osg::PositionAttitudeTransform;
		//{
		//	osg::Node* node2 = osgDB::readNodeFile("D:\\Shader\\res\\teapot.obj");
		//	pat1->setPosition(osg::Vec3(0, 0, 7));
		//	pat1->addChild(node2);
		//	g_modelGroup->addChild(pat1);
		//}

		osg::PositionAttitudeTransform* pat2 = new osg::PositionAttitudeTransform;
		{
			osg::Node* node2 = osgDB::readNodeFile("E:\\FileRecv\\abc.osg");
			pat2->setPosition(osg::Vec3(0, 15, 0));
			pat2->addChild(node2);
			g_modelGroup->addChild(pat2);
		}

		//osg::PositionAttitudeTransform* pat3 = new osg::PositionAttitudeTransform;
		//{
		//	osg::Node* node2 = osgDB::readNodeFile("D:\\Shader\\cube.obj");
		//	pat3->setPosition(osg::Vec3(0, 0, 5));
		//	pat3->addChild(node2);
		//	g_modelGroup->addChild(pat3);
		//}
	}
}

void start(float w, float h, osgViewer::Viewer* view)
{
	read_model();
	set_pingpong_texture();

	create_first_fbo(g_modelGroup, view);
	create_blur_fbo(w, h, view);
	create_hud_fbo(w, h, g_pingpong_texture[PING_PONG_NUM-1]);

	g_root->addChild(g_first_fbo);

	for (int i = 0; i < PING_PONG_NUM; i++)
		g_root->addChild(g_pingpong_fbo[i]);

	g_root->addChild(g_hudCamera);
}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);
	
	osgViewer::Viewer viewer(arguments);
	viewer.setSceneData(g_root);
	viewer.addEventHandler(new PickHandler(g_first_fbo));

	add_event_handler(viewer);
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

	return viewer.run();
}
