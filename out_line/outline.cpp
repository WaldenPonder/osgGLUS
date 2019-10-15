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

osg::Camera*	g_hudCamera  = new osg::Camera;
osg::Group*		g_root		 = new osg::Group();
osg::Camera*	g_first_fbo = new osg::Camera;
osg::Camera*	g_pingpong_fbo[10];
osg::Texture2D* g_pingpong_texture[2];

osg::Geometry* g_screenQuat = nullptr;

void set_up(float w, float h, osgViewer::Viewer* view);

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

		switch (ea.getEventType())
		{
		case (osgGA::GUIEventAdapter::FRAME):
		{
			//if (screenQuat)
			//{
			//	screenQuat->dirtyGLObjects();
			//}

			//fbo_camera->setViewport(view->getCamera()->getViewport());
			break;
		}
		case (osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_A)
			{
				osg::Viewport* vp = view->getCamera()->getViewport();
				set_up(vp->width(), vp->height(), view);
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
	for (int i = 0; i < 2; i++)
	{
		osg::Texture2D* texture2D = new osg::Texture2D;
		g_pingpong_texture[i % 2]  = texture2D;
		texture2D->setTextureSize(1024, 1024);
		texture2D->setInternalFormat(GL_RGBA);
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	}
}

void create_blur_camera(float w, float h, osgViewer::Viewer* view)
{
	set_pingpong_texture();

	for (int i = 0; i < 10; i++)
	{
		g_pingpong_fbo[i] = new osg::Camera;
		osg::Camera* fbo	   = g_pingpong_fbo[i];

		osg::Texture2D* texture2D  = g_pingpong_texture[i % 2];
		osg::Geometry*  screenQuat = osg::createTexturedQuadGeometry(
			 osg::Vec3(), osg::Vec3(w, 0, 0), osg::Vec3(0, h, 0));
		{
			fbo->setClearColor(osg::Vec4());
			fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			fbo->setProjectionMatrixAsOrtho2D(0, w, 0, h);
			fbo->setViewMatrix(osg::Matrix::identity());
			fbo->setRenderOrder(osg::Camera::PRE_RENDER);
			fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			fbo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			fbo->setViewport(0, 0, 1024, 1024);
			fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			g_first_fbo->attach(osg::Camera::COLOR_BUFFER, g_pingpong_texture[(i + 1) % 2],
								 0, 0, false,
								 4, 4);

			osg::Geode* geode = new osg::Geode;
			fbo->addChild(geode);
			geode->addChild(screenQuat);
			geode->getOrCreateStateSet()->setRenderingHint(
				osg::StateSet::TRANSPARENT_BIN);
			geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

			screenQuat->setDataVariance(osg::Object::DYNAMIC);
			screenQuat->setSupportsDisplayList(false);

			osg::StateSet* ss = geode->getOrCreateStateSet();

			ss->setTextureAttributeAndModes(0, texture2D,
											osg::StateAttribute::ON);

			osg::ref_ptr<osg::Program> program = new osg::Program;
			osg::Shader*			   vert	= osgDB::readShaderFile(osg::Shader::VERTEX, "blur.vert");
			osg::Shader*			   frag	= osgDB::readShaderFile(osg::Shader::FRAGMENT, "blur.frag");
			program->addShader(vert);
			program->addShader(frag);
			ss->addUniform(new osg::Uniform("baseTexture", 0));
			ss->addUniform(new osg::Uniform("u_screen_width", w));
			ss->addUniform(new osg::Uniform("u_screen_height", h));
			ss->addUniform(new osg::Uniform("horizontal", i % 2 == 0));
			ss->setAttributeAndModes(program, osg::StateAttribute::ON);
		}
	}
}

void create_hud_camera(float w, float h, osg::Texture2D* texture2D)
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
		
		osg::Geode* geode = new osg::Geode;
		g_hudCamera->addChild(geode);
		geode->addChild(g_screenQuat);
		geode->getOrCreateStateSet()->setRenderingHint(
			osg::StateSet::TRANSPARENT_BIN);
		geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		g_screenQuat->setDataVariance(osg::Object::DYNAMIC);
		g_screenQuat->setSupportsDisplayList(false);

		texture2D->setTextureSize(1024, 1024);
		texture2D->setInternalFormat(GL_RGBA);
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		osg::StateSet* ss = geode->getOrCreateStateSet();

		ss->setTextureAttributeAndModes(0, texture2D,
			osg::StateAttribute::ON);

		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert = osgDB::readShaderFile(osg::Shader::VERTEX, "outline.vert");
		osg::Shader*			   frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, "outline.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("u_screen_width", w));
		ss->addUniform(new osg::Uniform("u_screen_height", h));
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
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
		g_first_fbo->attach(osg::Camera::COLOR_BUFFER, g_pingpong_texture[0],
			0, 0, false,
			4, 4);
	
	g_first_fbo->setProjectionMatrix(view->getCamera()->getProjectionMatrix());
	g_first_fbo->setViewMatrix(view->getCamera()->getViewMatrix());
}

void set_up(float w, float h, osgViewer::Viewer* view)
{
	std::vector<osg::Vec3d> allPTs;
	allPTs.push_back(osg::Vec3(0, 0, 0));
	allPTs.push_back(osg::Vec3(100, 0, 0));
	allPTs.push_back(osg::Vec3(100, 100, 0));

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	osg::Group* modelGroup = new osg::Group;
	g_root->addChild(modelGroup);
	{
		//modelGroup->addChild(createLine(allPTs, osg::Vec4(0, 0, 1, 1), osg::PrimitiveSet::LINE_LOOP));

		osg::PositionAttitudeTransform* pat1 = new osg::PositionAttitudeTransform;
		{
			osg::Node* node2 = osgDB::readNodeFile("D:\\Shader\\res\\teapot.obj");
			pat1->setPosition(osg::Vec3(0, 0, 7));
			pat1->addChild(node2);
			modelGroup->addChild(pat1);
		}

		osg::PositionAttitudeTransform* pat2 = new osg::PositionAttitudeTransform;
		{
			osg::Node* node2 = osgDB::readNodeFile("E:\\FileRecv\\abc.osg");
			pat2->setPosition(osg::Vec3(0, 15, 0));
			pat2->addChild(node2);
			modelGroup->addChild(pat2);
		}

		osg::PositionAttitudeTransform* pat3 = new osg::PositionAttitudeTransform;
		{
			osg::Node* node2 = osgDB::readNodeFile("D:\\Shader\\cube.obj");
			pat3->setPosition(osg::Vec3(0, 0, 5));
			pat3->addChild(node2);
			modelGroup->addChild(pat3);
		}
	}

	create_first_fbo(modelGroup, view);
	
	create_blur_camera(w, h, view);
		
	create_hud_camera(w, h, g_pingpong_texture[1]);

	g_root->addChild(g_first_fbo);

	for (int i = 0; i < 10; i++)
		g_root->addChild(g_pingpong_fbo[i]);

	g_root->addChild(g_hudCamera);

}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	osg::setNotifyLevel(osg::NotifySeverity::INFO);

	osgViewer::Viewer viewer(arguments);
	viewer.setSceneData(g_root);
	viewer.addEventHandler(new PickHandler(g_first_fbo));

	add_event_handler(viewer);

	return viewer.run();
}
