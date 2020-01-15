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
	osg::Group*						root;
	osg::Node*						skybox;
	osg::Group*						draw_once_group;
	osg::PositionAttitudeTransform* scene;
	bool							rotX	 = false;
	bool							rotY	 = false;
	bool							rotZ	 = false;
	bool							clearRot = false;
}  // namespace g

#include "main.h"

osg::TextureCubeMap* equirectangular2Envmap()
{
	const float TEXTURE_SIZE = 1024.f;
	osg::Node*	n			 = readCube();

	//equirectangular To Cubemap
	osg::TextureCubeMap* env_cube_texture = new osg::TextureCubeMap;
	vector<osg::Matrix>	 view_mats;

	cubeTextureAndViewMats(env_cube_texture, view_mats, TEXTURE_SIZE);

	for (int i = 0; i < 6; i++)
	{
		osg::Camera* fbo = new osg::Camera;
		g::draw_once_group->addChild(fbo);
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

	//-------------------------------------------------------------设置shader
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_1.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_1.frag"));

	osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Playa_Sunrise/Playa_Sunrise_Env.hdr");
	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/ibl/hdr/Playa_Sunrise/Playa_Sunrise_8k.jpg");

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

osg::TextureCubeMap* envMap2IrradianceMap(osg::TextureCubeMap* env_cube_texture)
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
		g::draw_once_group->addChild(fbo);
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

osg::Node* renderSkyBox(osg::TextureCubeMap* env_cube_texture)
{
	osg::Node*						n	= readCube();
	osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
	const float						s	= 400.f;
	pat->setScale(osg::Vec3(s, s, s));
	pat->setAttitude(osg::Quat(osg::PI_2f, osg::X_AXIS));
	pat->addChild(n);

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

osg::PositionAttitudeTransform* renderScene(osg::TextureCubeMap* irradiance_map)
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

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_final.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_final.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(1, irradiance_map);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("irradiance_map", 1));
	g::scene = pat;
	return pat;
}

int main()
{
	osgViewer::Viewer view;
	osg::Group*		  root = new osg::Group;
	g::root				   = root;

	osg::Group* draw_once_group = new osg::Group;
	g::draw_once_group			= draw_once_group;

	osg::TextureCubeMap* env_cube_texture = equirectangular2Envmap();
	osg::TextureCubeMap* irradiance_map = envMap2IrradianceMap(env_cube_texture);

	//osgDB::writeObjectFile(*irradiance_map, shader_dir() + "/ibl/irradiance_map.osgb");
	//osg::TextureCubeMap* irradiance_map = dynamic_cast<osg::TextureCubeMap*>(osgDB::readObjectFile(shader_dir() + "/ibl/irradiance_map.osgb"));

	osg::Node*						skybox = renderSkyBox(env_cube_texture);
	osg::PositionAttitudeTransform* scene  = renderScene(irradiance_map);

	view.getCamera()->setEventCallback(new EventCallback);
	view.getCamera()->setPostDrawCallback(new CameraPostdrawCallback);

	root->addChild(g::draw_once_group);
	root->addChild(g::skybox);
	root->addChild(g::scene);

	view.setSceneData(root);
	add_event_handler(view);
	view.getCamera()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();

	return view.run();
}
