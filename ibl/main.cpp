﻿#include "pch.h"
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
HDR纹理，太阳绘制的颜色有误。

*/

/*
牛的方向： X向右，Y向外，Z向上
*/

namespace g
{
	osg::Group*						 root;
	osg::Node*						 skybox;
	osg::Group*						 draw_once_group;
	osg::PositionAttitudeTransform*	 modelParent;
	bool							 rotX		= false;
	bool							 rotY		= false;
	bool							 rotZ		= false;
	bool							 clearRot	= false;
	bool							 needRedraw = false;
	int								 imageIndex;
	osg::ref_ptr<osg::Group>		 sceneData;
	vector<osg::ref_ptr<osg::Image>> images;
}  // namespace g

#include "main.h"

osg::TextureCubeMap* equirectangular2Envmap()
{
	const float TEXTURE_SIZE = 1024.f;
	osg::Node*	n			 = createCube();

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

	osg::Texture2D* texture = new osg::Texture2D(g::images[g::imageIndex % g::images.size()]);
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
	osg::Node* n = createCube();

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

//------------------------------------------------------------- PrefilterMap
osg::TextureCubeMap* createPrefilterMap(osg::TextureCubeMap* env_cube_texture)
{
	osg::Node* n = createCube();

	const float TEXTURE_SIZE = 128.f;
	//prefilterMap
	osg::TextureCubeMap* prefilter_map = new osg::TextureCubeMap;
	vector<osg::Matrix>	 view_mats;
	cubeTextureAndViewMats(prefilter_map, view_mats, TEXTURE_SIZE);
	prefilter_map->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);

	unsigned int maxMipLevels = 5;

	for (size_t mip = 0; mip < 5; mip++)
	{
		for (int i = 0; i < 6; i++)
		{
			unsigned int mipWidth = TEXTURE_SIZE * std::pow(0.5, mip);
			unsigned int mipHeight = TEXTURE_SIZE * std::pow(0.5, mip);
			
			osg::Camera* fbo = new osg::Camera;
			g::draw_once_group->addChild(fbo);
			fbo->setRenderOrder(osg::Camera::PRE_RENDER);
			fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			fbo->setViewMatrix(view_mats[i]);
			fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_2f, 1, .1f, 10.f));
			std::cout << mipWidth << "\t" << mipHeight << endl;

			fbo->setViewport(0, 0, mipWidth, mipHeight);
			fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
			fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			fbo->attach(osg::Camera::COLOR_BUFFER, prefilter_map, mip, i, true);
			fbo->addChild(n);
		}
	}
	
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_3_prefilter.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_3_prefilter.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("roughness", 0.3f));

	return prefilter_map;
}

//-------------------------------------------------------------brdf map
osg::Texture2D* createBRDFTexture(osg::TextureCubeMap* env_cube_texture)
{
	const float TEXTURE_SIZE = 512.f;

	osg::Texture2D* brdfLUTTexture = new osg::Texture2D;
	brdfLUTTexture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	brdfLUTTexture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	brdfLUTTexture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	brdfLUTTexture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	brdfLUTTexture->setTextureSize(TEXTURE_SIZE, TEXTURE_SIZE);
	brdfLUTTexture->setInternalFormat(GL_RG16F);
	brdfLUTTexture->setSourceFormat(GL_RG);
	brdfLUTTexture->setSourceType(GL_FLOAT);

	osg::Camera* fbo = new osg::Camera;
	g::draw_once_group->addChild(fbo);
	fbo->setRenderOrder(osg::Camera::PRE_RENDER);
	fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	//fbo->setViewMatrix(osg::Matrix::identity());
	//fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_2f, 1, .1f, 10.f));
	fbo->setViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
	fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
	fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	fbo->attach(osg::Camera::COLOR_BUFFER, brdfLUTTexture);

	osg::Node* n = createQuad();
	fbo->addChild(n);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/ibl_4_brdf.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/ibl_4_brdf.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("roughness", 0.3));
	p->addBindAttribLocation("aPos", 0);
	p->addBindAttribLocation("aTexCoords", 1);

	return brdfLUTTexture;
}

//-------------------------------------------------------------skybox
osg::Node* renderSkyBox(osg::TextureCubeMap* env_cube_texture)
{
	osg::Node*						n	= createCube();
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

//-------------------------------------------------------------renderScene
osg::PositionAttitudeTransform* renderScene(osg::TextureCubeMap* irradiance_map, osg::TextureCubeMap* prefilter_map, osg::Texture2D* brdf_map)
{
	//osg::Node*						n = osgDB::readNodeFile("F:\\360Downloads\\model\\dddd.osgb");
	//osg::Node*						n	= osgDB::readNodeFile("F:\\360Downloads\\model\\abc.osgb");
	osg::Node* n = osgDB::readNodeFile("F:\\aaa.osgb");

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

	n->getOrCreateStateSet()->setTextureAttributeAndModes(2, prefilter_map);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("prefilter_map", 2));

	n->getOrCreateStateSet()->setTextureAttributeAndModes(3, brdf_map);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("brdf_map", 3));
	   
	g::modelParent = pat;
	return pat;
}

//-------------------------------------------------------------setUp
osg::ref_ptr<osg::Group> setUp()
{
	osg::ref_ptr<osg::Group> root = new osg::Group;
	g::root						  = root;

	osg::Group* draw_once_group = new osg::Group;
	g::draw_once_group			= draw_once_group;

	osg::TextureCubeMap* env_cube_texture = equirectangular2Envmap();
	osg::TextureCubeMap* irradiance_map	  = envMap2IrradianceMap(env_cube_texture);
	osg::TextureCubeMap* prefilter_map = createPrefilterMap(env_cube_texture);
	osg::Texture2D* brdf_map = createBRDFTexture(env_cube_texture);

	//osgDB::writeObjectFile(*irradiance_map, shader_dir() + "/ibl/irradiance_map.osgb");
	//osg::TextureCubeMap* irradiance_map = dynamic_cast<osg::TextureCubeMap*>(osgDB::readObjectFile(shader_dir() + "/ibl/irradiance_map.osgb"));

	renderSkyBox(env_cube_texture);
	renderScene(irradiance_map, prefilter_map, brdf_map);

	root->addChild(g::draw_once_group);
	root->addChild(g::skybox);
	root->addChild(g::modelParent);

	//绘制 brdf  texture
	if(0)
	{
		osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
		osg::Geode* geode = new osg::Geode;
		osg::Geometry* quat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(100, 0, 0), osg::Vec3(0, 100, 0));
		pat->addChild(geode);
		pat->setAttitude(osg::Quat(osg::PI_2f, osg::X_AXIS));
		geode->addChild(quat);

		pat->setPosition(osg::Vec3(100, 10, 10));

		auto* ss = geode->getOrCreateStateSet();
		ss->setTextureAttributeAndModes(0, brdf_map);
		osg::Program* p = new osg::Program;
		p->addShader(osgDB::readShaderFile(shader_dir() + "/ibl/simple.vert"));
		p->addShader(osgDB::readShaderFile(shader_dir() + "/ibl/simple.frag"));
		ss->setAttributeAndModes(p);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		g::modelParent->addChild(pat);
	}

	return root;
}

int main()
{
	loadImages();
	g::imageIndex = g::images.size() * 100;

	osgViewer::Viewer		 view;
	osg::ref_ptr<osg::Group> sceneData = new osg::Group;
	g::sceneData					   = sceneData;

	view.getCamera()->setEventCallback(new EventCallback);
	view.getCamera()->setPostDrawCallback(new CameraPostdrawCallback);
	view.setSceneData(sceneData);
	
	sceneData->addChild(setUp());

	add_event_handler(view);
	view.getCamera()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();

	return view.run();
}
