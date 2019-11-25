#include "stdafx.h"
#include "HighlightSystem.h"
#include <osg/Callback>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Vec3>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include "ViewControlData.h"
#include "DB/DbSystemUtil.h"
#include "DB/DbCommand.h"
#include <osg/io_utils>
#include <osg/AutoTransform>
#include <osgText/Text>
#include <osg/LineWidth>
#include "TH3d/ShxText/ShxText.h"

#define PING_PONG_NUM 4
#define UNIFORM_CALLBACK
#define CLEAR_COLOR osg::Vec4(0, 0, 0, 1)
#define MULTI_SAMPLE 0, 0

float  TEXTURE_SIZE1 = 1920.f;
float  TEXTURE_SIZE2 = 1080.f;

struct HighlightSystemPrivateData
{
	osg::Camera*			 first_fbo_ed_	  = nullptr;	  //得到离屏的图(需要边缘检测)
	osg::Camera*			 first_fbo_no_ed_ = nullptr;	  //得到离屏的图（不需要边缘检测）
	osg::Camera*			 second_fbo_	  = nullptr;	  //提取out line
	osg::Camera*			 pingpong_fbo_[PING_PONG_NUM];	  //模糊
	osg::Camera*			 hud_camera_		  = nullptr;  //最终显示
	osg::Texture2D*			 first_texture_		  = nullptr;
	osg::Texture2D*			 first_texture_no_ed_ = nullptr;
	osg::Texture2D*			 second_texture_	  = nullptr;
	CViewControlData*		 pOSG_;
	osg::ref_ptr<osg::Group> highlightRoot_ = new osg::Group;
	float					 w_, h_;

	osg::Vec3 u_color			  = osg::Vec3(0, 0, 1);
	float	  u_gradientThreshold = 0.005;
	float	  u_alpha_factor	  = .7f;
	bool	  is_enable			  = true;
};

#ifdef UNIFORM_CALLBACK
//轮廓颜色
class OutLineColorUniform : public osg::Uniform::Callback
{
 public:
	OutLineColorUniform(osg::Vec3& color) : color_(color) {}
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(color_);
	}

	osg::Vec3& color_;
};

//边缘阀值
class GradientThresholdUniform : public osg::Uniform::Callback
{
 public:
	GradientThresholdUniform(float& g) : gradientThreshold_(g) {}
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(gradientThreshold_);
	}

	float& gradientThreshold_;
};

//轮廓透明度
class AlphaUniform : public osg::Uniform::Callback
{
 public:
	AlphaUniform(float& g) : a_(g) {}
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		uniform->set(a_);
	}

	float& a_;
};
#endif

struct HighlightSystem::Impl : public HighlightSystemPrivateData
{
	class FirstCameraPredrawCallback : public osg::Camera::DrawCallback
	{
	 public:
		osg::observer_ptr<osg::Camera> first_fbo;
		osg::observer_ptr<osg::Camera> main_camera;

		Impl* imp_;
		FirstCameraPredrawCallback(osg::Camera* first, osg::Camera* main, Impl* imp) : first_fbo(first), main_camera(main), imp_(imp) {}
		virtual void operator()(osg::RenderInfo& renderInfo) const
		{
			osg::Viewport* vp = main_camera->getViewport();

			if (first_fbo.get())
			{
				osg::ref_ptr<osg::RefMatrix> proMat = new osg::RefMatrix(first_fbo->getProjectionMatrix());
				renderInfo.getState()->applyProjectionMatrix(proMat);
				renderInfo.getState()->applyModelViewMatrix(first_fbo->getViewMatrix());
			}

			if (imp_->w_ != vp->width() || imp_->h_ != vp->height())
			{
				TEXTURE_SIZE1 = imp_->w_ = vp->width();
				TEXTURE_SIZE2 = imp_->h_ = vp->height();
				
				imp_->pOSG_->addCommand(new DbCommandFunction(
					[this]() {
						imp_->start();
					}));
			}
		}
	};

	void set_pingpong_texture();

	//得到离屏的图
	void create_first_fbo();

	//提取out line
	void create_secend_fbo();

	//模糊
	void create_blur_fbo();

	//最终显示
	void create_hud(osg::Texture2D* texture);

	void start();
};

void HighlightSystem::Impl::set_pingpong_texture()
{
	auto create_texture = []() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_RGBA);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		return texture2d;
	};

	first_texture_		 = create_texture();
	second_texture_		 = create_texture();
	first_texture_no_ed_ = create_texture();
}

void HighlightSystem::Impl::create_first_fbo()
{
	auto create_first_fbo = [this](Impl* impl, osg::Texture2D* texture, osg::Camera* mainCamera) {
		osg::Camera* first_fbo = new osg::Camera;
		// set up the background color and clear mask.
		first_fbo->setClearColor(CLEAR_COLOR);
		first_fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// set view
		first_fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		// set viewport
		first_fbo->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);

		// set the camera to render before the main camera.
		first_fbo->setRenderOrder(osg::Camera::PRE_RENDER);

		// tell the camera to use OpenGL frame buffer object where supported.
		first_fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

		// attach the texture and use it as the color buffer.
		first_fbo->attach(osg::Camera::COLOR_BUFFER, texture,
						  0, 0, false, MULTI_SAMPLE);

		first_fbo->setProjectionMatrix(mainCamera->getProjectionMatrix());
		first_fbo->setViewMatrix(mainCamera->getViewMatrix());
		first_fbo->addPreDrawCallback(new FirstCameraPredrawCallback(first_fbo, mainCamera, impl));

		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert	   = osgDB::readShaderFile(osg::Shader::VERTEX, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_first.vert");
		osg::Shader*			   frag	   = osgDB::readShaderFile(osg::Shader::FRAGMENT, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_first.frag");
		program->addShader(vert);
		program->addShader(frag);

		osg::StateSet* ss = first_fbo->getOrCreateStateSet();
		ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		osg::ref_ptr<osg::Uniform> color = new osg::Uniform("u_color", u_color);
		color->setUpdateCallback(new OutLineColorUniform(u_color));
		ss->addUniform(color);
		ss->setAttributeAndModes(new osg::LineWidth(1), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);
		return first_fbo;
	};

	osg::Camera* mainCamera = pOSG_->getView()->getCamera();
	first_fbo_ed_			= create_first_fbo(this, first_texture_, mainCamera);
	first_fbo_ed_->setName("first_fbo_ed_");
	first_fbo_ed_->addDescription("获取第一张纹理，需要边缘检测");

	first_fbo_no_ed_ = create_first_fbo(this, first_texture_no_ed_, mainCamera);
	first_fbo_no_ed_->setName("first_fbo_no_ed_");
	first_fbo_no_ed_->addDescription("获取第一张纹理，不需要边缘检测");
}

void HighlightSystem::Impl::create_secend_fbo()
{
	second_fbo_ = new osg::Camera;
	second_fbo_->setName("second_fbo_");
	second_fbo_->addDescription("获取轮廓");
	osg::Geometry* quat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(TEXTURE_SIZE1, 0, 0), osg::Vec3(0, TEXTURE_SIZE2, 0));
	{
		second_fbo_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		second_fbo_->setProjectionMatrixAsOrtho2D(0, TEXTURE_SIZE1, 0, TEXTURE_SIZE2);
		second_fbo_->setViewMatrix(osg::Matrix::identity());
		second_fbo_->setRenderOrder(osg::Camera::PRE_RENDER);
		second_fbo_->setClearColor(CLEAR_COLOR);
		second_fbo_->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		second_fbo_->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
		second_fbo_->getOrCreateStateSet()->setMode(GL_LIGHTING,
													osg::StateAttribute::OFF);
		second_fbo_->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);
		// attach the texture and use it as the color buffer.
		second_fbo_->attach(osg::Camera::COLOR_BUFFER, second_texture_,
							0, 0, false, MULTI_SAMPLE);

		osg::Geode* geode = new osg::Geode;
		second_fbo_->addChild(geode);
		geode->addChild(quat);
		quat->setSupportsDisplayList(false);

		osg::StateSet*			   ss	   = geode->getOrCreateStateSet();
		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert	   = osgDB::readShaderFile(osg::Shader::VERTEX, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_second.vert");
		osg::Shader*			   frag	   = osgDB::readShaderFile(osg::Shader::FRAGMENT, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_second.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("baseTexture_no_edge_detection", 1));
		ss->addUniform(new osg::Uniform("u_screen_width", TEXTURE_SIZE1));
		ss->addUniform(new osg::Uniform("u_screen_height", TEXTURE_SIZE2));
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(0, first_texture_, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(1, first_texture_no_ed_, osg::StateAttribute::ON);

		osg::ref_ptr<osg::Uniform> color = new osg::Uniform("u_color", u_color);
		color->setUpdateCallback(new OutLineColorUniform(u_color));
		ss->addUniform(color);

		osg::ref_ptr<osg::Uniform> gradient = new osg::Uniform("u_gradientThreshold", u_gradientThreshold);
		gradient->setUpdateCallback(new GradientThresholdUniform(u_gradientThreshold));
		ss->addUniform(gradient);
	}
}

void HighlightSystem::Impl::create_blur_fbo()
{
	vector<osg::Vec2> dirs;

	for (int i = 0; i < PING_PONG_NUM; i++)
	{
		dirs.push_back(i % 2 == 0 ? osg::Vec2(1, 0) : osg::Vec2(0, 1));
	}

	for (int i = 0; i < PING_PONG_NUM; i++)
	{
		pingpong_fbo_[i] = new osg::Camera;
		osg::Camera* fbo = pingpong_fbo_[i];

		osg::Geometry* screenQuat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(TEXTURE_SIZE1, 0, 0), osg::Vec3(0, TEXTURE_SIZE2, 0));
		{
			fbo->setName("FBO_CAMERA");
			// set up the background color and clear mask.
			fbo->setClearColor(CLEAR_COLOR);
			fbo->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// set view
			fbo->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			// set viewport
			fbo->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);

			// set the camera to render before the main camera.
			fbo->setRenderOrder(osg::Camera::PRE_RENDER);

			// tell the camera to use OpenGL frame buffer object where supported.
			fbo->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

			// attach the texture and use it as the color buffer.
			fbo->attach(osg::Camera::COLOR_BUFFER, i % 2 == 0 ? first_texture_ : second_texture_,
						0, 0, false, MULTI_SAMPLE);

			fbo->setProjectionMatrixAsOrtho2D(0, TEXTURE_SIZE1, 0, TEXTURE_SIZE2);
			fbo->setViewMatrix(osg::Matrix::identity());

			osg::Geode* geode = new osg::Geode;
			fbo->addChild(geode);
			geode->addChild(screenQuat);

			osg::StateSet* ss = geode->getOrCreateStateSet();

			ss->setTextureAttributeAndModes(0, i % 2 == 0 ? second_texture_ : first_texture_, osg::StateAttribute::ON);

			osg::ref_ptr<osg::Program> program = new osg::Program;
			osg::Shader*			   vert	   = osgDB::readShaderFile(osg::Shader::VERTEX, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_blur.vert");
			osg::Shader*			   frag	   = osgDB::readShaderFile(osg::Shader::FRAGMENT, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_blur.frag");
			program->addShader(vert);
			program->addShader(frag);
			ss->addUniform(new osg::Uniform("baseTexture", 0));
			ss->addUniform(new osg::Uniform("u_screen_width", TEXTURE_SIZE1));
			ss->addUniform(new osg::Uniform("u_screen_height", TEXTURE_SIZE2));
			ss->addUniform(new osg::Uniform("u_dir", dirs[i]));
			ss->setAttributeAndModes(program, osg::StateAttribute::ON);
		}
	}
}

void HighlightSystem::Impl::create_hud(osg::Texture2D* texture)
{
	osg::Geode*	   geode_quat = nullptr;
	osg::Geometry* screenQuat = nullptr;
	hud_camera_				  = new osg::Camera;

	screenQuat = osg::createTexturedQuadGeometry(osg::Vec3(), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	{
		hud_camera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hud_camera_->setProjectionMatrixAsOrtho2D(0, w_, 0, h_);
		hud_camera_->setViewMatrix(osg::Matrix::identity());
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
		ss->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

		osg::ref_ptr<osg::Program> program = new osg::Program;
		osg::Shader*			   vert	   = osgDB::readShaderFile(osg::Shader::VERTEX, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_final.vert");
		osg::Shader*			   frag	   = osgDB::readShaderFile(osg::Shader::FRAGMENT, DbSystemUtil::GetApplicationPath() + "Resources/shader/outline_final.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);

		osg::ref_ptr<osg::Uniform> alpha = new osg::Uniform("u_alpha_factor", u_alpha_factor);
		alpha->setUpdateCallback(new AlphaUniform(u_alpha_factor));
		ss->addUniform(alpha);
	}
}

void HighlightSystem::Impl::start()
{
	if (!is_enable) return;

	vector<osg::ref_ptr<osg::Node>> currentSelected, currentSelected_no_ed;
	{
		if (first_fbo_ed_)
		{
			for (int i = 0; i < first_fbo_ed_->getNumChildren(); i++)
				currentSelected.push_back(first_fbo_ed_->getChild(i));
		}

		if (first_fbo_no_ed_)
		{
			for (int i = 0; i < first_fbo_no_ed_->getNumChildren(); i++)
				currentSelected_no_ed.push_back(first_fbo_no_ed_->getChild(i));
		}
	}

	//--------------------------------------------------
	highlightRoot_->removeChildren(0, highlightRoot_->getNumChildren());
	set_pingpong_texture();

	create_first_fbo();
	create_secend_fbo();
	create_blur_fbo();
	create_hud(second_texture_);

	highlightRoot_->addChild(first_fbo_ed_);
	highlightRoot_->addChild(first_fbo_no_ed_);
	highlightRoot_->addChild(second_fbo_);

	for (int i = 0; i < PING_PONG_NUM; i++)
		highlightRoot_->addChild(pingpong_fbo_[i]);

	highlightRoot_->addChild(hud_camera_);

	for (auto& n : currentSelected) first_fbo_ed_->addChild(n);

	for (auto& n : currentSelected_no_ed) first_fbo_no_ed_->addChild(n);
}

//--------------------------HighlightSystem------------------------
HighlightSystem::HighlightSystem(CViewControlData* pOSG)
{
	impl		= new Impl;
	impl->pOSG_ = pOSG;
	pOSG->GetRoot()->addChild(impl->highlightRoot_);
	impl->highlightRoot_->setNodeMask(0);

	pOSG->addCommand(new DbCommandFunction(
		[this]() {
			impl->start();
		}));
}

HighlightSystem::~HighlightSystem()
{
	SAFE_DELETE(impl);
}

void HighlightSystem::addHighlight(osg::Node* pNode, bool isNeedEdgeDetection/* = true*/)
{
	if (!pNode || !impl->is_enable) return;
	impl->highlightRoot_->setNodeMask(NM_NO_PICK);

	if (isHighlight(pNode)) return;

	if (isNeedEdgeDetection)
		impl->first_fbo_ed_->addChild(pNode);		
	else
		impl->first_fbo_no_ed_->addChild(pNode);
}

void HighlightSystem::removeHighlight(osg::Node* pNode)
{
	if (!impl->is_enable) return;

	if (!isHighlight(pNode)) return;
	impl->first_fbo_ed_->removeChild(pNode);
	impl->first_fbo_no_ed_->removeChild(pNode);

	if (impl->first_fbo_ed_->getNumChildren() == 0 && impl->first_fbo_no_ed_->getNumChildren() == 0)
	{
		impl->highlightRoot_->setNodeMask(0);
	}
}

bool HighlightSystem::isHighlight(osg::Node* pNode)
{
	return impl->first_fbo_ed_->containsNode(pNode) || impl->first_fbo_no_ed_->containsNode(pNode);
}

void HighlightSystem::enable(bool flag)
{
	impl->highlightRoot_->setNodeMask(flag ? NM_NO_PICK : 0);
	impl->is_enable = flag;
	if (!flag)
	{
		impl->highlightRoot_->removeChildren(0, impl->highlightRoot_->getNumChildren());
	}
}

void HighlightSystem::updateViewmatrix(const osg::Matrixd& mat)
{
	if (impl->first_fbo_ed_) impl->first_fbo_ed_->setViewMatrix(mat);
	if (impl->first_fbo_no_ed_) impl->first_fbo_no_ed_->setViewMatrix(mat);
}

void HighlightSystem::updateProjectionMatrix(const osg::Matrixd& mat)
{
	if (impl->first_fbo_ed_) impl->first_fbo_ed_->setProjectionMatrix(mat);
	if (impl->first_fbo_no_ed_) impl->first_fbo_no_ed_->setProjectionMatrix(mat);
}

void HighlightSystem::setColor(const osg::Vec4& color)
{
	impl->u_color		 = osg::Vec3(color[0], color[1], color[2]);
	impl->u_alpha_factor = color.w();
}

void HighlightSystem::setOutLineFactor(float f /*= 0.005*/)
{
	impl->u_gradientThreshold = f;
}
