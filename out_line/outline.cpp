#include "pch.h"
#include "../common/common.h"

#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <sstream>
#include "osg/AutoTransform"

void test1(osg::Group * root, osg::Node * node);


struct MyCameraPostDrawCallback : public osg::Camera::DrawCallback
{
	MyCameraPostDrawCallback(osg::Image* image) :
		_image(image)
	{
	}

	virtual void operator () (const osg::Camera& /*camera*/) const
	{
		if (_image && _image->getPixelFormat() == GL_RGBA && _image->getDataType() == GL_UNSIGNED_BYTE)
		{
			// we'll pick out the center 1/2 of the whole image,
			int column_start = _image->s() / 4;
			int column_end = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end = 3 * row_start;


			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				unsigned char* data = _image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					(*data) = 255 - (*data); ++data;
					(*data) = 255 - (*data); ++data;
					(*data) = 255 - (*data); ++data;
					(*data) = 255; ++data;
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
			int column_end = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end = 3 * row_start;

			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				float* data = (float*)_image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					(*data) = 1.0f - (*data); ++data;
					(*data) = 1.0f - (*data); ++data;
					(*data) = 1.0f - (*data); ++data;
					(*data) = 1.0f; ++data;
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
	camere_callback(osg::Camera* c) :c_(c) {}
	virtual bool run(osg::Object* object, osg::Object* data)
	{
		osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
		string name = camera->getName();

		camera->setProjectionMatrix(c_->getProjectionMatrix());
		camera->setViewMatrix(c_->getViewMatrix());

		return traverse(object, data);
	}
};

int main(int argc, char **argv) {
  osg::ArgumentParser arguments(&argc, argv);

  osg::setNotifyLevel(osg::NotifySeverity::INFO);

  std::vector<osg::Vec3d> allPTs;
  allPTs.push_back(osg::Vec3(0, 0, 0));
  allPTs.push_back(osg::Vec3(100, 0, 0));
  allPTs.push_back(osg::Vec3(100, 100, 0));

  osg::DisplaySettings::instance()->setNumMultiSamples(4);

  //osg::Node *node = createLine(allPTs, osg::Vec4(0, 1, 1, 1), osg::PrimitiveSet::LINE_STRIP);
  osg::Node* node = osgDB::readNodeFile("cow.osg");

  osg::Group *root = new osg::Group();
  root->addChild(node);

  osgViewer::Viewer viewer(arguments);
  viewer.setSceneData(root);


  osg::Texture2D* texture2D = new osg::Texture2D;
  osg::Geometry* gem = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(100, 0, 0), osg::Vec3(0, 100, 0));
  {

	  osg::Geode* geode = new osg::Geode;
	  root->addChild(geode);
	  geode->addChild(gem);
	  	  
	  gem->setDataVariance(osg::Object::DYNAMIC);
	  gem->setSupportsDisplayList(false);

	  texture2D->setTextureSize(512, 512);
	  texture2D->setInternalFormat(GL_RGBA);
	  texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	  texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

	  {
		  texture2D->setInternalFormat(GL_RGBA16F_ARB);
		  texture2D->setSourceFormat(GL_RGBA);
		  texture2D->setSourceType(GL_FLOAT);
	  }

	  osg::StateSet* stateset = geode->getOrCreateStateSet();

	  stateset->setTextureAttributeAndModes(0, texture2D, osg::StateAttribute::ON);

	 // osg::Image* img = osgDB::readImageFile("F:\\aaa.tga");
	  //texture2D->setImage(img);
  }



  // then create the camera node to do the render to texture
  if(1)
  {
	  osg::Camera* camera = new osg::Camera;
	  camera->setName("FBO_CAMERA");
	  camera->addUpdateCallback(new camere_callback(viewer.getCamera()));
	  // set up the background color and clear mask.
	  //camera->setClearColor(osg::Vec4(0.1f, 0.1f, 0.3f, 1.0f));
	  camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  //const osg::BoundingSphere& bs = node->getBound();

	  //float znear = 1.0f*bs.radius();
	  //float zfar = 3.0f*bs.radius();

	  //// 2:1 aspect ratio as per flag geometry below.
	  //float proj_top = 0.25f*znear;
	  //float proj_right = 0.5f*znear;

	  //znear *= 0.9f;
	  //zfar *= 1.1f;

	  // set up projection.
	//  camera->setProjectionMatrixAsFrustum(-proj_right, proj_right, -proj_top, proj_top, .1, 10000);

	  // set view
	  camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	//  camera->setViewMatrixAsLookAt(bs.center() - osg::Vec3(0.0f, 2.0f, 0.0f)*bs.radius(), bs.center(), osg::Vec3(0.0f, 0.0f, 1.0f));

	  // set viewport
	  camera->setViewport(0, 0, 512, 512);

	  // set the camera to render before the main camera.
	  camera->setRenderOrder(osg::Camera::PRE_RENDER);

	  // tell the camera to use OpenGL frame buffer object where supported.
	  camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	
	  if (1)
	  {
		  osg::Image* image = new osg::Image;
		  image->allocateImage(512, 512, 1, GL_RGBA, GL_FLOAT);

		  // attach the image so its copied on each frame.
		  camera->attach(osg::Camera::COLOR_BUFFER, image);
		// camera->setPostDrawCallback(new MyCameraPostDrawCallback(image));

		  // Rather than attach the texture directly to illustrate the texture's ability to
		  // detect an image update and to subload the image onto the texture.  You needn't
		  // do this when using an Image for copying to, as a separate camera->attach(..)
		  // would suffice as well, but we'll do it the long way round here just for demonstration
		  // purposes (long way round meaning we'll need to copy image to main memory, then
		  // copy it back to the graphics card to the texture in one frame).
		  // The long way round allows us to manually modify the copied image via the callback
		  // and then let this modified image by reloaded back.
		  texture2D->setImage(0, image);
	  }
	  else
	  {
		  // attach the texture and use it as the color buffer.
		  camera->attach(osg::Camera::COLOR_BUFFER, texture2D,
			  0, 0, false);
			  //samples, colorSamples);
	  }


	  // add subgraph to render
	  camera->addChild(node);
	  root->addChild(camera);
	  //viewer.addSlave(camera);
  }
   
  add_event_handler(viewer);
  return viewer.run();
}


void test1(osg::Group * root, osg::Node * node)
{
	int N = 1;
	for (int i = 0; i < N; i++) {
		float offset = 1e-3;

		const char *VS =
			"#version 330 compatibility \n"
			"uniform float outlineWidth; \n"
			"void main()"
			"{ \n"
			"    vec4 clip = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
			"    vec3 normal = normalize(gl_NormalMatrix * gl_Normal); \n"
			"    normal.x *= gl_ProjectionMatrix[0][0]; \n"
			"    normal.y *= gl_ProjectionMatrix[1][1]; \n"
			"    clip.xy += normal.xy * clip.w * outlineWidth;\n"
			"    gl_Position = clip; \n"
			"} \n";

		std::stringstream fs_;  fs_ << "#version 330\n"
			"void main() \n"
			"{ \n"
			<< "float a = " << ((1.f / N) * (N - i)) << " ;\n"
			<< " gl_FragColor = vec4(1,0,0, 1);\n"
			<< "}\n";

		string ss = fs_.str();
		const char *FS = ss.c_str();

		osg::Program *program = new osg::Program();
		program->addShader(new osg::Shader(osg::Shader::VERTEX, VS));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, FS));

		osg::AutoTransform* at = new osg::AutoTransform;
		at->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
		root->addChild(at);

		osg::Group *outline = new osg::Group();
		{
			osg::Program *program = new osg::Program();
			program->addShader(new osg::Shader(osg::Shader::VERTEX, VS));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, FS));

			osg::StateSet *outlineSS = outline->getOrCreateStateSet();
			outlineSS->setRenderBinDetails(1, "RenderBin");
			outlineSS->setAttributeAndModes(
				new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
			outlineSS->setMode(GL_CULL_FACE, 0);
			outlineSS->setAttribute(program);
			outlineSS->addUniform(new osg::Uniform("outlineWidth", offset));
			outlineSS->setAttributeAndModes(new osg::LineWidth(2),
				osg::StateAttribute::ON);
			outline->addChild(node);

			outlineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			outlineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
			// outlineSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			outlineSS->setAttributeAndModes(
				new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
					osg::BlendFunc::ONE_MINUS_SRC_ALPHA),
				osg::StateAttribute::ON);
		}

		osg::Group *outline2 = new osg::Group();
		{
			osg::StateSet *outlineSS = outline2->getOrCreateStateSet();
			outlineSS->setRenderBinDetails(1, "RenderBin");
			outlineSS->setAttributeAndModes(
				new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
			outlineSS->setMode(GL_CULL_FACE, 0);
			outlineSS->setAttribute(program);
			outlineSS->addUniform(new osg::Uniform("outlineWidth", -offset));
			outlineSS->setAttributeAndModes(new osg::LineWidth(2),
				osg::StateAttribute::ON);
			outline2->addChild(node);

			outlineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			outlineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
			// outlineSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			outlineSS->setAttributeAndModes(
				new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
					osg::BlendFunc::ONE_MINUS_SRC_ALPHA),
				osg::StateAttribute::ON);
		}

		at->addChild(outline);
		at->addChild(outline2);
	}

	osg::Group *model = new osg::Group();
	model->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");
	model->addChild(node);
	root->addChild(model);
}
