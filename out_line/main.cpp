#include "stdafx.h"
#include "../common/common.h"

#include "osg/AutoTransform"
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <sstream>
#include "HighlightSystem.h"

std::shared_ptr<HighlightSystem> g_highlightSystem;
void read_model(osg::Group* root);

class PickHandler : public osgGA::GUIEventHandler
{

 public:
	PickHandler() {}

	~PickHandler() {}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* view = dynamic_cast<osgViewer::Viewer*>(&aa);
			
		switch (ea.getEventType())
		{
		case (osgGA::GUIEventAdapter::FRAME):
		{
		}
		case (osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_A)
			{
				//osg::Viewport* vp = view->getCamera()->getViewport();
				//g_highlightSystem->reset();
				read_model(view->getSceneData()->asGroup());
			}
			break;
		}
		}
		return false;
	}
};

void read_model(osg::Group* root)
{
	osg::Group* modelGroup = new osg::Group;

	std::vector<osg::Vec3d> allPTs;
	allPTs.push_back(osg::Vec3(0, 0, 0));
	allPTs.push_back(osg::Vec3(100, 0, 0));
	allPTs.push_back(osg::Vec3(100, 100, 0));

	std::vector<osg::Vec3d> allPTs2;
	allPTs2.push_back(osg::Vec3(0, 0, 0));
	allPTs2.push_back(osg::Vec3(10, 0, 0));
	allPTs2.push_back(osg::Vec3(10, 10, 0));
	//root->addChild(modelGroup);
	{
		modelGroup->addChild(createLine(allPTs, osg::Vec4(0, 0, 1, 1), osg::PrimitiveSet::LINE_LOOP));
		//root->addChild(createLine(allPTs2, osg::Vec4(1, 0, 1, 1), osg::PrimitiveSet::LINE_LOOP));

		//osg::PositionAttitudeTransform* pat1 = new osg::PositionAttitudeTransform;
		//{
		//	osg::Node* node2 = osgDB::readNodeFile("E:\\Shader\\res\\teapot.obj");
		//	pat1->setPosition(osg::Vec3(0, 0, 7));
		//	pat1->addChild(node2);
		//	modelGroup->addChild(pat1);
		//}

		//osg::PositionAttitudeTransform* pat2 = new osg::PositionAttitudeTransform;
		//{
		//	osg::Node* node2 = osgDB::readNodeFile("F:\\FileRecv\\abc.osg");
		//	pat2->setPosition(osg::Vec3(0, 15, 0));
		//	pat2->addChild(node2);
		//	modelGroup->addChild(pat2);
		//}

		//osg::PositionAttitudeTransform* pat3 = new osg::PositionAttitudeTransform;
		//{
		//	osg::Node* node2 = osgDB::readNodeFile("E:\\Shader\\cube.obj");
		//	pat3->setPosition(osg::Vec3(0, 0, 5));
		//	pat3->addChild(node2);
		//	modelGroup->addChild(pat3);
		//}
	}

	g_highlightSystem->addHighlight(modelGroup);
}

class Base;
class A;
class B;
class C;
class Visitor
{
 public:
	void apply(Base* a)
	{
		cout << "BBASE\n";
	}

	void apply(A* a)
	{
		cout << "AA\n";
	}

	void apply(B* a)
	{
		cout << "BB\n";
	}
};

class Base
{
public:

	virtual void accept(Visitor& v)
	{
		v.apply(this);
	}
};


class A : public Base
{
 public:

	//virtual void accept(Visitor& v)
	//{
	//	v.apply(this);
	//}
};

class B : public Base
{
 public:

	virtual void accept(Visitor& v)
	{
		v.apply(this);
	}
};

class C : public Base
{
 public:

	virtual void accept(Visitor& v)
	{
		v.apply(this);
	}
};


int main(int argc, char** argv)
{
	Base* p = new C;
	Visitor v;
	p->accept(v); // "BBASE"


	Base* p2 = new A;
	p2->accept(v); //  "BBASE"

	Base* p3 = new B;
	p3->accept(v);  //  "BB"

	getchar();
	osg::ArgumentParser arguments(&argc, argv);

	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	osgViewer::Viewer viewer(arguments);

	osg::Group* root = new osg::Group;
	viewer.setSceneData(root);
	viewer.addEventHandler(new PickHandler());

	g_highlightSystem = std::make_shared<HighlightSystem>(&viewer);
	

	add_event_handler(viewer);
	osg::setNotifyLevel(osg::NotifySeverity::WARN);

	return viewer.run();
}
