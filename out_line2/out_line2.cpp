#include "pch.h"
#include "../common/common.h"

osg::Node* test1(osg::Node* n)
{
	osg::Group* p = new osg::Group;
	p->addChild(n);
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(shader_dir() + "/outline2/out_line.vert"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/outline2/out_line.geom"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/outline2/out_line.frag"));
	p->getOrCreateStateSet()->setAttributeAndModes(program);

	osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT);
	p->getOrCreateStateSet()->setAttributeAndModes(cullface, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	return p;
}

osg::Node* test0(osg::Node* n)
{
	osg::Group* p = new osg::Group;
	p->addChild(n);
	//osg::ref_ptr<osg::Program> program = new osg::Program;
	//program->addShader(osgDB::readShaderFile(shader_dir() + "/outline2/o.vert"));
	//program->addShader(osgDB::readShaderFile(shader_dir() + "/outline2/o.frag"));
	//p->getOrCreateStateSet()->setAttributeAndModes(program);

	//p->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	//p->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	//osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT);
	//p->getOrCreateStateSet()->setAttributeAndModes(cullface, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	return p;
}

int main(int argc, char** argv)
{
	osg::ArgumentParser arguments(&argc, argv);

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	osgViewer::Viewer viewer(arguments);

	osg::Group* root = new osg::Group;
	//auto* n = osgDB::readNodeFile("F:\\BaiduYunDownload\\A匝道2号桥.osgb");
	auto* n = osgDB::readNodeFile("F:/model/cube.obj");

	//root->addChild(test0(n));
	root->addChild(test1(n));

	viewer.setSceneData(root);

	add_event_handler(viewer);
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

	//viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	return viewer.run();
}