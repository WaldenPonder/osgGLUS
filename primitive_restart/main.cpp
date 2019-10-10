// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
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
	//pGeometry->setVertexArray(verts.get());
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
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, SOLUTION_DIR + "shader/line_stripe.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, SOLUTION_DIR + "shader/line_stripe.frag"));

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


osg::Node* create_lines(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3d>		 PTs;

	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < 100; i++)
		{
			PTs.push_back(osg::Vec3d(i * 10, 0, 0));

			if (i % 4 < 2)
				PTs.back() += osg::Vec3(0, 50, 0);

			PTs.back() += osg::Vec3(0, j * 150, 0);
		}
		PTs.push_back(osg::Vec3(-1, -1, -1));  //这个点不会显示，但OSG计算包围盒的时候还是会考虑它
	}

	osg::Geometry* n = createLine2(PTs, osg::Vec4(1, 0, 0, 1), view.getCamera());
	geode->addDrawable(n);

	return geode.release();
}

int main()
{
	osg::Node* n;
	
	osgViewer::Viewer view;

	osg::Group* root = new osg::Group;
	//root->addChild(osgDB::readNodeFile("cow.osg"));
	root->addChild(create_lines(view));

	view.setSceneData(root);
	add_event_handler(view);

	//osg::DisplaySettings::instance()->setGLContextVersion("4.3");
	//osg::DisplaySettings::instance()->setShaderHint(osg::DisplaySettings::SHADER_GL3);

	view.realize();
	return view.run();
}

