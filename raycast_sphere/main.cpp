// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"

osgViewer::Viewer* g_viewer = nullptr;

//使用osg内置更方便
//class MVPCallback : public osg::Uniform::Callback
//{
//public:
//	MVPCallback(osg::Camera* camera, const osg::Matrix& mat) : mCamera(camera), mat_(mat)
//	{
//	}
//	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
//	{
//		osg::Matrix modelView = mCamera->getViewMatrix();
//		uniform->set(mat_ * modelView);
//	}
//
//private:
//	osg::Camera* mCamera;
//	osg::Matrix mat_;
//};

//-------------------------------------------------------------
osg::Group* ff()
{
	osg::Group* root = new osg::Group;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.vert"));
	program->addShader(osgDB::readShaderFile(shader_dir() + "/raycast_sphere.frag"));

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
			root->addChild(pat);
			pat->setPosition(osg::Vec3(10 * i, 10 * j, 0));

			osg::Node* n = osgDB::readNodeFile(shader_dir() + "/res/cube.obj");
			pat->addChild(n);

			osg::StateSet* ss = n->getOrCreateStateSet();

			//osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
			osg::ref_ptr<osg::CullFace> cullface = new osg::CullFace(osg::CullFace::FRONT);
			ss->setAttributeAndModes(cullface, osg::StateAttribute::ON);
			//geode->getOrCreateStateSet()->setAttributeAndModes(model);
			
			ss->setAttributeAndModes(program);
			ss->addUniform(new osg::Uniform("center", osg::Vec3(10 * i, 10 * j, 0)));
			ss->addUniform(new osg::Uniform("radius", 1.0f));

			osg::Geode* geode = dynamic_cast<osg::Geode*>(n->asGroup()->getChild(0));
			osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));
			geometry->setVertexAttribArray(0, geometry->getVertexArray());
			geometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
			program->addBindAttribLocation("aPos", 0);

			geometry->setVertexAttribArray(1, geometry->getNormalArray());
			geometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
			program->addBindAttribLocation("aNormal", 1);

			//osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "modelViewMat"));
			//u_MVP->setUpdateCallback(new MVPCallback(g_viewer->getCamera(), pat->getWorldMatrices()[0]));
			//ss->addUniform(u_MVP);
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
		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
		}

		return false;
	}

	osgViewer::Viewer* view_;
};

int main()
{
	osgViewer::Viewer view;
	g_viewer = &view;

	osg::Group*		  root = new osg::Group;
	root->addChild(ff());

	view.setSceneData(root);

	EventCallback* c = new EventCallback;
	c->view_		 = &view;
	view.addEventHandler(c);

	add_event_handler(view);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	view.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	return view.run();
}
