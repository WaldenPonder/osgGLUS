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

osg::Camera*   hudCamera  = new osg::Camera;
osg::Group*	root		  = new osg::Group();
osg::Camera*   fbo_camera = new osg::Camera;
osg::Geometry* screenQuat = nullptr;

void set_up(float w, float h, osgViewer::Viewer* view);

struct MyCameraPostDrawCallback : public osg::Camera::DrawCallback
{
	MyCameraPostDrawCallback(osg::Image* image) : _image(image) {}

	virtual void operator()(const osg::Camera& /*camera*/) const
	{
		if (_image && _image->getPixelFormat() == GL_RGBA && _image->getDataType() == GL_UNSIGNED_BYTE)
		{
			// we'll pick out the center 1/2 of the whole image,
			int column_start = _image->s() / 4;
			int column_end   = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end   = 3 * row_start;

			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				unsigned char* data = _image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					(*data) = 255 - (*data);
					++data;
					(*data) = 255 - (*data);
					++data;
					(*data) = 255 - (*data);
					++data;
					(*data) = 255;
					++data;
				}
			}

			// dirty the image (increments the modified count) so that any textures
			// using the image can be informed that they need to update.
			_image->dirty();
		}
		else if (_image && _image->getPixelFormat() == GL_RGBA && _image->getDataType() == GL_FLOAT)
		{
			// we'll pick out the center 1/2 of the whole image,
			int column_start = _image->s() / 4;
			int column_end   = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end   = 3 * row_start;

			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				float* data = ( float* )_image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					(*data) = 1.0f - (*data);
					++data;
					(*data) = 1.0f - (*data);
					++data;
					(*data) = 1.0f - (*data);
					++data;
					(*data) = 1.0f;
					++data;
				}
			}

			// dirty the image (increments the modified count) so that any textures
			// using the image can be informed that they need to update.
			_image->dirty();
		}
	}

	osg::Image* _image;
};

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

void set_up(float w, float h, osgViewer::Viewer* view)
{
	std::vector<osg::Vec3d> allPTs;
	allPTs.push_back(osg::Vec3(0, 0, 0));
	allPTs.push_back(osg::Vec3(100, 0, 0));
	allPTs.push_back(osg::Vec3(100, 100, 0));

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	//osg::Node* node =
	//	createLine(allPTs, osg::Vec4(0, 1, 1, 1), osg::PrimitiveSet::LINE_STRIP);
	//osg::Node* node = osgDB::readNodeFile("D:\\Shader\\cube.obj"); //osgDB::readNodeFile("cow.osg");
	osg::Node* node = osgDB::readNodeFile("E:\\FileRecv\\abc.osg");

	//root->addChild(node);
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	//pat->setPosition(osg::Vec3(0, 15, 0));
	pat->addChild(node);
	root->addChild(pat);

	//viewer.setUpViewAcrossAllScreens();

	osg::Texture2D* texture2D = new osg::Texture2D;
	screenQuat				  = osg::createTexturedQuadGeometry(
		   osg::Vec3(), osg::Vec3(w, 0, 0), osg::Vec3(0, h, 0));
	{
		hudCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hudCamera->setProjectionMatrixAsOrtho2D(0, w, 0, h);
		hudCamera->setViewMatrix(osg::Matrix::identity());
		hudCamera->setRenderOrder(osg::Camera::POST_RENDER);
		hudCamera->setClearMask(GL_DEPTH_BUFFER_BIT);
		hudCamera->getOrCreateStateSet()->setMode(GL_LIGHTING,
												  osg::StateAttribute::OFF);
		root->addChild(hudCamera);

		osg::Geode* geode = new osg::Geode;
		hudCamera->addChild(geode);
		geode->addChild(screenQuat);
		geode->getOrCreateStateSet()->setRenderingHint(
			osg::StateSet::TRANSPARENT_BIN);
		geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		screenQuat->setDataVariance(osg::Object::DYNAMIC);
		screenQuat->setSupportsDisplayList(false);

		texture2D->setTextureSize(1024, 1024);
		texture2D->setInternalFormat(GL_RGBA);
		texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		osg::StateSet* ss = geode->getOrCreateStateSet();

		ss->setTextureAttributeAndModes(0, texture2D,
										osg::StateAttribute::ON);

		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert	= osgDB::readShaderFile(osg::Shader::VERTEX, "outline.vert");
		osg::Shader*			   frag	= osgDB::readShaderFile(osg::Shader::FRAGMENT, "outline.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("u_screen_width", w));
		ss->addUniform(new osg::Uniform("u_screen_height", h));
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	}
	root->addChild(fbo_camera);

	// then create the camera node to do the render to texture

	// add subgraph to render
	fbo_camera->addChild(node);

	//view->addSlave(fbo_camera, false);
	fbo_camera->setName("FBO_CAMERA");
	//fbo_camera->addEventCallback(new camere_callback(viewer.getCamera()));
	// set up the background color and clear mask.
	fbo_camera->setClearColor(osg::Vec4());
	fbo_camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set view
	fbo_camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	fbo_camera->setViewport(0, 0, 1024, 1024);

	// set the camera to render before the main camera.
	fbo_camera->setRenderOrder(osg::Camera::PRE_RENDER);

	// tell the camera to use OpenGL frame buffer object where supported.
	fbo_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	bool useImage = false;
	if (useImage)
	{
		osg::Image* image = new osg::Image;
		image->allocateImage(1024, 1024, 1, GL_RGBA, GL_FLOAT);

		// attach the image so its copied on each frame.
		fbo_camera->attach(osg::Camera::COLOR_BUFFER, image, 4, 4);
		// camera->setPostDrawCallback(new MyCameraPostDrawCallback(image));

		// Rather than attach the texture directly to illustrate the texture's
		// ability to detect an image update and to subload the image onto the
		// texture.  You needn't do this when using an Image for copying to, as a
		// separate camera->attach(..) would suffice as well, but we'll do it the
		// long way round here just for demonstration purposes (long way round
		// meaning we'll need to copy image to main memory, then copy it back to
		// the graphics card to the texture in one frame). The long way round
		// allows us to manually modify the copied image via the callback and then
		// let this modified image by reloaded back.
		texture2D->setImage(0, image);
	}
	else
	{
		// attach the texture and use it as the color buffer.
		fbo_camera->attach(osg::Camera::COLOR_BUFFER, texture2D,
						   0, 0, false,
						   4, 4);
	}

	fbo_camera->setProjectionMatrix(view->getCamera()->getProjectionMatrix());
	fbo_camera->setViewMatrix(view->getCamera()->getViewMatrix());
}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	osg::setNotifyLevel(osg::NotifySeverity::INFO);

	osgViewer::Viewer viewer(arguments);
	viewer.setSceneData(root);
	viewer.addEventHandler(new PickHandler(fbo_camera));

	add_event_handler(viewer);

	return viewer.run();
}
