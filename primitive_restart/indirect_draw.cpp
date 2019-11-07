// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <osg/PrimitiveSetIndirect>

osg::ref_ptr<osg::DefaultIndirectCommandDrawArrays> g_indirectCommands = new osg::DefaultIndirectCommandDrawArrays;

vector<int> g_index;

osg::Geometry* createLine3(const std::vector<osg::Vec3d>& allPTs, osg::Vec4 color, osg::Camera* camera)
{
	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		verts->push_back(allPTs[i]);
	}

	pGeometry->setUseVertexBufferObjects(true); //不启用VBO的话，图元重启没效果
	pGeometry->setUseDisplayList(false);

	osg::StateSet* ss = pGeometry->getOrCreateStateSet();
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

	pGeometry->setVertexArray(verts);
	pGeometry->setVertexAttribArray(0, verts, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	pGeometry->setDataVariance(osg::Object::STATIC);

	osg::Vec4Array* colours = new osg::Vec4Array(1);
	pGeometry->setColorArray(colours, osg::Array::BIND_OVERALL);
	(*colours)[0].set(1.0f, .0f, .0f, 1.0f);
	
	//--------------------------------------------------	
	g_indirectCommands->getBufferObject()->setUsage(GL_STATIC_DRAW);

	osg::MultiDrawArraysIndirect *ipr = new osg::MultiDrawArraysIndirect(GL_LINE_STRIP);
	ipr->setIndirectCommandArray(g_indirectCommands.get());
	pGeometry->addPrimitiveSet(ipr);
	   
	return pGeometry.release();
}

osg::Node* create_lines(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3d>		 PTs;

	int COUNT = 0;
	for (int k = 0; k < 10000; k++)
	{
		float z = 0;
		for (int j = 0; j < 100; j++)
		{
			int preIndex = PTs.size();
			for (int i = 0; i < 10; i++)
			{
				PTs.push_back(osg::Vec3(i * 10, j * 10, 50 * k));
			}
			
			osg::DrawArraysIndirectCommand cmd;
			cmd.first = preIndex;
			cmd.count = PTs.size() - preIndex;
			cmd.baseInstance = 0;
			cmd.instanceCount = 1;
			g_indirectCommands->push_back(cmd);
			//PTs.push_back(PTs.back());
			COUNT++;
			g_index.push_back(PTs.size() - COUNT);
			//cout << "SIZE " << g_index.back() << endl;
		}
		
		for (int j = 0; j < 100; j++)
		{
			int preIndex = PTs.size();
			for (int i = 0; i < 10; i++)
			{
				PTs.push_back(osg::Vec3(i * 10, j * 10, 100 + 100 * k));
			}

			osg::DrawArraysIndirectCommand cmd;
			cmd.first = preIndex;
			cmd.count = PTs.size() - preIndex;
			cmd.baseInstance = 0;
			cmd.instanceCount = 1;
			g_indirectCommands->push_back(cmd);
			//PTs.push_back(PTs.back());
			COUNT++;
			g_index.push_back(PTs.size() - COUNT);
			//cout << "SIZE " << g_index.back() << endl;
		}
	}

	osg::Geometry* n = createLine3(PTs, osg::Vec4(1, 0, 0, 1), view.getCamera());
	geode->addDrawable(n);

	return geode.release();
}

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
public:

	PickHandler() :
		_mx(0.0), _my(0.0),
		_usePolytopeIntersector(true),
		_useWindowCoordinates(false),
		_precisionHint(osgUtil::Intersector::USE_DOUBLE_CALCULATIONS),
		_primitiveMask(osgUtil::PolytopeIntersector::ALL_PRIMITIVES) {}

	~PickHandler() {}

	void setPrimitiveMask(unsigned int primitiveMask) { _primitiveMask = primitiveMask; }
	
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if (!viewer) return false;

		switch (ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::KEYUP):
		{ 
			if (ea.getKey() == 'p')
			{
				_usePolytopeIntersector = !_usePolytopeIntersector;
				if (_usePolytopeIntersector)
				{
					osg::notify(osg::NOTICE) << "Using PolytopeIntersector" << std::endl;
				}
				else {
					osg::notify(osg::NOTICE) << "Using LineSegmentIntersector" << std::endl;
				}
			}
			else if (ea.getKey() == 'c')
			{
				_useWindowCoordinates = !_useWindowCoordinates;
				if (_useWindowCoordinates)
				{
					osg::notify(osg::NOTICE) << "Using window coordinates for picking" << std::endl;
				}
				else {
					osg::notify(osg::NOTICE) << "Using projection coordiates for picking" << std::endl;
				}
			}

			return false;
		}
		case(osgGA::GUIEventAdapter::PUSH):
		case(osgGA::GUIEventAdapter::MOVE):
		{
			_mx = ea.getX();
			_my = ea.getY();
			return false;
		}
		case(osgGA::GUIEventAdapter::RELEASE):
		{
			if (_mx == ea.getX() && _my == ea.getY())
			{
				// only do a pick if the mouse hasn't moved
				pick(ea, viewer);
			}
			return true;
		}

		default:
			return false;
		}
	}

	void pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
	{
		osg::Node* scene = viewer->getSceneData();
		if (!scene) return;

		osg::notify(osg::NOTICE) << std::endl;

		osg::Node* node = 0;
		osg::Group* parent = 0;
		
		if (_usePolytopeIntersector)
		{
			osgUtil::PolytopeIntersector* picker;
			if (0)
			{
				// use window coordinates
				// remap the mouse x,y into viewport coordinates.
				osg::Viewport* viewport = viewer->getCamera()->getViewport();
				double mx = viewport->x() + (int)((double)viewport->width()*(ea.getXnormalized()*0.5 + 0.5));
				double my = viewport->y() + (int)((double)viewport->height()*(ea.getYnormalized()*0.5 + 0.5));

				// half width, height.
				double w = 5.0f;
				double h = 5.0f;
				picker = new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, mx - w, my - h, mx + w, my + h);
			}
			else {
				double mx = ea.getXnormalized();
				double my = ea.getYnormalized();
				double w = 0.05;
				double h = 0.05;
				picker = new osgUtil::PolytopeIntersector(osgUtil::Intersector::PROJECTION, mx - w, my - h, mx + w, my + h);
			}

			picker->setPrecisionHint(_precisionHint);
			picker->setPrimitiveMask(_primitiveMask);

			osgUtil::IntersectionVisitor iv(picker);

			osg::ElapsedTime elapsedTime;

			viewer->getCamera()->accept(iv);

			//OSG_NOTICE << "PoltyopeIntersector traversal took " << elapsedTime.elapsedTime_m() << "ms" << std::endl;

			if (picker->containsIntersections())
			{
				for (osgUtil::PolytopeIntersector::Intersection intersection : picker->getIntersections())
				{
					cout	<< "pre_index " << intersection.primitiveIndex
						<< std::endl;

					osg::NodePath& nodePath = intersection.nodePath;
					node = (nodePath.size() >= 1) ? nodePath[nodePath.size() - 1] : 0;
					parent = (nodePath.size() >= 2) ? dynamic_cast<osg::Group*>(nodePath[nodePath.size() - 2]) : 0;

					for (int i = 0; i < g_index.size(); i++)
					{
						if (intersection.primitiveIndex + 2 < g_index[i])
						{
							//cout << "i " << i << endl; break;
						}
					}

					return;
				}
				
				//if (node) std::cout << "  Hits " << node->className() << " nodePath size " << nodePath.size() << std::endl;
			}

		}
	
		// now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
	}


	void setPrecisionHint(osgUtil::Intersector::PrecisionHint hint) { _precisionHint = hint; }

protected:

	float _mx, _my;
	bool _usePolytopeIntersector;
	bool _useWindowCoordinates;
	osgUtil::Intersector::PrecisionHint _precisionHint;
	unsigned int _primitiveMask;

};

int main()
{
	osgViewer::Viewer view;

	osg::Group* root = new osg::Group;
	//root->addChild(osgDB::readNodeFile("cow.osg"));
	root->addChild(create_lines(view));

	osg::ref_ptr<osg::KdTreeBuilder> kdBuild = new osg::KdTreeBuilder;
	root->accept(*kdBuild);

	view.setSceneData(root);
	add_event_handler(view);
	view.addEventHandler(new PickHandler);
	//osg::DisplaySettings::instance()->setGLContextVersion("4.3");
	//osg::DisplaySettings::instance()->setShaderHint(osg::DisplaySettings::SHADER_GL3);
	osg::setNotifyLevel(osg::NotifySeverity::WARN);
	view.realize();
	return view.run();
}

