// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <osg/Texture2D>
#include <random>

//http://www.codinglabs.net/article_physically_based_rendering.aspx
//https://learnopengl.com/PBR/IBL/Specular-IBL
//https://metashapes.com/blog/realtime-image-based-lighting-using-spherical-harmonics/

//从HDR文件，得到带贴图的CUBE


void configProgram(osg::Node* n)
{
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl_1.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl_1.frag"));
			
	osg::Image* img = osgDB::readImageFile(shader_dir() + "/Playa_Sunrise/Playa_Sunrise_Env.hdr");
	osg::Texture2D* texture = new osg::Texture2D(img);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);

	texture->setInternalFormat(GL_RGBA16F_ARB);
	//texture->setSourceFormat(GL_RGBA);
	//texture->setSourceType(GL_FLOAT);
	n->getOrCreateStateSet()->setAttributeAndModes(p);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));
}

int main()
{
	osgViewer::Viewer view;
	osg::Group* root = new osg::Group;
	
	osg::Node* n = osgDB::readNodeFile(shader_dir() + "/cube.obj");
	configProgram(n);
	root->addChild(n);

	view.setSceneData(root);
	add_event_handler(view);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
