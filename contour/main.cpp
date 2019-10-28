// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/Switch>
#include <osg/io_utils>
#include "CollectPointsVisitor.h"

osg::Switch* g_root   = new osg::Switch;
osg::ref_ptr<osg::Group> g_scene = new osg::Group;
osg::Camera* g_camera = nullptr;
osg::ref_ptr<osg::Node> g_contour = nullptr;

//------------------------------------------------------------------------------------------

class UpdateLODUniform : public osg::Uniform::Callback
{
 public:
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(_lod);
		//cout << "select index " << _selected << endl;
	}

	int _lod;
};

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

osg::ref_ptr<UpdateLODUniform> u_updateLODUniform = new UpdateLODUniform;

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
		if (allPTs[i].x() == -1 && allPTs[i].y() == -1) //图元重启
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

	osg::StateSet* stateset = pGeometry->getOrCreateStateSet();
	stateset->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	stateset->setMode(GL_PRIMITIVE_RESTART, osg::StateAttribute::ON);
	stateset->setAttributeAndModes(new osg::PrimitiveRestartIndex(kLastIndex), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("CONTOUR");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/contour.vert"));
	if(bUseGeometry)
		program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/contour.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/contour.frag"));

	stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(verts);
	pGeometry->setVertexAttribArray(0, verts, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	//-----------uniform
	osg::Uniform* u_lod(new osg::Uniform("u_lod", 1));
	u_lod->setUpdateCallback(u_updateLODUniform);
	stateset->addUniform(u_lod);

	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	stateset->addUniform(u_MVP);

	return pGeometry.release();
}

osg::Node* create_lines(osgViewer::Viewer& view, int LOD)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3>		 PTs;

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 1e7; i++)
		{
			//if (i % LOD == 0)
			{
				PTs.push_back(osg::Vec3d(i * 10, 0, 0));

				if (i % 4 < 2)
					PTs.back() += osg::Vec3(0, 50, 0);

				PTs.back() += osg::Vec3(0, j * 150, 0);
			}
		}
		PTs.push_back(osg::Vec3(-1, -1, -1));  //这个点不会显示，但OSG计算包围盒的时候还是会考虑它
	}

	osg::Geometry* n = createLine2(PTs, osg::Vec4(1, 0, 0, 1), view.getCamera(), false);
	geode->addDrawable(n);

	return geode.release();
}

CollectPointsVisitor g_cpv;

class PickHandler : public osgGA::GUIEventHandler
{

 public:
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
				g_scene->removeChildren(0, g_scene->getNumChildren());
				osg::ref_ptr<osg::Geode> geode = new osg::Geode;
				osg::Geometry* n = createLine2(g_cpv.resultPts_, osg::Vec4(1, 0, 0, 1), view->getCamera(), false);
				n->setCullingActive(false);
				geode->addDrawable(n);
				g_scene->addChild(geode); break;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_B)
			{
				g_scene->removeChildren(0, g_scene->getNumChildren());
				osg::ref_ptr<osg::Geode> geode = new osg::Geode;
				osg::Geometry* n = createLine2(g_cpv.resultPts_, osg::Vec4(1, 0, 0, 1), view->getCamera(), true);
				n->setCullingActive(false);
				geode->addDrawable(n);
				g_scene->addChild(geode); break;
			}
		}
		}
		return false;
	}
};

struct ComputeBoundingSphereCallback : public osg::Node::ComputeBoundingSphereCallback
{
	ComputeBoundingSphereCallback() {}

	ComputeBoundingSphereCallback(const ComputeBoundingSphereCallback& org, const osg::CopyOp& copyop) :
		osg::Node::ComputeBoundingSphereCallback(org, copyop) {}

	META_Object(osg, ComputeBoundingSphereCallback);

	virtual osg::BoundingSphere computeBound(const osg::Node&) const 
	{ 
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

		osg::BoundingSphere bs;
		bs.expandBy(bb);
		return bs;
	}

};

int main()
{
	osgViewer::Viewer view;

	g_contour = osgDB::readNodeFile("morelines.shp");
	g_contour->accept(g_cpv);
	g_scene->setComputeBoundingSphereCallback(new ComputeBoundingSphereCallback);
	
	g_root->addChild(g_scene);
	view.setSceneData(g_root);
	add_event_handler(view);
	view.addEventHandler(new PickHandler);
	view.realize();
	g_camera = view.getCamera();
	g_camera->setComputeNearFarMode(osg::Camera::COMPUTE_NEAR_FAR_USING_PRIMITIVES);

	return view.run();
}
