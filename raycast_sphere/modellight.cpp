// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"

osgViewer::Viewer* g_viewer = nullptr;

osg::Group* scene()
{
	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/ibl/modelLit.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/ibl/modelLit.frag"));

	//
	osg::Image* img; 
	img = osgDB::readImageFile("D:\\glacier_diffuse.hdr");
	//img = osgDB::readImageFile("D:\\pisa_diffuse.hdr");
	osg::Texture2D* texture = new osg::Texture2D(img);
	texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
	texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

	texture->setInternalFormat(GL_RGB16F_ARB);
	texture->setSourceFormat(GL_RGB);
	texture->setSourceType(GL_FLOAT);

	osg::Group* root = new osg::Group;
	osg::Node* n = osgDB::readNodeFile("F:\\FileRecv\\桥梁模型_2.osgb");
	root->addChild(n);
	n->getOrCreateStateSet()->setAttributeAndModes(p, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));

	return root;
}

//class CameraPosCallback : public osg::Uniform::Callback
//{
//public:
//	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
//	{
//		osgGA::StandardManipulator* mani = dynamic_cast<osgGA::StandardManipulator*>(_viewer->getCameraManipulator());
//		if (mani)
//		{
//			osg::Vec3d eye;
//			osg::Quat  quat;
//			mani->getTransformation(eye, quat);
//
//			osg::Vec3 pos1 = eye * g_viewer->getCamera()->getViewMatrix();
//			uniform->set(pos1);
//		}
//	}
//};

osg::Vec3 g_lightPos;
class LightPosCallback : public osg::Uniform::Callback
{
public:
	LightPosCallback(osg::ref_ptr<osg::Camera> c) : camera_(c) {}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		osg::Vec3 pos(0, 0, 1000);
		pos = g_lightPos * camera_->getViewMatrix();
		uniform->set(pos);
	}

	osg::ref_ptr<osg::Camera> camera_;
};

osg::Group* scene2()
{
	osg::ref_ptr<osg::Uniform> metallic = new osg::Uniform("metallic", 66 / 100.f);
	osg::ref_ptr<osg::Uniform> roughness = new osg::Uniform("roughness", 40 / 100.f);
	osg::ref_ptr<osg::Uniform> ambient_factor = new osg::Uniform("ambient_factor", 40 / 100.f);
	osg::ref_ptr<osg::Uniform> albedo = new osg::Uniform("albedo", osg::Vec3(.5, .5, .5));
	//osg::ref_ptr<osg::Uniform> lightPos = new osg::Uniform("u_lightPosition", osg::Vec3());

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/pbr.vert"));
	p->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/pbr.frag"));

	osg::Group* root = new osg::Group;
	osg::Node* n = osgDB::readNodeFile("F:\\FileRecv\\桥梁模型_2.osgb");
	//n = osgDB::readNodeFile("F:\\FileRecv\\桥梁模型.osgb");
	root->addChild(n);
	n->getOrCreateStateSet()->setAttributeAndModes(p, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	//n->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture);
	//n->getOrCreateStateSet()->addUniform(new osg::Uniform("equirectangularMap", 0));

	n->getOrCreateStateSet()->addUniform(metallic);
	n->getOrCreateStateSet()->addUniform(roughness);
	n->getOrCreateStateSet()->addUniform(ambient_factor);
	n->getOrCreateStateSet()->addUniform(albedo);
	//n->getOrCreateStateSet()->addUniform(lightPos);
	//lightPos->setUpdateCallback(new LightPosCallback(g_viewer->getCamera()));

	//osg::ComputeBoundsVisitor cbv;
	//n->accept(cbv);
	//g_lightPos = cbv.getBoundingBox().corner(7) + osg::Vec3(0, 0, 100);

	return root;
}

int main()
{
	osgViewer::Viewer view;
	g_viewer = &view;

	view.setSceneData(scene2());
	add_event_handler(view);
	view.realize();
	view.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	return view.run();
}
