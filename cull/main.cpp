// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"
#if 0

osg::Group* createScene()
{
	osg::ref_ptr<osg::Group> parent = new osg::Group;

	//地形
	osg::ref_ptr<osg::Box> box = new osg::Box(osg::Vec3(), 1000, 1000, 10);
	{
		osg::ref_ptr<osg::ShapeDrawable> plane = new osg::ShapeDrawable(box);
		plane->setColor(osg::Vec4(1, 0, 0, 1));
		parent->addChild(plane);

		osg::StateSet* ss = plane->getOrCreateStateSet();
		ss->setRenderBinDetails(10, "RenderBin");
	}

	//用于遮挡地形的区域
	osg::ref_ptr<osg::Box> box2 = new osg::Box(osg::Vec3(), 100, 100, 20);
	{
		osg::ref_ptr<osg::ShapeDrawable> plane = new osg::ShapeDrawable(box2);
		plane->setColor(osg::Vec4(0, 1, 0, 0));
		parent->addChild(plane);

		osg::StateSet* ss = plane->getOrCreateStateSet();
		ss->setRenderBinDetails(9, "RenderBin");
		ss->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		//ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_WRITEMASK, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);		
	}
	
	osg::Node* cow = osgDB::readNodeFile("cow.osg");
	{
		osg::MatrixTransform* tran = new osg::MatrixTransform;
		tran->setMatrix(osg::Matrix::translate(osg::Vec3(0, 0, 0)));
		tran->addChild(cow);
		parent->addChild(tran);
	}
	

	return parent.release();
}
#else

//pick
//-------------------------------------------------------------
osg::Group* ff()
{
	osg::Group* root = new osg::Group;

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geode->addDrawable(geometry);
	root->addChild(geode);

	osg::ref_ptr<osg::Vec3Array> vert = new osg::Vec3Array;
	geometry->setVertexArray(vert);
	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
	geometry->setColorArray(color, osg::Array::BIND_PER_VERTEX);

	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<float> urd(0, 1);

	for (int i = 0; i < 3; i++)
	{
		osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
		root->addChild(pat);
		pat->setPosition(osg::Vec3(50 * i, 0, 0));
		pat->addChild(osgDB::readNodeFile("cow.osg"));

		osg::Node* c = pat;
		osg::ComputeBoundsVisitor cbv;
		c->accept(cbv);
		osg::BoundingBox bb = cbv.getBoundingBox();

		if (!bb.valid()) continue;

		float xMin = bb.xMin(), xMax = bb.xMax();
		float yMin = bb.yMin(), yMax = bb.yMax();
		float zMin = bb.zMin(), zMax = bb.zMax();

		OSG_WARN << "DELTA: " << (xMax - xMin) << "\t" << (yMax - yMin) << "\t" << (zMax - zMin) << "\n";

		float r = urd(eng), g = urd(eng), b = urd(eng);
		for (int i = 0; i < 16; i++)
		{
			color->push_back(osg::Vec4(r, g, b, 1));
		}

		//front
		vert->push_back(osg::Vec3(xMin, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMin, yMin, zMax));

		//back
		vert->push_back(osg::Vec3(xMin, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));

		//left
		vert->push_back(osg::Vec3(xMin, yMin, zMin));
		vert->push_back(osg::Vec3(xMin, yMax, zMin));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMin, zMax));

		//right
		vert->push_back(osg::Vec3(xMax, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));

		//top
		vert->push_back(osg::Vec3(xMin, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));

		//bottom
		vert->push_back(osg::Vec3(xMin, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));

		pat->setNodeMask(0);
	}

	geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vert->size()));

	osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	geode->getOrCreateStateSet()->setAttributeAndModes(model);

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
			osg::ref_ptr<osgUtil::IntersectorGroup> pickGroup = new osgUtil::IntersectorGroup;
			{
				std::default_random_engine			   generator(time(NULL));
				std::uniform_real_distribution<double> distribution(0., 1.);

				clock_t t = clock();
				//for (int i = 0; i < 10; i++)
				//{
				double x = distribution(generator);
				double y = distribution(generator);

				double mx = ea.getXnormalized(); //range[-1, 1]
				double my = ea.getYnormalized();
				double w = 0.05;
				double h = 0.05;
				osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::PROJECTION, ea.getXnormalized(), ea.getYnormalized());
				pickGroup->addIntersector(picker);
				//}

				osgUtil::IntersectionVisitor iv(pickGroup);
				view_->getCamera()->accept(iv);
				//OSG_WARN << "TIME: " << (clock() - t) << std::endl;

				if (picker->containsIntersections())
				{
					osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();
					osg::notify(osg::NOTICE) << "Picked " << intersection.localIntersectionPoint << std::endl
						<< "  primitive index " << intersection.primitiveIndex
						<< std::endl;

					osg::NodePath& nodePath = intersection.nodePath;
				}
			}

		}

		return false;
	}

	osgViewer::Viewer* view_;
};

#endif

int main()
{
	osgViewer::Viewer view;
	osg::Group*		  root = new osg::Group;
	root->addChild(ff());

	view.setSceneData(root);

	EventCallback* c = new EventCallback;
	c->view_ = &view;
	view.addEventHandler(c);

	add_event_handler(view);

	osg::Camera* camera = view.getCamera();
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
