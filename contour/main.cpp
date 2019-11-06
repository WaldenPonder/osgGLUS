// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/Switch>
#include <osg/io_utils>
#include "CollectPointsVisitor.h"

#define NM_NO_SHOW 1
#define NM_NO_PICK 2


class CustomGroup : public osg::Group
{
public:
	virtual void traverse(osg::NodeVisitor& nv)
	{
		if (nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
			cout << "n" << endl;
		}

		if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
			// || nv.getVisitorType() == osg::NodeVisitor::NODE_VISITOR)
		{
			return;
		}

		return osg::Group::traverse(nv);
	}
};

osg::ref_ptr<osg::Group>		 g_root   = new osg::Group;
osg::ref_ptr<osg::Group> g_scene  = new osg::Group;
osg::Camera*			 g_camera = nullptr;
osg::ref_ptr<osg::Node>  g_contour;

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

osg::Geometry* createLine2(const std::vector<osg::Vec3>& allPTs, osg::Vec4 color, osg::Camera* camera, bool bUseGeometry)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;
	cout << "PTS SIZE " << allPTs.size() << endl;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;
	for (int i = 0; i < allPTs.size(); i++)
	{
		verts->push_back(allPTs[i]);
	}

	const int							   kLastIndex = allPTs.size() - 1;
	osg::ref_ptr<osg::ElementBufferObject> ebo		  = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices	= new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);

	int count = 0;
	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		if (allPTs[i].x() == -1 && allPTs[i].y() == -1)  //图元重启
		{
			indices->push_back(kLastIndex);
		}
		else
		{
			indices->push_back(i);
		}
		count++;
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true);  //不启用VBO的话，图元重启没效果
	pGeometry->setDataVariance(osg::Object::STATIC);
	pGeometry->setCullingActive(false);
	pGeometry->setVertexArray(verts);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(0, 1, 1, 1));
	colors->setBinding(osg::Array::BIND_OVERALL);
	pGeometry->setColorArray(colors);

	osg::StateSet* stateset = pGeometry->getOrCreateStateSet();
	stateset->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	stateset->setMode(GL_PRIMITIVE_RESTART, osg::StateAttribute::ON);
	stateset->setAttributeAndModes(new osg::PrimitiveRestartIndex(kLastIndex), osg::StateAttribute::ON);

	return pGeometry.release();
}

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
				g_scene->removeChildren(0, g_scene->getNumChildren());
				osg::ref_ptr<osg::Geode> geode = new osg::Geode;
				osg::Geometry*			 n	 = createLine2(g_cpv.resultPts_, osg::Vec4(1, 0, 0, 1), viewer->getCamera(), true);
				geode->setCullingActive(false);
				geode->addDrawable(n);
				geode->setNodeMask(NM_NO_PICK);
				g_scene->addChild(geode);
				geode->setComputeBoundingSphereCallback(new ComputeBoundingSphereCallback);
				break;
			}
		}
		}
		return false;
	}
};


int main()
{
	vector<osg::Vec3> pts;
	set<osg::Vec3> pts2;
	//pts.resize(1e6);

	clock_t t = clock();
	for (int i = 0; i < 1e7; i++)
	{
		pts.push_back(osg::Vec3(i, i, i));
		//pts[i] = ();
	}

	cout <<  "AAA   " << (clock() - t) << "\t" << pts.size() << std::endl;
	t = clock();

	for (auto& p : pts)
	{
		pts2.insert(p);
	}

	cout << "BBB   " << (clock() - t) << "\t" << pts2.size() << std::endl;

	getchar();

	osgViewer::Viewer view;

	CustomGroup* grp = new CustomGroup;
	g_contour = osgDB::readNodeFile("E:\\FileRecv\\xx8.shp");
	grp->addChild(g_contour);
	//g_contour = osgDB::readNodeFile("E:\\FileRecv\\morelines.shp");

	g_contour->accept(g_cpv);
	//g_contour->setNodeMask(NM_NO_SHOW);
	g_root->addChild(grp);

	osg::ref_ptr<osg::KdTreeBuilder> kdBuild = new osg::KdTreeBuilder;
	g_contour->accept(*kdBuild);

	g_root->addChild(g_scene);
	view.setSceneData(g_root);
	add_event_handler(view);
	view.addEventHandler(new PickHandler);
	view.realize();
	g_camera = view.getCamera();
	g_camera->setCullMask(~NM_NO_SHOW);

	return view.run();
}
