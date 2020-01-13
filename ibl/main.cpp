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

osg::TextureCubeMap* equirectangular2Cubemap(osg::Group* root)
{
	osg::Node* n = osgDB::readNodeFile(shader_dir() + "/cube.obj");

	//equirectangular To Cubemap
	osg::TextureCubeMap* env_cube_texture = new osg::TextureCubeMap;
	{
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
		osg::Camera* first_fbo = new osg::Camera;
		root->addChild(first_fbo);
		first_fbo->setViewMatrix(view_mats[i]);
		first_fbo->setRenderOrder(osg::Camera::PRE_RENDER);
		first_fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		first_fbo->setProjectionMatrix(osg::Matrix::perspective(osg::PI_4f, 1., .1f, 10.f));
		first_fbo->setViewport(0, 0, 512, 512);
		first_fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		first_fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		first_fbo->attach(osg::Camera::COLOR_BUFFER, env_cube_texture, 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
		first_fbo->addChild(n);
	}
	
	//-------------------------------------------------------------设置shader
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_1.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_1.frag"));

	osg::Image* img = osgDB::readImageFile(shader_dir() + "/Playa_Sunrise/Playa_Sunrise_Env.hdr");
	//osg::Image* img = osgDB::readImageFile(shader_dir() + "/Ridgecrest_Road/Ridgecrest_Road_Ref.hdr");

	osg::Texture2D* texture = new osg::Texture2D(img);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	texture->setInternalFormat(GL_RGBA16F_ARB);
	texture->setSourceFormat(GL_RGBA);
	texture->setSourceType(GL_FLOAT);
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));

	return env_cube_texture;
}

void renderCubeMap(osg::Group* root, osg::TextureCubeMap* env_cube_texture)
{
	osg::Node* n = osgDB::readNodeFile(shader_dir() + "/cube.obj");
	root->addChild(n);

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_2.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_2.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, env_cube_texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("env_cube_texture", 0));
}

int main()
{
	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
		
	osg::TextureCubeMap* env_cube_texture = equirectangular2Cubemap(root);
	renderCubeMap(root, env_cube_texture);

	view.setSceneData(root);
	add_event_handler(view);
	view.getCamera()->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();

	return view.run();
}
