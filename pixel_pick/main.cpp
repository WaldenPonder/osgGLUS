// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>

//------------------------------------------------------------------------------------------
class MVPCallback : public osg::Uniform::Callback
{
 public:
	MVPCallback(osg::Camera* camera) : mCamera(camera)
	{
	}
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		osg::Matrix modelView = mCamera->getViewMatrix();
		osg::Matrix projectM  = mCamera->getProjectionMatrix();
		uniform->set(modelView * projectM);
	}

 private:
	osg::Camera* mCamera;
};

//https://blog.csdn.net/qq_16123279/article/details/82463266

osg::Geometry* createLine2(const std::vector<osg::Vec3d>& allPTs, const std::vector<osg::Vec3d>& colors, osg::Camera* camera)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;

	//传递给shader
	osg::ref_ptr<osg::Vec3Array> a_color = new osg::Vec3Array;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> a_pos = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		a_pos->push_back(allPTs[i]);
	}

	const int							   kLastIndex = allPTs.size() - 1;
	osg::ref_ptr<osg::ElementBufferObject> ebo		  = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices	= new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_STRIP);

	std::default_random_engine			  eng(time(NULL));
	std::uniform_real_distribution<float> rand(.3, 1.);

	int count = 0;
	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		if (allPTs[i].x() == -1 && allPTs[i].y() == -1)
		{
			indices->push_back(kLastIndex);
		}
		else
		{
			indices->push_back(i);
		}
		count++;

		a_color->push_back(colors[i]);
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true);  //不启用VBO的话，图元重启没效果

	osg::StateSet* ss = pGeometry->getOrCreateStateSet();
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	ss->setMode(GL_PRIMITIVE_RESTART, osg::StateAttribute::ON);
	ss->setAttributeAndModes(new osg::PrimitiveRestartIndex(kLastIndex), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("LINESTRIPE");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_stripe2.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_stripe2.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(a_pos);
	pGeometry->setVertexAttribArray(0, a_pos, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	pGeometry->setVertexAttribArray(1, a_color, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_color", 1);

	//-----------uniform
	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);

	return pGeometry.release();
}

osg::Node* create_lines(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3d>		 PTs, COLORs;

	std::default_random_engine			  eng(time(NULL));
	std::uniform_real_distribution<float> rand(.3, 1.);

	const float SIZE = 100;

	vector<osg::Vec3d> colors;
	colors.push_back(osg::Vec3(1, 0, 0));
	colors.push_back(osg::Vec3(0, 1, 0));
	colors.push_back(osg::Vec3(0, 0, 1));
	colors.push_back(osg::Vec3(1, 1, 0));
	colors.push_back(osg::Vec3(1, 1, 1));

	colors.push_back(osg::Vec3(1, 0, 0));
	colors.push_back(osg::Vec3(0, 1, 0));
	colors.push_back(osg::Vec3(0, 0, 1));
	colors.push_back(osg::Vec3(1, 1, 0));
	colors.push_back(osg::Vec3(1, 0, 1));
	colors.push_back(osg::Vec3(1, 1, 1));
	colors.push_back(osg::Vec3(0, 0, 1));

	PTs.push_back(osg::Vec3(0, 0, 0));
	PTs.push_back(osg::Vec3(100, 0, 0));
	PTs.push_back(osg::Vec3(50, 100, 0));
	PTs.push_back(osg::Vec3(0, 0, 0));
	PTs.push_back(osg::Vec3(-1, -1, -1));

	osg::Geometry* n = createLine2(PTs, colors, view.getCamera());
	n->setName("LINE1");
	n->setUserValue("ID", osg::Vec4(0, 1, 0, 1));
	geode->addDrawable(n);



	//-------------------------------------------------
	PTs.clear();
	PTs.push_back(osg::Vec3(200, 0, 0));
	PTs.push_back(osg::Vec3(300, 0, 0));
	PTs.push_back(osg::Vec3(300, 100, 0));
	PTs.push_back(osg::Vec3(200, 100, 0));
	PTs.push_back(osg::Vec3(200, 0, 0));
	PTs.push_back(osg::Vec3(-1, -1, -1));

	osg::Geometry* n2 = createLine2(PTs, colors, view.getCamera());
	n2->setName("LINE2");
	n2->setUserValue("ID", osg::Vec4(1, 0, 0, 1));
	geode->addDrawable(n2);

	//uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
	//uniform->set(osg::Vec4(1, 0, 0, 1));
	//n2->getOrCreateStateSet()->addUniform(uniform);

	return geode.release();
}

class UniformVisitor : public osg::NodeVisitor
{
public:
	UniformVisitor(osg::Camera* camera) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { camera_ = camera; }

	virtual void apply(osg::Geometry& gem) override
	{
		osg::StateSet* ss = camera_->getOrCreateStateSet();
		osg::Vec4 color;
		gem.getUserValue("ID", color);
		ss->getUniform("u_color")->set(color);
	}

	osg::Camera* camera_;
};

// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler
{
 public:
	PickHandler() : _mx(0.0), _my(0.0),
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
		case (osgGA::GUIEventAdapter::KEYUP):
		{
			if (ea.getKey() == 'p')
			{
				_usePolytopeIntersector = !_usePolytopeIntersector;
				if (_usePolytopeIntersector)
				{
					osg::notify(osg::NOTICE) << "Using PolytopeIntersector" << std::endl;
				}
				else
				{
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
				else
				{
					osg::notify(osg::NOTICE) << "Using projection coordiates for picking" << std::endl;
				}
			}

			return false;
		}
		case (osgGA::GUIEventAdapter::PUSH):
		case (osgGA::GUIEventAdapter::MOVE):
		{
			_mx = ea.getX();
			_my = ea.getY();
			return false;
		}
		case (osgGA::GUIEventAdapter::RELEASE):
		{
			if (_mx == ea.getX() && _my == ea.getY())
			{
				// only do a pick if the mouse hasn't moved
				//pick(ea, viewer);
				addSlave(ea, viewer);
			}
			return true;
		}

		default:
			return false;
		}
	}

	void addSlave(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
	{
		static bool b = false;
		if (b) return;
		b = true;

		osg::ref_ptr<osg::Camera> camera = new osg::Camera;
		camera->setViewport(0, 0, 200, 200);
		camera->setGraphicsContext(viewer->getCamera()->getGraphicsContext());
		viewer->addSlave(camera);

		osg::StateSet* ss = camera->getOrCreateStateSet();
		osg::Program* program = new osg::Program;
		program->setName("PIXEL_PICK");
		program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/pixel_pick.vert"));
		program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/pixel_pick.frag"));
			   
		ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		//osg::ref_ptr<UniformVisitor> uv = new UniformVisitor(camera);
		//camera->accept(*uv);

		osg::Uniform* uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
		uniform->set(osg::Vec4(0, 1, 0, 1));
		ss->addUniform(uniform);
	}

	void pick(const osgGA::GUIEventAdapter& ea, osgViewer::Viewer* viewer)
	{
		osg::Node* scene = viewer->getSceneData();
		if (!scene) return;

		osg::notify(osg::NOTICE) << std::endl;

		osg::Node*  node   = 0;
		osg::Group* parent = 0;

		if (_usePolytopeIntersector)
		{
			osgUtil::PolytopeIntersector* picker;
			if (0)
			{
				// use window coordinates
				// remap the mouse x,y into viewport coordinates.
				osg::Viewport* viewport = viewer->getCamera()->getViewport();
				double		   mx		= viewport->x() + ( int )(( double )viewport->width() * (ea.getXnormalized() * 0.5 + 0.5));
				double		   my		= viewport->y() + ( int )(( double )viewport->height() * (ea.getYnormalized() * 0.5 + 0.5));

				// half width, height.
				double w = 5.0f;
				double h = 5.0f;
				picker   = new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, mx - w, my - h, mx + w, my + h);
			}
			else
			{
				double mx = ea.getXnormalized();
				double my = ea.getYnormalized();
				double w  = 0.05;
				double h  = 0.05;
				picker	= new osgUtil::PolytopeIntersector(osgUtil::Intersector::PROJECTION, mx - w, my - h, mx + w, my + h);
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
					cout << "pre_index " << intersection.primitiveIndex
						 << std::endl;

					osg::NodePath& nodePath = intersection.nodePath;
					node					= (nodePath.size() >= 1) ? nodePath[nodePath.size() - 1] : 0;
					parent					= (nodePath.size() >= 2) ? dynamic_cast<osg::Group*>(nodePath[nodePath.size() - 2]) : 0;
				}

				//if (node) std::cout << "  Hits " << node->className() << " nodePath size " << nodePath.size() << std::endl;
			}
		}

		// now we try to decorate the hit node by the osgFX::Scribe to show that its been "picked"
	}

	void setPrecisionHint(osgUtil::Intersector::PrecisionHint hint) { _precisionHint = hint; }

 protected:
	float								_mx, _my;
	bool								_usePolytopeIntersector;
	bool								_useWindowCoordinates;
	osgUtil::Intersector::PrecisionHint _precisionHint;
	unsigned int						_primitiveMask;
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
