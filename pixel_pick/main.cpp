// custom_drawable.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>

#define  WINDOWSIZE 8

bool g_bMouseRelease = false;
osg::ref_ptr<osg::Camera> g_pickCamera = new osg::Camera;

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


struct MyCameraPostDrawCallback : public osg::Camera::DrawCallback
{
	MyCameraPostDrawCallback(osg::Image* image) :
		_image(image)
	{
	}

	virtual void operator () (const osg::Camera& /*camera*/) const
	{
		if (!g_bMouseRelease) return;
		g_bMouseRelease = false;
		cout << "-------------------------\n\n";

		if (_image && _image->getPixelFormat() == GL_RGBA && _image->getDataType() == GL_UNSIGNED_BYTE)
		{
			// we'll pick out the center 1/2 of the whole image,
			int column_start = _image->s() / 4;
			int column_end = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end = 3 * row_start;

			string str;
			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				unsigned char* data = _image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					unsigned char r = *data; data++;
					unsigned char g = *data; data++;
					unsigned char b = *data; data++;
					unsigned char a = *data; data++;

					if (r + g + b + a > 0)
					{
						cout << std::to_string(r) << "\t" << std::to_string(g) << "\t" << std::to_string(b) << "\t" << std::to_string(a) << endl;
					}
				}
			}

			// dirty the image (increments the modified count) so that any textures
			// using the image can be informed that they need to update.
			_image->dirty();
		}
		else if (_image && _image->getPixelFormat() == GL_RGBA && _image->getDataType() == GL_FLOAT)
		{
			// we'll pick out the center 1/2 of the whole image,
			int column_start = _image->s() / 4;
			int column_end = 3 * column_start;

			int row_start = _image->t() / 4;
			int row_end = 3 * row_start;
			string str;
			// and then invert these pixels
			for (int r = row_start; r < row_end; ++r)
			{
				float* data = (float*)_image->data(column_start, r);
				for (int c = column_start; c < column_end; ++c)
				{
					float r = *data; data++;
					float g = *data; data++;
					float b = *data; data++;
					float a = *data; data++;

					if (r + g + b + a > 0)
					{
						cout << std::to_string(r) << "\t" << std::to_string(g) << "\t" << std::to_string(b) << "\t" << std::to_string(a) << endl;
					}
				}
			}
			// dirty the image (increments the modified count) so that any textures
			// using the image can be informed that they need to update.
			_image->dirty();		
		}

	}

	osg::Image* _image;
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

	//-----------------------------------------------uniform
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
	geode->addDrawable(n);

	osg::Uniform* uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
	uniform->set(osg::Vec4(1, 0, 0, 1.));
	n->getOrCreateStateSet()->addUniform(uniform);

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
	geode->addDrawable(n2);

	uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
	uniform->set(osg::Vec4(0, 1, 0, 1.));
	n2->getOrCreateStateSet()->addUniform(uniform);

	return geode.release();
}


void createHudCamera(osgViewer::Viewer* viewer, osg::Image* image)
{
	osg::Geode*	geode_quat = nullptr;
	osg::Geometry* screenQuat = nullptr;
	int w_ = 200, h_ = 200;
	osg::Camera* hud_camera_ = new osg::Camera;
	viewer->getSceneData()->asGroup()->addChild(hud_camera_);
	
	screenQuat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	{
		hud_camera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hud_camera_->setProjectionMatrixAsOrtho2D(0, w_, 0, h_);
		hud_camera_->setViewMatrix(osg::Matrix::identity());
		hud_camera_->setViewport(0, 0, w_, h_);
		hud_camera_->setRenderOrder(osg::Camera::POST_RENDER);
		hud_camera_->setClearMask(GL_DEPTH_BUFFER_BIT);
		hud_camera_->getOrCreateStateSet()->setMode(GL_LIGHTING,
			osg::StateAttribute::OFF);

		geode_quat = new osg::Geode;
		hud_camera_->addChild(geode_quat);
		geode_quat->addChild(screenQuat);
		geode_quat->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		geode_quat->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		screenQuat->setDataVariance(osg::Object::DYNAMIC);
		screenQuat->setSupportsDisplayList(false);

		osg::StateSet* ss = geode_quat->getOrCreateStateSet();
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		ss->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
	}
}

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

	void screenRay(float x, float y, osg::Camera* pCamera, osg::Vec3& start, osg::Vec3& end)
	{
		osg::Matrixd mtPVW = pCamera->getViewMatrix() *
			pCamera->getProjectionMatrix() *
			pCamera->getViewport()->computeWindowMatrix();

		osg::Matrixd mtInvPVW;
		mtInvPVW.invert(mtPVW);

		osg::Vec4d nearPosition(x, y, 0.0, 1.0);
		osg::Vec4d worldNearPosition = nearPosition * mtInvPVW;
		worldNearPosition /= worldNearPosition.w();

		osg::Vec4d farPosition(x, y, 1.0, 1.0);
		osg::Vec4d worldFarPosition = farPosition * mtInvPVW;
		worldFarPosition /= worldFarPosition.w();

		start.x() = worldNearPosition.x();
		start.y() = worldNearPosition.y();
		start.z() = worldNearPosition.z();

		end.x() = worldFarPosition.x();
		end.y() = worldFarPosition.y();
		end.z() = worldFarPosition.z();
	}

	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if (!viewer) return false;

		switch (ea.getEventType())
		{
		case (osgGA::GUIEventAdapter::KEYUP):
		{
			return false;
		}
		case (osgGA::GUIEventAdapter::PUSH):
		{
			_mx = ea.getX();
			_my = ea.getY();

			osg::Viewport* vp = viewer->getCamera()->getViewport();

			float wx = vp->width();
			float wy = vp->height();
			
			float x = ea.getXnormalized();
			float y = ea.getYnormalized();

			osg::Matrix mat = viewer->getCamera()->getViewMatrix() * 
				viewer->getCamera()->getProjectionMatrix();
		
			osg::Vec3 eye, center;
			
			eye = osg::Vec3(x, y, -1) * osg::Matrix::inverse(mat);
			center = osg::Vec3(x, y, 1) * osg::Matrix::inverse(mat);

			osg::Vec3 up = osg::Vec3(0, 1, 0) *viewer->getCamera()->getViewMatrix();
					   
			//g_pickCamera->setProjectionMatrix(viewer->getCamera()->getProjectionMatrix());
			g_pickCamera->setProjectionMatrix(osg::Matrix::ortho2D(0, WINDOWSIZE, 0, WINDOWSIZE));
			g_pickCamera->setViewMatrix(osg::Matrix::lookAt(eye, center, up));

			break;
		}
		case (osgGA::GUIEventAdapter::MOVE):
		{

			return false;
		}
		case (osgGA::GUIEventAdapter::RELEASE):
		{
			g_bMouseRelease = true;
			if (_mx == ea.getX() && _my == ea.getY())
			{
				// only do a pick if the mouse hasn't moved
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
				
		g_pickCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		g_pickCamera->setClearColor(osg::Vec4());
		g_pickCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		g_pickCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		g_pickCamera->setRenderOrder(osg::Camera::PRE_RENDER);
		g_pickCamera->setViewport(0, 0, WINDOWSIZE, WINDOWSIZE);
		g_pickCamera->setGraphicsContext(viewer->getCamera()->getGraphicsContext());
		g_pickCamera->setViewMatrix(viewer->getCamera()->getViewMatrix());
		g_pickCamera->setProjectionMatrix(viewer->getCamera()->getProjectionMatrix());
		g_pickCamera->addChild(viewer->getSceneData()->asGroup()->getChild(0));
		//viewer->addSlave(g_pickCamera);
		viewer->getSceneData()->asGroup()->addChild(g_pickCamera);

		osg::StateSet* ss	  = g_pickCamera->getOrCreateStateSet();
		osg::Program*  program = new osg::Program;
		program->setName("PIXEL_PICK");
		program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/pixel_pick.vert"));
		program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/pixel_pick.frag"));

		ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

		osg::Image* image = new osg::Image;		
		image->allocateImage(WINDOWSIZE, WINDOWSIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE);

		// attach the image so its copied on each frame.
		g_pickCamera->attach(osg::Camera::COLOR_BUFFER, image);

		g_pickCamera->setPostDrawCallback(new MyCameraPostDrawCallback(image));


		createHudCamera(viewer, image);

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
	root->addChild(create_lines(view));

	view.setSceneData(root);
	add_event_handler(view);
	view.addEventHandler(new PickHandler);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
