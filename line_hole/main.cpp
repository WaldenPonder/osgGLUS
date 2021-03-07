// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osgDB/ReadFile"
#include <osg/TextureBuffer>

#define TEXTURE_SIZE1 1024
#define TEXTURE_SIZE2 1024

//#define TEXTURE_SIZE1 2048
//#define TEXTURE_SIZE2 2048

osg::Group* g_root;
osg::BoundingBox g_line_bbox;
osg::Vec4 CLEAR_COLOR(0, 0, 0, 1);// (204 / 255, 213 / 255, 240 / 255, 1);

//------------------------------------------------------------------------------------------
class MVPCallback : public osg::Uniform::Callback
{
public:
	MVPCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		osg::Matrix modelView = mCamera->getViewMatrix();
		osg::Matrix projectM = mCamera->getProjectionMatrix();
		uniform->set(modelView * projectM);
	}

private:
	osg::Camera* mCamera;
};

osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer1;
osg::ref_ptr<osg::TextureBuffer>  g_textureBuffer2;

osg::ref_ptr<osg::TextureBuffer> create_tbo(const vector<int>& data)
{
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(data.size(), 1, 1, GL_R32I, GL_INT);

	for (int i = 0; i < data.size(); i++)
	{
		int* ptr = (int*)image->data(i);
		*ptr = data[i];
	}

	osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
	tbo->setImage(image.get());
	tbo->setInternalFormat(GL_R32I);
	return tbo;
}

//https://blog.csdn.net/qq_16123279/article/details/82463266

osg::Geometry* createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec3>& colors,
	const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode = osg::PrimitiveSet::LINE_LOOP)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;

	//传递给shader
	osg::ref_ptr<osg::Vec3Array> a_color = new osg::Vec3Array;
	osg::ref_ptr<osg::UIntArray> a_id = new osg::UIntArray;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> a_pos = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		a_pos->push_back(allPTs[i]);
	}

	osg::ref_ptr<osg::ElementBufferObject> ebo = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices = new osg::DrawElementsUInt(mode);

	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		indices->push_back(i);

		if(i < colors.size())
		   a_color->push_back(colors[i]);
		else
			a_color->push_back(colors.back());

		if(i < ids.size())
		   a_id->push_back(ids[i]);
		else
			a_id->push_back(ids.back());
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true);

	osg::StateSet* ss = pGeometry->getOrCreateStateSet();
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("LINESTRIPE");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/line_hole.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(a_pos);
	pGeometry->setVertexAttribArray(0, a_pos, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	pGeometry->setVertexAttribArray(1, a_color, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_color", 1);

	pGeometry->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_id", 2);
	   
	//-----------------------------------------------uniform
	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);

	//osg::Matrix windowMat = camera->getViewport()->computeWindowMatrix();
	//osg::Uniform* u_mat = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "windowMat");
	//u_mat->set(windowMat);
	//ss->addUniform(u_mat);

	return pGeometry.release();
}


#include "scene.h"

std::vector<osg::Texture2D*> createRttCamera(osgViewer::Viewer* viewer)
{
	auto create_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_RGBA);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		return texture2d;
	};
	
	auto create_id_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_R32UI);
		texture2d->setSourceFormat(GL_R32UI);
		texture2d->setSourceType(GL_UNSIGNED_INT);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		return texture2d;
	};

	auto create_depth_texture = []()
	{
		// Setup shadow texture
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture->setInternalFormat(GL_DEPTH_COMPONENT);
		texture->setShadowComparison(true);
		texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		// the shadow comparison should fail if object is outside the texture
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setBorderColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		return texture;
	};

	osg::Camera* rttCamera = new osg::Camera;
	g_root->addChild(rttCamera);
	// set up the background color and clear mask.
	rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set view
	rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	rttCamera->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);

	// set the camera to render before the main camera.
	rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);

	// tell the camera to use OpenGL frame buffer object where supported.
	rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	osg::Texture2D* texture = create_texture();
	osg::Texture2D* depthTexture = create_texture();
	osg::Texture2D* idTexture = create_id_texture();
	osg::Texture2D* linePtTexture = create_texture();

	// attach the texture and use it as the color buffer.
	rttCamera->attach(osg::Camera::COLOR_BUFFER0, texture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER1, idTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER2, depthTexture,0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER3, linePtTexture, 0, 0, false);

	osg::Camera* mainCamera = viewer->getCamera();
	rttCamera->setProjectionMatrix(mainCamera->getProjectionMatrix());
	rttCamera->setViewMatrix(mainCamera->getViewMatrix());
	rttCamera->addPreDrawCallback(new CameraPredrawCallback(rttCamera, mainCamera));

	rttCamera->addChild(create_lines(*viewer));

	return { texture, depthTexture, idTexture, linePtTexture};
}

osg::Geometry* myCreateTexturedQuadGeometry(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner, const osg::Vec3& widthVec, 
	const osg::Vec3& heightVec, float l = 0, float b = 0, float r = 1, float t = 1)
{
	using namespace osg;
	Geometry* geom = new Geometry;

	Vec3Array* coords = new Vec3Array(4);
	(*coords)[0] = corner + heightVec;
	(*coords)[1] = corner;
	(*coords)[2] = corner + widthVec;
	(*coords)[3] = corner + widthVec + heightVec;
	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	Vec2Array* tcoords = new Vec2Array(4);
	(*tcoords)[0].set(l, t);
	(*tcoords)[1].set(l, b);
	(*tcoords)[2].set(r, b);
	(*tcoords)[3].set(r, t);
	geom->setVertexAttribArray(1, tcoords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_uv", 1);

	DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
	elems->push_back(0);
	elems->push_back(1);
	elems->push_back(2);

	elems->push_back(2);
	elems->push_back(3);
	elems->push_back(0);
	geom->addPrimitiveSet(elems);

	return geom;
}

//最终显示的贴图
osg::Camera* createHudCamera(osgViewer::Viewer* viewer, std::vector<osg::Texture2D*> TEXTURES)
{
	osg::Geode* geode_quat = nullptr;
	osg::Geometry* screenQuat = nullptr;
	osg::Camera* hud_camera_ = new osg::Camera;

	osg::Camera* mainCamera = viewer->getCamera();
	mainCamera->setClearColor(CLEAR_COLOR);
	osg::Viewport* vp = mainCamera->getViewport();

	float w_ = TEXTURE_SIZE1;
	float h_ = TEXTURE_SIZE2;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	screenQuat = myCreateTexturedQuadGeometry(program, osg::Vec3(), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	{
		hud_camera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hud_camera_->setProjectionMatrixAsOrtho2D(0, w_, 0, h_);
		hud_camera_->setViewMatrix(osg::Matrix::identity());
		hud_camera_->setRenderOrder(osg::Camera::POST_RENDER);
		hud_camera_->setClearColor(CLEAR_COLOR);
		hud_camera_->setClearMask(GL_DEPTH_BUFFER_BIT);
		hud_camera_->getOrCreateStateSet()->setMode(GL_LIGHTING,
			osg::StateAttribute::OFF);

		geode_quat = new osg::Geode;
		hud_camera_->addChild(geode_quat);
		screenQuat->setUseVertexBufferObjects(true);
		geode_quat->addChild(screenQuat);
		geode_quat->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		geode_quat->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		screenQuat->setDataVariance(osg::Object::DYNAMIC);
		screenQuat->setSupportsDisplayList(false);

		osg::StateSet* ss = geode_quat->getOrCreateStateSet();
		ss->setTextureAttributeAndModes(0, TEXTURES[0], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(1, TEXTURES[1], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(2, TEXTURES[2], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(3, TEXTURES[3], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(4, g_textureBuffer1, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(5, g_textureBuffer2, osg::StateAttribute::ON);
				
		osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole_quad.vert");
		osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole_quad.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("depthTexture", 1));
		ss->addUniform(new osg::Uniform("idTexture", 2));
		ss->addUniform(new osg::Uniform("linePtTexture", 3));
		ss->addUniform(new osg::Uniform("textureBuffer1", 4));
		ss->addUniform(new osg::Uniform("textureBuffer2", 5));

		osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
		u_MVP->setUpdateCallback(new MVPCallback(hud_camera_));
		ss->addUniform(u_MVP);

	//	program->addBindAttribLocation("a_pos", 0);
	//	program->addBindAttribLocation("a_uv", 1);
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	}

	return hud_camera_;
}

int setUp(osgViewer::Viewer &view)
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
	//size = 134217728
	//int size = 0;
	//glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &size);

	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
	g_root = root;
	view.setSceneData(root);

    setUp(view);

	std::vector<osg::Texture2D*> textures = createRttCamera(&view);
	osg::Camera* hud_camera = createHudCamera(&view, textures);
	root->addChild(hud_camera);

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

	//osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	

	return view.run();
}