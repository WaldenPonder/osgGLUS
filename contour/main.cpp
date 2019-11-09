// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/Switch>
#include <osg/io_utils>
#include "CollectPointsVisitor.h"
#include "Manager.h"
#include <osg/MatrixTransform>

#include <osgSim/Impostor>
#include <osgSim/InsertImpostorsVisitor>
#include <osg/ComputeBoundsVisitor>
#include <random>
#define NM_NO_SHOW 1
#define NM_NO_PICK 2

class CustomGroup : public osg::Group
{
 public:
	virtual void traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
		}

		if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
		// || nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
			return;
		}

		return osg::Group::traverse(nv);
	}
};

osg::ref_ptr<osg::Group> g_root   = new osg::Group;
osg::ref_ptr<osg::Group> g_scene  = new osg::Group;
osg::Camera*			 g_camera = nullptr;

CollectPointsVisitor g_cpv;

struct ComputeBoundingSphereCallback : public osg::Node::ComputeBoundingSphereCallback
{
	ComputeBoundingSphereCallback() {}

	ComputeBoundingSphereCallback(const ComputeBoundingSphereCallback& org, const osg::CopyOp& copyop) : osg::Node::ComputeBoundingSphereCallback(org, copyop) {}

	META_Object(osg, ComputeBoundingSphereCallback);

	virtual osg::BoundingSphere computeBound(const osg::Node&) const
	{
		if (bCalcu)
			return bs_;

		bCalcu = true;
		osg::BoundingBox bb;
		for (auto& p : g_cpv.resultPts_)
		{
			if (p.x() == -1 && p.y() == -1)
			{
				continue;
			}
			else
			{
				bb.expandBy(p);
			}
		}
		bs_ = osg::BoundingSphere();
		bs_.expandBy(bb);

		return bs_;
	}
	mutable bool				bCalcu = false;
	mutable osg::BoundingSphere bs_;
};

class PickHandler : public osgGA::GUIEventHandler
{

 public:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		viewer->getEventVisitor()->setTraversalMask(~NM_NO_SHOW);

		switch (ea.getEventType())
		{
		case (osgGA::GUIEventAdapter::FRAME):
		{
			break;
		}
		case (osgGA::GUIEventAdapter::RELEASE):
		{
			clock_t t = clock();
			break;
			osgUtil::PolytopeIntersector* picker;

			// use window coordinates
			// remap the mouse x,y into viewport coordinates.
			osg::Viewport* viewport = viewer->getCamera()->getViewport();
			double		   mx		= viewport->x() + ( int )(( double )viewport->width() * (ea.getXnormalized() * 0.5 + 0.5));
			double		   my		= viewport->y() + ( int )(( double )viewport->height() * (ea.getYnormalized() * 0.5 + 0.5));

			// half width, height.
			double w = 5.0f;
			double h = 5.0f;
			picker   = new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, mx - w, my - h, mx + w, my + h);
			osgUtil::IntersectionVisitor iv(picker);
			iv.setTraversalMask(~NM_NO_PICK);
			viewer->getCamera()->accept(iv);

			if (picker->containsIntersections())
			{
				osgUtil::PolytopeIntersector::Intersection intersection = picker->getFirstIntersection();
				osg::NodePath&							   nodePath		= intersection.nodePath;
				osg::Node*								   node			= (nodePath.size() >= 1) ? nodePath[nodePath.size() - 1] : 0;

				if (node) std::cout << "  Hits " << node->className() << " nodePath size " << nodePath.size() << std::endl;
			}

			cout << (clock() - t) << endl;
			break;
		}
		case (osgGA::GUIEventAdapter::KEYDOWN):
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_B)
			{

			}
		}
		}
		return false;
	}
};

class CollectGeometryVisitor : public osg::NodeVisitor
{
 public:
	CollectGeometryVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
	{
	}

	virtual void apply(osg::Geometry& geo)
	{
		osg::Geode* g = new osg::Geode;
		g->addChild(&geo);
		geos_.push_back(g);
	}

 public:
	std::vector<osg::ref_ptr<osg::Node>> geos_;
};

int main()
{
	osgViewer::Viewer view;

	Manager manage(g_root, 20, 20);

	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<double> rand_color(.3, 1.);

#if 1

	const float SIZE = 100;
	for (float i = 1; i < 1000; i++)
	{		
		for (float j = 1; j < 1000; j++)
		{
			std::uniform_int_distribution<int> rand_x(SIZE * i, SIZE * i + SIZE);
			std::uniform_int_distribution<int> rand_y(SIZE * j, SIZE * j + SIZE);

			vector<osg::Vec3d> pts;
			for (int k = 0; k < 20; k++)
			{
				int x = rand_x(eng);
				int y = rand_y(eng);

				pts.push_back(osg::Vec3(x, y, 0));
			}
			
			osg::Geometry* geom = createLine(pts, osg::Vec4(rand_color(eng), rand_color(eng), rand_color(eng), 1), osg::PrimitiveSet::LINE_STRIP);
			osg::Geode* geode = new osg::Geode;
			geode->addChild(geom);
			manage.nodes_.push_back(geode);
		}
	}

#else
	osg::ref_ptr<osg::Node> g_contour = osgDB::readNodeFile("E:\\FileRecv\\boston_buildings_utm19.shp");
	CollectGeometryVisitor cgv;
	g_contour->accept(cgv);
	
	for (int i = 0; i < 10; i++)
	{
		osg::MatrixTransform* tran = new osg::MatrixTransform;
		tran->setMatrix(osg::Matrix::translate(g_contour->getBound().center() * 10.f));
		manage.nodes_.push_back(tran);
	}
	manage.nodes_ = cgv.geos_;

#endif

	manage.build();

	view.setSceneData(g_root);
	add_event_handler(view);
	view.addEventHandler(new PickHandler);
	view.realize();
	g_camera = view.getCamera();
	//g_camera->setCullMask(~NM_NO_SHOW);
	//g_camera->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
	return view.run();
}
