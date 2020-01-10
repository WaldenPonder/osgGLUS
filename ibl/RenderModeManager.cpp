#include "RenderModeManager.h"
#include <osg/Texture1D>
#include <osgUtil/PerlinNoise>
#include <osgDB/ReadFile>
#include "IView.h"
#include "dtSetting.h"
#include "OSGViewer.h"
#include "osg/Node"

#define TEXUNIT_SINE 1
#define TEXUNIT_NOISE 2
using namespace osg;

//渲染模式
enum render_model
{
	rm_normal		 = 0,
	rm_wire			 = 1,
	rm_phong		 = 2,
	rm_pbr			 = 3,
	rm_dynamic_light = 4,
	rm_marble		 = 5,
	rm_pure_color	= 6,
};

//选中效果
enum select_effect
{
	eft_out_line,
	eft_scribe,
	eft_color,
};

struct program_type
{
	Program_ptr phong;
	Program_ptr pbr;
	Program_ptr dynamic_light;
	Program_ptr marble;
	Program_ptr pure_color;
};

class LightPosCallback : public osg::Uniform::Callback
{
 public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		const osg::FrameStamp* fs = nv->getFrameStamp();
		if (fs)
		{
			float angle = osg::inDegrees(( float )fs->getFrameNumber());
			uniform->set(osg::Vec3(20.f * std::cosf(angle), 20.f * std::sinf(angle), 1.f));
		}
	}
};

class CameraPosCallback : public osg::Uniform::Callback
{
 public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		if (!_viewer) return;

		osgGA::StandardManipulator* mani = dynamic_cast<osgGA::StandardManipulator*>(_viewer->getCameraManipulator());
		if (mani)
		{
			osg::Vec3d eye;
			osg::Quat  quat;
			mani->getTransformation(eye, quat);

			osg::Vec3 pos1 = eye * _viewer->getCamera()->getViewMatrix();
			uniform->set(pos1);
		}
	}

	IView* _viewer;
};

class SelectCallback : public osg::Uniform::Callback
{
 public:
	virtual void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		if (!obj.get()) return;

		dtObject_ptr ptr;
		if (obj.lock(ptr))
		{
			uniform->set(ptr->selected());
		}		
	}

	dtObject_wptr obj;
};

struct RenderModeManager::Impl
{
	IView*		 _viewer;
	program_type _program;

	//phong
	osg::ref_ptr<osg::Uniform> albedo_factor = new osg::Uniform("albedo_factor", .8f);
	osg::ref_ptr<osg::Uniform> diff_factor   = new osg::Uniform("diff_factor", .3f);
	osg::ref_ptr<osg::Uniform> spec_factor   = new osg::Uniform("spec_factor", .3f);
	osg::ref_ptr<osg::Uniform> CameraPos	 = new osg::Uniform("CameraPos", osg::Vec3());
	//marble
	osg::ref_ptr<osg::Uniform> NoiseTex = new osg::Uniform("NoiseTex", TEXUNIT_NOISE);
	osg::ref_ptr<osg::Uniform> SineTex = new osg::Uniform("SineTex", TEXUNIT_SINE);
	//pbr
	osg::ref_ptr<osg::Uniform> metallic = new osg::Uniform("metallic", 63 / 100.f);
	osg::ref_ptr<osg::Uniform> roughness = new osg::Uniform("roughness", 33 / 100.f);
	osg::ref_ptr<osg::Uniform> ambient_factor = new osg::Uniform("ambient_factor", 20 / 100.f);
	//dynamic_light
	osg::ref_ptr<osg::Uniform> lightSpecular = new osg::Uniform("lightSpecular", osg::Vec4(1, 1, 1, 1));
	osg::ref_ptr<osg::Uniform> shininess = new osg::Uniform("shininess", 64.f);
	osg::ref_ptr<osg::Uniform> lightPos = new osg::Uniform("lightPosition", osg::Vec3());


	osg::ref_ptr<CameraPosCallback> _cameraPosCallback = new CameraPosCallback;
	osg::ref_ptr<LightPosCallback>  _lightPosCallback  = new LightPosCallback;

	osg::ref_ptr<osg::Texture3D> _noiseTexture;
	osg::ref_ptr<osg::Texture1D> _sineTexture;

	render_model  _render_model  = rm_phong;
	select_effect _select_effect = eft_scribe;
};

#pragma region marble

static osg::Image* make1DSineImage(int texSize)
{
	const float PI = 3.1415927;

	osg::Image* image = new osg::Image;
	image->setImage(texSize, 1, 1,
					4, GL_RGBA, GL_UNSIGNED_BYTE,
					new unsigned char[4 * texSize],
					osg::Image::USE_NEW_DELETE);

	GLubyte* ptr = image->data();
	float	inc = 2. * PI / ( float )texSize;
	for (int i = 0; i < texSize; i++)
	{
		*ptr++ = (GLubyte)((sinf(i * inc) * 0.5 + 0.5) * 255.);
		*ptr++ = 0;
		*ptr++ = 0;
		*ptr++ = 1;
	}
	return image;
}

static osg::Texture1D* make1DSineTexture(int texSize)
{
	osg::Texture1D* sineTexture = new osg::Texture1D;
	sineTexture->setWrap(osg::Texture1D::WRAP_S, osg::Texture1D::REPEAT);
	sineTexture->setFilter(osg::Texture1D::MIN_FILTER, osg::Texture1D::LINEAR);
	sineTexture->setFilter(osg::Texture1D::MAG_FILTER, osg::Texture1D::LINEAR);
	sineTexture->setImage(make1DSineImage(texSize));
	return sineTexture;
}

#pragma endregion

class SetUniformVisitor : public osg::NodeVisitor
{
 public:
	SetUniformVisitor(RenderModeManager& m) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _rmm(m)
	{
	}

	virtual void apply(osg::Geometry& geometry)
	{
		osg::Array* colorArr = geometry.getColorArray();
		if (!colorArr) return;

		bool flag = true;

		osg::Array::Type type = colorArr->getType();

		_ss				   = getStateSet(geometry);
		osg::Material* mat = dynamic_cast<osg::Material*>(_ss->getAttribute(osg::StateAttribute::MATERIAL));
		if (mat)
		{
			diffuse  = mat->getDiffuse(osg::Material::FRONT);
			ambient  = mat->getAmbient(osg::Material::FRONT);
			specular = mat->getSpecular(osg::Material::FRONT);
		}
		else if (type == osg::Array::Vec4ubArrayType)
		{
			osg::Vec4ubArray* arr = dynamic_cast<osg::Vec4ubArray*>(colorArr);
			if (arr && arr->size())
			{
				for (auto v : *arr)
				{
					osg::Vec4ub var = v;
					ambient = diffuse = specular = osg::Vec4(var[0] / 255.f, var[1] / 255.f, var[2] / 255.f, var[3] / 255.f);
					flag						 = true;
				}
			}
		}
		else if (type == osg::Array::Vec4dArrayType)
		{
			osg::Vec4dArray* arr2 = dynamic_cast<osg::Vec4dArray*>(colorArr);
			if (arr2 && arr2->size())
			{
				ambient = diffuse = specular = arr2->at(0);
				flag						 = true;
			}
		}
		else if (type == osg::Array::Vec4ArrayType)
		{
			osg::Vec4Array* arr = dynamic_cast<osg::Vec4Array*>(colorArr);
			if (arr && arr->size())
			{
				ambient = diffuse = specular = arr->at(0);
				flag						 = true;
			}
		}

		if (flag)
		{
			setUniform(geometry);
		}

		traverse(geometry);
	}

	osg::StateSet* getStateSet(osg::Geometry& geometry)
	{
		if (geometry.getStateSet()) return geometry.getStateSet();

		osg::Node* n = &geometry;
		while (n && n->getNumParents())
		{
			for (size_t i = 0; i < n->getNumParents(); i++)
			{
				n = n->getParent(i);

				if (n == _rmm.impl->_viewer->graphNode())
					continue;

				if (n && n->getStateSet()) return n->getStateSet();
			}
		}

		return geometry.getOrCreateStateSet();
	}

	void setUniform(osg::Geometry& geometry)
	{
		dtSetting* st = db_data->get<dtSetting>(g::id_setting);
		if (!st) return;

		_rmm.impl->_render_model = render_model(st->_render_model.val);
		_rmm.impl->_select_effect = select_effect(st->_select_model.val);

		_ss = getStateSet(geometry);
		_ss->getUniformList().clear();

		if (_rmm.impl->_render_model == rm_dynamic_light)
		{
			_ss->addUniform(new osg::Uniform("lightDiffuse", ambient));

			_ss->addUniform(_rmm.impl->lightSpecular);
			_ss->addUniform(_rmm.impl->shininess);
			_ss->addUniform(_rmm.impl->lightPos);
		}
		else if (_rmm.impl->_render_model == rm_phong)
		{
			_rmm.impl->albedo_factor->set(st->_phong_albedo / 100.f);
			_rmm.impl->diff_factor->set(st->_phong_diffuse / 100.f);
			_rmm.impl->spec_factor->set(st->_phong_specular / 100.f);

			_ss->addUniform(new osg::Uniform("albedo", ambient));

			_ss->addUniform(_rmm.impl->albedo_factor);
			_ss->addUniform(_rmm.impl->diff_factor);
			_ss->addUniform(_rmm.impl->spec_factor);
			_ss->addUniform(_rmm.impl->CameraPos);
		}
		else if (_rmm.impl->_render_model == rm_pbr)
		{
			_ss->addUniform(new osg::Uniform("albedo", ambient));

			_rmm.impl->metallic->set(st->_pbr_metallic / 100.f);
			_rmm.impl->roughness->set(st->_pbr_roughness / 100.f);
			_rmm.impl->ambient_factor->set(st->_pbr_ambient_factor / 100.f);

			_ss->addUniform(_rmm.impl->metallic);
			_ss->addUniform(_rmm.impl->roughness);
			_ss->addUniform(_rmm.impl->ambient_factor);
			_ss->addUniform(_rmm.impl->CameraPos);
		}
		else if (_rmm.impl->_render_model == rm_marble)
		{
			if (!_rmm.impl->_noiseTexture)
			{
				_rmm.impl->_noiseTexture = osgUtil::create3DNoiseTexture(256);
			}

			if (!_rmm.impl->_sineTexture)
			{
				_rmm.impl->_sineTexture = make1DSineTexture(1024);
			}

			_ss->setTextureAttribute(TEXUNIT_NOISE, _rmm.impl->_noiseTexture);
			_ss->setTextureAttribute(TEXUNIT_SINE, _rmm.impl->_sineTexture);
			_ss->addUniform(_rmm.impl->NoiseTex);
			_ss->addUniform(_rmm.impl->SineTex);
		}
	}

	//要最后调用
	void selectedUpdate(dtObject_wptr dt)
	{
		if (_rmm.impl->_render_model == rm_normal)
			return;

		if (dt.get() && _ss.get())
		{
			osg::ref_ptr<osg::Uniform>   selected = new osg::Uniform("selected", dt->selected());
			osg::ref_ptr<SelectCallback> callback = new SelectCallback;
			callback->obj						  = dt;
			selected->setUpdateCallback(callback);
			_ss->addUniform(selected);
		}
	}

	osg::Vec4 diffuse;
	osg::Vec4 specular = osg::Vec4(1, 1, 1, 1);
	osg::Vec4 ambient  = osg::Vec4(1, 1, 1, 1);

	osg::observer_ptr<osg::StateSet>	 _ss;
	RenderModeManager& _rmm;
};

RenderModeManager::RenderModeManager()
{
	impl = new Impl;
	impl->CameraPos->setUpdateCallback(impl->_cameraPosCallback);
	impl->lightPos->setUpdateCallback(impl->_lightPosCallback);
}

RenderModeManager::~RenderModeManager()
{
	SAFE_DELETE(impl);
}

Program_ptr RenderModeManager::program(int rm)
{
	if (impl->_viewer->getOSGView2D() || impl->_viewer->getOSGViewer2D()) return nullptr;

	if (rm == rm_normal || rm == rm_wire)
	{
		return nullptr;
	}
	else if (rm == rm_pbr)
	{
		impl->_program.pbr = new osg::Program;
		osg::Shader* vert  = osgDB::readShaderFile(osg::Shader::VERTEX, app::app_dir_path + "/shader/PBR.vert");
		osg::Shader* frag  = osgDB::readShaderFile(osg::Shader::FRAGMENT, app::app_dir_path + "/shader/PBR.frag");
		impl->_program.pbr->addShader(vert);
		impl->_program.pbr->addShader(frag);

		return impl->_program.pbr;
	}
	else if (rm == rm_phong)
	{
		if (!impl->_program.phong)
		{
			impl->_program.phong = new osg::Program;
			osg::Shader* vert	= osgDB::readShaderFile(osg::Shader::VERTEX, app::app_dir_path + "/shader/Phong.vert");
			osg::Shader* frag	= osgDB::readShaderFile(osg::Shader::FRAGMENT, app::app_dir_path + "/shader/Phong.frag");
			impl->_program.phong->addShader(vert);
			impl->_program.phong->addShader(frag);
		}

		return impl->_program.phong;
	}
	else if (rm == rm_dynamic_light)
	{
		if (!impl->_program.dynamic_light)
		{
			impl->_program.dynamic_light = new osg::Program;
			osg::Shader* vert			 = osgDB::readShaderFile(osg::Shader::VERTEX, app::app_dir_path + "/shader/dynamic_light.vert");
			osg::Shader* frag			 = osgDB::readShaderFile(osg::Shader::FRAGMENT, app::app_dir_path + "/shader/dynamic_light.frag");
			impl->_program.dynamic_light->addShader(vert);
			impl->_program.dynamic_light->addShader(frag);
		}

		return impl->_program.dynamic_light;
	}
	else if (rm == rm_marble)
	{
		if (!impl->_program.marble)
		{
			impl->_program.marble = new osg::Program;
			osg::Shader* vert	 = osgDB::readShaderFile(osg::Shader::VERTEX, app::app_dir_path + "/shader/marble.vert");
			osg::Shader* frag	 = osgDB::readShaderFile(osg::Shader::FRAGMENT, app::app_dir_path + "/shader/marble.frag");
			impl->_program.marble->addShader(vert);
			impl->_program.marble->addShader(frag);
		}

		return impl->_program.marble;
	}
	else if (rm == rm_pure_color)
	{
		if (!impl->_program.pure_color)
		{
			impl->_program.pure_color = new osg::Program;
			osg::Shader* vert		  = osgDB::readShaderFile(osg::Shader::VERTEX, app::app_dir_path + "/shader/pure_color.vert");
			osg::Shader* frag		  = osgDB::readShaderFile(osg::Shader::FRAGMENT, app::app_dir_path + "/shader/pure_color.frag");
			impl->_program.pure_color->addShader(vert);
			impl->_program.pure_color->addShader(frag);
		}

		return impl->_program.pure_color;
	}

	return nullptr;
}

void RenderModeManager::setWireModel(bool b)
{
	osg::StateSet*	ss		  = impl->_viewer->getCamera()->getOrCreateStateSet();
	osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(ss->getAttribute(osg::StateAttribute::POLYGONMODE));
	if (!polyModeObj)
	{
		polyModeObj = new osg::PolygonMode;
		ss->setAttribute(polyModeObj);
	}

	polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK, b ? osg::PolygonMode::LINE : osg::PolygonMode::FILL);
}

void RenderModeManager::setViewer(IView* v)
{
	if (v)
	{
		impl->_viewer = v;
		impl->_cameraPosCallback->_viewer = v;
	}
}

void RenderModeManager::updataProgram()
{
	if (impl->_viewer->getOSGView2D() || impl->_viewer->getOSGViewer2D()) return;

	dtSetting* st = db_data->get<dtSetting>(g::id_setting);
	if (!st) return;

	//设置渲染模式
	setWireModel(false);

	impl->_render_model  = render_model(st->_render_model.val);
	impl->_select_effect = select_effect(st->_select_model.val);

	osg::StateSet* _ss = impl->_viewer->graphNode()->getOrCreateStateSet();

	if (impl->_render_model == render_model::rm_normal)
	{
		osg::Program* program_ = dynamic_cast<osg::Program*>(_ss->getAttribute(osg::StateAttribute::PROGRAM));
		if (program_)
		{
			_ss->removeAttribute(program_);
		}
	}
	else if (impl->_render_model == render_model::rm_wire)
	{
		setWireModel(true);
	}
	else
	{
		_ss->setAttributeAndModes(program(impl->_render_model), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	}
}

void RenderModeManager::setUniform(dtObject_wptr dt)
{
	if (impl->_viewer->getOSGView2D() || impl->_viewer->getOSGViewer2D()) return;

	if (!dt.get() || !dt->object()) return;

	dtObject* ptr = dt.get();

	osg::Node* node = dynamic_cast<osg::Node*>(ptr->object());
	if (!node) return;

	SetUniformVisitor fssv(*this);
	node->accept(fssv);

	fssv.selectedUpdate(dt);
}

void RenderModeManager::updateUniform(osg::ref_ptr<osg::Node> node)
{
	if (impl->_viewer->getOSGView2D() || impl->_viewer->getOSGViewer2D()) return;

	if (!node) return;

	SetUniformVisitor fssv(*this);
	node->accept(fssv);
}
