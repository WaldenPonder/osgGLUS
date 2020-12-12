// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"

osgViewer::Viewer* g_viewer = nullptr;
int				   g_size   = 5;

osg::Group* g_group1 = nullptr;
osg::Group* g_group2 = nullptr;
osg::Group* g_root = nullptr;

//-------------------------------------------------------------
osg::Group* scene1()
{
	g_group1 = new osg::Group;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.vert"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.frag"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.geom"));

	osg::Node* cube = osgDB::readNodeFile(shader_dir() + "/res/cube.obj");
		
	osg::Matrix windowMat = g_viewer->getCamera()->getViewport()->computeWindowMatrix();
	osg::Uniform* u_mat = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "windowMat");
	u_mat->set(windowMat);

	g_group1->getOrCreateStateSet()->setAttributeAndModes(program);

	for (int i = 0; i < g_size; i++)
	{
		for (int j = 0; j < g_size; j++)
		{
			for (int k = 0; k < g_size; k++)
			{
				osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
				g_group1->addChild(pat);
				pat->setPosition(osg::Vec3(10 * i, 10 * j, k * 10));

				pat->addChild(cube);

				osg::StateSet* ss = cube->getOrCreateStateSet();

				//osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
				//geode->getOrCreateStateSet()->setAttributeAndModes(model);
								
				ss->addUniform(new osg::Uniform("center", osg::Vec3(10 * i, 10 * j, k * 10)));
				ss->addUniform(new osg::Uniform("radius", 1.0f));
				ss->addUniform(u_mat);

				osg::Geode*	geode	= dynamic_cast<osg::Geode*>(cube->asGroup()->getChild(0));
				osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));
				geometry->setVertexAttribArray(0, geometry->getVertexArray());
				geometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
				program->addBindAttribLocation("aPos", 0);
			}
		}
	}

	return g_group1;
}

osg::Group* scene2()
{
	g_group2 = new osg::Group;

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
				g_group2->addChild(pat);
				pat->setPosition(osg::Vec3(10 * i, 10 * j, k * 10));
				pat->addChild(geode);
			}
		}
	}

	return g_group2;
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
			else if (ea.getKey() == 'a')
			{
				g_root->addChild(g_group1 = scene1());
				g_root->addChild(g_group2 = scene2());
				g_group2->setNodeMask(0);   
			}
			else if(ea.getKey() == 'b')
			{
				osg::ref_ptr<osg::Program> program = new osg::Program;
				program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.vert"));
				program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.frag"));
				program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.geom"));

				osg::Matrix windowMat = g_viewer->getCamera()->getViewport()->computeWindowMatrix();
				osg::Uniform* u_mat = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "windowMat");
				u_mat->set(windowMat);

				g_group1->getOrCreateStateSet()->setAttributeAndModes(program);
			}
		}
		return false;
	}
};

int main()
{
	osgViewer::Viewer view;
	g_viewer = &view;
	view.realize();

	g_root = new osg::Group;

	osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT);
	g_root->getOrCreateStateSet()->setAttributeAndModes(cullface, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	   
	view.setSceneData(g_root);
	
	view.addEventHandler(new EventCallback);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	
	view.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	add_event_handler(view);

	return view.run();
}
