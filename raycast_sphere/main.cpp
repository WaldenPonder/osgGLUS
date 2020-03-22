// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"

osgViewer::Viewer* g_viewer = nullptr;
int				   g_size   = 60;

osg::Group* g_group1 = nullptr;
osg::Group* g_group2 = nullptr;

//-------------------------------------------------------------
osg::Group* scene1()
{
	osg::Group* root = new osg::Group;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.vert"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.frag"));

	osg::Node* cube = osgDB::readNodeFile(shader_dir() + "/res/cube.obj");

	for (int i = 0; i < g_size; i++)
	{
		for (int j = 0; j < g_size; j++)
		{
			for (int k = 0; k < g_size; k++)
			{
				osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
				root->addChild(pat);
				pat->setPosition(osg::Vec3(10 * i, 10 * j, k * 10));

				pat->addChild(cube);

				osg::StateSet* ss = cube->getOrCreateStateSet();

				//osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
				//geode->getOrCreateStateSet()->setAttributeAndModes(model);

				ss->setAttributeAndModes(program);
				ss->addUniform(new osg::Uniform("center", osg::Vec3(10 * i, 10 * j, k * 10)));
				ss->addUniform(new osg::Uniform("radius", 1.0f));

				osg::Geode*	geode	= dynamic_cast<osg::Geode*>(cube->asGroup()->getChild(0));
				osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));
				geometry->setVertexAttribArray(0, geometry->getVertexArray());
				geometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
				program->addBindAttribLocation("aPos", 0);
			}
		}
	}

	return root;
}

osg::Group* scene2()
{
	osg::Group* root = new osg::Group;

	osg::ref_ptr<osg::Geode>		 geode  = new osg::Geode;
	osg::ref_ptr<osg::ShapeDrawable> sphere = new osg::ShapeDrawable(new osg::Sphere());
	geode->addDrawable(sphere);
	sphere->setColor(osg::Vec4(1, 0, 0, 1));

	for (int i = 0; i < g_size; i++)
	{
		for (int j = 0; j < g_size; j++)
		{
			for (int k = 0; k < g_size; k++)
			{
				osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
				root->addChild(pat);
				pat->setPosition(osg::Vec3(10 * i, 10 * j, k * 10));
				pat->addChild(geode);
			}
		}
	}

	return root;
}

class EventCallback : public osgGA::GUIEventHandler
{
 public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/,
						osg::Object* object, osg::NodeVisitor* nv)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == '1')
			{
				g_group1->setNodeMask(~0);
				g_group2->setNodeMask(0);
				cout << "group 1\n";
			}
			else if (ea.getKey() == '2')
			{
				g_group1->setNodeMask(0);
				g_group2->setNodeMask(~0);
				cout << "group 2\n";
			}
		}
		return false;
	}
};

int main()
{
	osgViewer::Viewer view;
	g_viewer = &view;

	osg::Group* root = new osg::Group;

	osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT);
	root->getOrCreateStateSet()->setAttributeAndModes(cullface, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

	root->addChild(g_group1 = scene1());
	root->addChild(g_group2 = scene2());
	g_group2->setNodeMask(0);

	view.setSceneData(root);
	add_event_handler(view);
	view.addEventHandler(new EventCallback);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	view.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	return view.run();
}
