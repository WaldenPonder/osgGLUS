#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <osg/Depth>
#include <random>

//http://www.hdrlabs.com/sibl/archive.html
//http://www.codinglabs.net/article_physically_based_rendering.aspx
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://metashapes.com/blog/realtime-image-based-lighting-using-spherical-harmonics/

/*
  纹理为什么会乱


  位置为什么会乱
*/

//#define RENDER_SIMPLE_CUBE

/*
牛的方向： X向右，Y向外，Z向上
*/

namespace g
{
	osg::Node* skybox;
	bool rotX;
	bool rotY;
	bool rotZ;
}

osg::Node* readCube()
{
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	//osg::Node*						n	= osgDB::readNodeFile(shader_dir() + "/cube.obj");
	//pat->addChild(n);

	// pat->setAttitude(osg::Quat(osg::PI_2f, osg::X_AXIS));
	
	//cube 原本Y向上

	osg::Geode* geode = new osg::Geode;
	pat->addChild(geode);
	osg::Geometry* geometry = new osg::Geometry;
	geode->addDrawable(geometry);
	geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 36));
	osg::Vec3Array* arr = new osg::Vec3Array;
	geometry->setVertexArray(arr);
	geometry->setUseVertexBufferObjects(true);

	// back face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));

	//front face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));

	// left face
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));

	// right face
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));

	// bottom face
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, -1.0f, -1.0f));

	//top face
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, -1.0f));
	arr->push_back(osg::Vec3(-1.0f, 1.0f, 1.0f));

	return pat;
}

class CameraPostdrawCallback : public osg::Camera::DrawCallback
{
public:
	osg::observer_ptr<osg::Group> once_group_;
	osg::PositionAttitudeTransform* scene_;

	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		once_group_->setNodeMask(0);

		if (g::rotX)
		{
			rot_ += delta;
			scene_->setAttitude(osg::Quat(rot_, osg::X_AXIS));
		}
		else if (g::rotY)
		{
			rot_ += delta;
			scene_->setAttitude(osg::Quat(rot_, osg::Y_AXIS));
		}
		else if (g::rotZ)
		{
			rot_ += delta;
			scene_->setAttitude(osg::Quat(rot_, osg::Z_AXIS));
		}
		else
		{
			scene_->setAttitude(osg::Quat(0, osg::Z_AXIS));
		}
	}

	mutable float rot_ = 0;
	float delta = .05;
};

class EventCallback : public osgGA::GUIEventHandler
{
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* nv)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_X)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotX = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Y)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotY = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Z)
			{
				g::rotX = g::rotY = g::rotZ = false;
				g::rotZ = true;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Space)
			{
				g::rotX = g::rotY = g::rotZ = false;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_O)
			{
				static bool flag = true;
				flag = !flag;
				g::skybox->setNodeMask(flag ? ~0 : 0);
			}
		}
		return false;
	}
};

struct ComputeBoundingSphereCallback : public osg::Node::ComputeBoundingSphereCallback
{
	virtual osg::BoundingSphere computeBound(const osg::Node&) const
	{
		osg::BoundingSphere bs;
		return bs;
	}
};


void cubeTextureAndViewMats(osg::TextureCubeMap* cube_texture, vector<osg::Matrix>& view_mats, int size)
{
	cube_texture->setTextureSize(size, size);
	cube_texture->setInternalFormat(GL_RGB16F_ARB);
	cube_texture->setSourceFormat(GL_RGB);
	cube_texture->setSourceType(GL_FLOAT);
	cube_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
	cube_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	cube_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);

	float DISTANCE = 73.f;

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(-DISTANCE, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(DISTANCE, 0.0f, 0.0f), osg::Vec3(-1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, DISTANCE, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, -DISTANCE, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f)));

	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, DISTANCE), osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, -DISTANCE), osg::Vec3(0.0f, 0.0f, -1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
}

osg::TextureCubeMap* equirectangular2Envmap(osg::Group* root)
{
	const float TEXTURE_SIZE = 1024.f;
	osg::Node*	n			 = readCube();

	//equirectangular To Cubemap
	osg::TextureCubeMap* env_cube_texture = new osg::TextureCubeMap;
	vector<osg::Matrix>	 view_mats;

	cubeTextureAndViewMats(env_cube_texture, view_mats, TEXTURE_SIZE);

#ifdef RENDER_SIMPLE_CUBE
	root->addChild(n);
#else
	for (int i = 0; i < 6; i++)
	{
		osg::Camera* fbo = new osg::Camera;
		root->addChild(fbo);
		fbo->setRenderOrder(osg::Camera::PRE_RENDER);
		fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		fbo->setViewMatrix(view_mats[i]);
		fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_2f, 1, .1f, 10.f));
		fbo->setViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
		fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo->attach(osg::Camera::COLOR_BUFFER, env_cube_texture, 0, i);
		fbo->addChild(n);
	}
#endif

	//-------------------------------------------------------------设置shader
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_1.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_1.frag"));

	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Playa_Sunrise/Playa_Sunrise_Env.hdr");	
	osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Playa_Sunrise/Playa_Sunrise_8k.jpg");

	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Ridgecrest_Road/Ridgecrest_Road_Env.hdr");
	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Ridgecrest_Road/Ridgecrest_Road_4k_Bg.jpg");

	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Walk_Of_Fame/Mans_Outside_Env.hdr");
	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/MonValley_Lookout/MonValley_A_LookoutPoint_Env.hdr");
	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Sierra_Madre_B/Sierra_Madre_B_Env.hdr");

	osg::Texture2D* texture = new osg::Texture2D(img);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	texture->setInternalFormat(GL_RGB16F_ARB);
	texture->setSourceFormat(GL_RGB);
	texture->setSourceType(GL_FLOAT);
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));

	return env_cube_texture;
}

osg::TextureCubeMap* envMap2IrradianceMap(osg::Group* root, osg::TextureCubeMap* env_cube_texture)
{
	osg::Node* n = readCube();

	const float TEXTURE_SIZE = 64.f;
	//equirectangular To Cubemap
	osg::TextureCubeMap* irradiance_map = new osg::TextureCubeMap;
	vector<osg::Matrix>	 view_mats;
	cubeTextureAndViewMats(irradiance_map, view_mats, TEXTURE_SIZE);

	for (int i = 0; i < 6; i++)
	{
		osg::Camera* fbo = new osg::Camera;
		root->addChild(fbo);

		fbo->setRenderOrder(osg::Camera::PRE_RENDER);
		fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		fbo->setViewMatrix(view_mats[i]);
		fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_2f, 1, .1f, 10.f));
		fbo->setViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
		fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo->attach(osg::Camera::COLOR_BUFFER, irradiance_map, 0, i);
		fbo->addChild(n);
	}

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_2.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_2.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));

	return irradiance_map;
}

osg::Node* renderSkyBox(osg::Group* root, osg::TextureCubeMap* env_cube_texture)
{
	osg::Node*						n	= readCube();
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	const float						s	= 400.f;
	pat->setScale(osg::Vec3(s, s, s));
	pat->setAttitude(osg::Quat(osg::PI_2f, osg::X_AXIS));
	pat->addChild(n);
	root->addChild(pat);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_skybox.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_skybox.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));
	n->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
    n->getOrCreateStateSet()->setAttribute(new osg::Depth(osg::Depth::NEVER));
	pat->setComputeBoundingSphereCallback(new ComputeBoundingSphereCallback);
	g::skybox = pat;
	return pat;
}

osg::PositionAttitudeTransform* renderScene(osg::Group* root, osg::TextureCubeMap* irradiance_map)
{
	//osg::Node*						n = osgDB::readNodeFile("F:\\360Downloads\\model\\dddd.osgb");
	//osg::Node*						n	= osgDB::readNodeFile("F:\\360Downloads\\model\\abc.osgb");
	osg::Node* n = osgDB::readNodeFile("cow.osg");

	osg::ComputeBoundsVisitor cbv;
	n->accept(cbv);
	osg::Vec3 center = cbv.getBoundingBox().center();

	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	const float						s	= 1.f;
	pat->setScale(osg::Vec3(s, s, s));
	pat->setPosition(-center);
	pat->addChild(n);
	root->addChild(pat);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_final.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_final.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, irradiance_map);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("irradiance_map", 0));

	return pat;
}

int main()
{
	osgViewer::Viewer view;
	osg::Group*		  root = new osg::Group;

	osg::Group* once_group = new osg::Group;
	root->addChild(once_group);

	osg::TextureCubeMap* env_cube_texture = equirectangular2Envmap(once_group);

#ifndef RENDER_SIMPLE_CUBE
	osg::TextureCubeMap* irradiance_map = envMap2IrradianceMap(once_group, env_cube_texture);
	
	//osgDB::writeObjectFile(*irradiance_map, shader_dir() + "/ibl/irradiance_map.osgb");
	//osg::TextureCubeMap* irradiance_map = dynamic_cast<osg::TextureCubeMap*>(osgDB::readObjectFile(shader_dir() + "/ibl/irradiance_map.osgb"));

	osg::Node* skybox = renderSkyBox(root, env_cube_texture);
	osg::PositionAttitudeTransform* scene = renderScene(root, irradiance_map);
	
	CameraPostdrawCallback* callback = new CameraPostdrawCallback;
	callback->once_group_			 = once_group;
	callback->scene_ = scene;

	view.getCamera()->addPostDrawCallback(callback);
	view.getCamera()->setEventCallback(new EventCallback);
#endif

	view.setSceneData(root);
	add_event_handler(view);
	view.getCamera()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();

	return view.run();
}
