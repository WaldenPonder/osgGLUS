// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <osg/Texture2D>
#include <osg/TextureCubeMap>
#include <random>

//http://www.codinglabs.net/article_physically_based_rendering.aspx
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://metashapes.com/blog/realtime-image-based-lighting-using-spherical-harmonics/

//从HDR文件，得到带贴图的CUBE
//得到辐照度cube map

osg::TextureCubeMap* equirectangular2Envmap(osg::Group* root)
{
	osg::Node* n = osgDB::readNodeFile(shader_dir() + "/cube.obj");

	//equirectangular To Cubemap
	osg::TextureCubeMap* env_cube_texture = new osg::TextureCubeMap;
	{
		env_cube_texture->setTextureSize(512, 512);
		env_cube_texture->setInternalFormat(GL_RGBA16F_ARB);
		env_cube_texture->setSourceFormat(GL_RGB);
		env_cube_texture->setSourceType(GL_FLOAT);
		env_cube_texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		env_cube_texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		env_cube_texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		env_cube_texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		env_cube_texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	}

	vector<osg::Matrix> view_mats;
	{
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(-1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	}

	for (int i = 0; i < 6; i++)
	{
		osg::Camera* fbo = new osg::Camera;
		root->addChild(fbo);
		
		fbo->setRenderOrder(osg::Camera::PRE_RENDER);
		fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		fbo->setViewMatrix(view_mats[i]);
		fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_4f, 1., .1f, 10.f));
		fbo->setViewport(0, 0, 512, 512);
		fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo->attach(osg::Camera::COLOR_BUFFER, env_cube_texture, 0, i);
		fbo->addChild(n);
	}
	
	//-------------------------------------------------------------设置shader
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_1.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_1.frag"));

	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/Playa_Sunrise/Playa_Sunrise_Env.hdr");
	osg::Image* img = osgDB::readImageFile(shader_dir() + "/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr");

	osg::Texture2D* texture = new osg::Texture2D(img);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	texture->setInternalFormat(GL_RGBA16F_ARB);
	texture->setSourceFormat(GL_RGB);
	texture->setSourceType(GL_FLOAT);
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));

	return env_cube_texture;
}

osg::TextureCubeMap* envMap2IrradianceMap(osg::Group* root, osg::TextureCubeMap* env_cube_texture)
{
	osg::Node* n = osgDB::readNodeFile(shader_dir() + "/cube.obj");

	const float TEXTURE_SIZE = 32.f;
	//equirectangular To Cubemap
	osg::TextureCubeMap* irradiance_map = new osg::TextureCubeMap;
	{
		irradiance_map->setTextureSize(TEXTURE_SIZE, TEXTURE_SIZE);
		irradiance_map->setInternalFormat(GL_RGBA16F_ARB);
		irradiance_map->setSourceFormat(GL_RGB);
		irradiance_map->setSourceType(GL_FLOAT);
		irradiance_map->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
		irradiance_map->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
		irradiance_map->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
		irradiance_map->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		irradiance_map->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	}

	vector<osg::Matrix> view_mats;
	{
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(-1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, -1.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
		view_mats.push_back(osg::Matrix::lookAt(osg::Vec3(0.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, -1.0f), osg::Vec3(0.0f, -1.0f, 0.0f)));
	}

	for (int i = 0; i < 6; i++)
	{
		osg::Camera* fbo = new osg::Camera;
		root->addChild(fbo);

		fbo->setRenderOrder(osg::Camera::PRE_RENDER);
		fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		fbo->setViewMatrix(view_mats[i]);
		fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_4f, 1., .1f, 10.f));
		fbo->setViewport(0, 0, TEXTURE_SIZE, TEXTURE_SIZE);
		fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fbo->attach(osg::Camera::COLOR_BUFFER, irradiance_map, 0, i);
		fbo->addChild(n);
	}
	   	
	root->addChild(n);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_2.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_2.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));

	return irradiance_map;
}

void renderScene(osg::Group* root, osg::TextureCubeMap* irradiance_map)
{
	osg::Node* n = osgDB::readNodeFile("cow.osg");
	root->addChild(n);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_final.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_final.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, irradiance_map);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("irradiance_map", 0));
}

int main()
{
	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
		
	osg::TextureCubeMap* env_cube_texture = equirectangular2Envmap(root);
	osg::TextureCubeMap* irradiance_map = envMap2IrradianceMap(root, env_cube_texture);
	renderScene(root, irradiance_map);

	view.setSceneData(root);
	add_event_handler(view);
	view.getCamera()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();

	return view.run();
}
