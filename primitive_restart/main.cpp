// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include "THPolytopeIntersector.h"
#include "THKdTree.h"

vector<int> g_index;
//------------------------------------------------------------------------------------------

class UpdateSelecteUniform : public osg::Uniform::Callback
{
public:

	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(_selected);
		//cout << "select index " << _selected << endl;
	}

	int _selected;
};


class MVPCallback : public osg::Uniform::Callback
{
public:
	MVPCallback(osg::Camera * camera) :mCamera(camera) {
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv) {
		osg::Matrix modelView = mCamera->getViewMatrix();
		osg::Matrix projectM = mCamera->getProjectionMatrix();
		uniform->set(modelView * projectM);
	}

private:
	osg::Camera * mCamera;
};

osg::ref_ptr<UpdateSelecteUniform> u_updateSelecteUniform = new UpdateSelecteUniform;

//https://blog.csdn.net/qq_16123279/article/details/82463266

osg::Geometry* createLine2(const std::vector<osg::Vec3d>& allPTs, osg::Vec4 color, osg::Camera* camera)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;

	//传递给shader
	osg::ref_ptr<osg::Vec2Array> a_index = new osg::Vec2Array;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;
	
	for (int i = 0; i < allPTs.size(); i++)
	{
		verts->push_back(allPTs[i]);
	}

	const int							   kLastIndex = allPTs.size() - 1;
	osg::ref_ptr<osg::ElementBufferObject> ebo = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);

	float start_index = 0, end_index = 0;
	int count = 0;
	int segment_count = 0;
	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		if (allPTs[i].x() == -1 && allPTs[i].y() == -1)
		{
			indices->push_back(kLastIndex);

			for (size_t j = a_index->size() - 1; count > 0; j--, count--)
			{
				a_index->at(j).y() = i;
			}

			start_index = i;
			segment_count++;
		}
		else
		{
			indices->push_back(i);
		}
		count++;
		a_index->push_back(osg::Vec2(start_index, 0));
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true); //不启用VBO的话，图元重启没效果

	osg::StateSet* stateset = pGeometry->getOrCreateStateSet();
	stateset->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	stateset->setMode(GL_PRIMITIVE_RESTART, osg::StateAttribute::ON);
	stateset->setAttributeAndModes(new osg::PrimitiveRestartIndex(kLastIndex), osg::StateAttribute::ON);


	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("LINESTRIPE");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_stripe.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_stripe.frag"));

	stateset->setAttributeAndModes(program, osg::StateAttribute::ON);


	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(verts);
	pGeometry->setVertexAttribArray(0, verts, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	pGeometry->setVertexAttribArray(1, a_index, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_index", 1);

	//-----------uniform
	osg::Uniform* u_selected_index(new osg::Uniform("u_selected_index", -1));
	u_selected_index->setUpdateCallback(u_updateSelecteUniform);
	stateset->addUniform(u_selected_index);

	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	stateset->addUniform(u_MVP);

	return pGeometry.release();
}


osg::Geometry* createLine3(osg::MultiDrawArrays* draw, const std::vector<osg::Vec3d>& allPTs, osg::Vec4 color, osg::Camera* camera)
{
	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		verts->push_back(allPTs[i]);
	}

	pGeometry->addPrimitiveSet(draw);
	pGeometry->setUseVertexBufferObjects(true); //不启用VBO的话，图元重启没效果

	osg::StateSet* stateset = pGeometry->getOrCreateStateSet();
	stateset->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);

	pGeometry->setVertexArray(verts);
	pGeometry->setVertexAttribArray(0, verts, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);

	osg::Vec4Array* colours = new osg::Vec4Array(1);
	pGeometry->setColorArray(colours, osg::Array::BIND_OVERALL);
	(*colours)[0].set(1.0f, .0f, .0f, 1.0f);

	return pGeometry.release();
}

osg::Node* create_lines(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3d>		 PTs;

	osg::ref_ptr<osg::MultiDrawArrays>	multidraw = new osg::MultiDrawArrays(osg::PrimitiveSet::LINE_STRIP);
	int COUNT = 0;
	for (int k = 0; k < 1; k++)
	{
		float z = 0;
		for (int j = 0; j < 10; j++)
		{
			int preIndex = PTs.size();
			for (int i = 0; i < 10; i++)
			{
				PTs.push_back(osg::Vec3(i * 10, j * 10, 50 * k));
			}
			PTs.push_back(PTs.back());
			multidraw->add(preIndex, PTs.size() - preIndex);
		
			g_index.push_back(PTs.size());
			cout << "SIZE " << g_index.back() << endl;
		}

		for (int j = 0; j < 10; j++)
		{
			int preIndex = PTs.size();
			for (int i = 0; i < 10; i++)
			{
				PTs.push_back(osg::Vec3(i * 10, j * 10, 100 * k + 100));
			}
			PTs.push_back(PTs.back());
			multidraw->add(preIndex, PTs.size() - preIndex);
			
			g_index.push_back(PTs.size() + COUNT);
			cout << "SIZE " << g_index.back() << endl;
		}
	}

	osg::Geometry* n = createLine3(multidraw, PTs, osg::Vec4(1, 0, 0, 1), view.getCamera());
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

			OSG_NOTICE << "PoltyopeIntersector traversal took " << elapsedTime.elapsedTime_m() << "ms" << std::endl;

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
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}

