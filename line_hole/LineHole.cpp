#include "pch.h"
#include "LineHole.h"
#include "osg/BlendFunc"
#include <osg/PolygonOffset>

std::vector<osg::Texture2D*> LineHole::createRttCamera(osgViewer::Viewer* viewer)
{
	auto create_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_RGBA);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		return texture2d;
	};

	auto create_id_texture = [&]() {
		osg::Texture2D* texture2d = new osg::Texture2D;
		texture2d->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture2d->setInternalFormat(GL_R32UI);
		texture2d->setSourceFormat(GL_R32UI);
		texture2d->setSourceType(GL_UNSIGNED_INT);
		texture2d->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
		texture2d->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
		return texture2d;
	};

	auto create_depth_texture = []()
	{
		// Setup shadow texture
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setTextureSize(TEXTURE_SIZE1, TEXTURE_SIZE2);
		texture->setInternalFormat(GL_DEPTH_COMPONENT);
		texture->setShadowComparison(true);
		texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
		texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

		// the shadow comparison should fail if object is outside the texture
		texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
		texture->setBorderColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		return texture;
	};

	osg::Camera* rttCamera = new osg::Camera;
	g_root->addChild(rttCamera);
	// set up the background color and clear mask.
	rttCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set view
	rttCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	// set viewport
	rttCamera->setViewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2);

	// set the camera to render before the main camera.
	rttCamera->setRenderOrder(osg::Camera::PRE_RENDER);

	// tell the camera to use OpenGL frame buffer object where supported.
	rttCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

	g_texture = create_texture();
	g_depthTexture = create_texture();
	g_idTexture = create_id_texture();
	g_linePtTexture = create_texture();

	// attach the texture and use it as the color buffer.
	rttCamera->attach(osg::Camera::COLOR_BUFFER0, g_texture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER1, g_idTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER2, g_depthTexture, 0, 0, false);
	rttCamera->attach(osg::Camera::COLOR_BUFFER3, g_linePtTexture, 0, 0, false);

	osg::Camera* mainCamera = viewer->getCamera();
	rttCamera->setProjectionMatrix(mainCamera->getProjectionMatrix());
	rttCamera->setViewMatrix(mainCamera->getViewMatrix());
	rttCamera->addPreDrawCallback(new CameraPredrawCallback(rttCamera, mainCamera));

	rttCamera->addChild(create_lines(*viewer));
	
	return { g_texture, g_depthTexture, g_idTexture, g_linePtTexture };
}

osg::ref_ptr<osg::TextureBuffer> LineHole::create_tbo(const vector<int>& data)
{
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(data.size(), 1, 1, GL_R32I, GL_INT);

	for (int i = 0; i < data.size(); i++)
	{
		int* ptr = (int*)image->data(i);
		*ptr = data[i];
	}

	osg::ref_ptr<osg::TextureBuffer> tbo = new osg::TextureBuffer;
	tbo->setImage(image.get());
	tbo->setInternalFormat(GL_R32I);
	return tbo;
}

osg::Camera* LineHole::createHudCamera(osgViewer::Viewer* viewer, std::vector<osg::Texture2D*> TEXTURES)
{
	osg::Geode* geode_quat = nullptr;
	osg::Geometry* screenQuat = nullptr;
	osg::Camera* hud_camera_ = new osg::Camera;

	osg::Camera* mainCamera = viewer->getCamera();
	mainCamera->setClearColor(CLEAR_COLOR);
	osg::Viewport* vp = mainCamera->getViewport();

	float w_ = TEXTURE_SIZE1;
	float h_ = TEXTURE_SIZE2;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	screenQuat = createFinalHudTextureQuad(program, osg::Vec3(), osg::Vec3(w_, 0, 0), osg::Vec3(0, h_, 0));
	{
		hud_camera_->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
		hud_camera_->setProjectionMatrixAsOrtho2D(0, w_, 0, h_);
		hud_camera_->setViewMatrix(osg::Matrix::identity());
		hud_camera_->setRenderOrder(osg::Camera::POST_RENDER);
		hud_camera_->setClearColor(CLEAR_COLOR);
		hud_camera_->setClearMask(GL_DEPTH_BUFFER_BIT);
		hud_camera_->getOrCreateStateSet()->setMode(GL_LIGHTING,
			osg::StateAttribute::OFF);

		geode_quat = new osg::Geode;
		hud_camera_->addChild(geode_quat);
		screenQuat->setUseVertexBufferObjects(true);
		geode_quat->addChild(screenQuat);
		geode_quat->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		geode_quat->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

		screenQuat->setDataVariance(osg::Object::DYNAMIC);
		screenQuat->setSupportsDisplayList(false);

		osg::StateSet* ss = geode_quat->getOrCreateStateSet();
		ss->setTextureAttributeAndModes(0, TEXTURES[0], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(1, TEXTURES[1], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(2, TEXTURES[2], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(3, TEXTURES[3], osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(4, g_textureBuffer1, osg::StateAttribute::ON);
		ss->setTextureAttributeAndModes(5, g_textureBuffer2, osg::StateAttribute::ON);

		osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_quad.vert");
		osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_quad.frag");
		program->addShader(vert);
		program->addShader(frag);
		ss->addUniform(new osg::Uniform("baseTexture", 0));
		ss->addUniform(new osg::Uniform("depthTexture", 1));
		ss->addUniform(new osg::Uniform("idTexture", 2));
		ss->addUniform(new osg::Uniform("linePtTexture", 3));
		ss->addUniform(new osg::Uniform("textureBuffer1", 4));
		ss->addUniform(new osg::Uniform("textureBuffer2", 5));

		osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
		u_MVP->setUpdateCallback(new MVPCallback(hud_camera_));
		ss->addUniform(u_MVP);

		//	program->addBindAttribLocation("a_pos", 0);
		//	program->addBindAttribLocation("a_uv", 1);
		ss->setAttributeAndModes(program, osg::StateAttribute::ON);
	}

	return hud_camera_;
}

void LineHole::setUpHiddenLineStateset(osg::StateSet* ss, osg::Camera* camera)
{
	osg::Depth* depth = new osg::Depth(osg::Depth::GREATER);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);
	ss->setAttributeAndModes(new osg::LineStipple(1, 0x0fff), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("line_hole");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_dot_line.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/line_hole/line_hole_dot_line.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_dot_line.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);

	//-----------attribute  addBindAttribLocation
	program->addBindAttribLocation("a_pos", 0);
	program->addBindAttribLocation("a_color", 1);
	program->addBindAttribLocation("a_id", 2);

	//-----------------------------------------------uniform
	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);
}

void LineHole::setUpStateset(osg::StateSet* ss, osg::Camera* camera)
{
	osg::Depth* depth = new osg::Depth(osg::Depth::LEQUAL);
	ss->setAttributeAndModes(depth, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED);
	ss->setAttributeAndModes(new osg::LineWidth(2), osg::StateAttribute::ON);

	//------------------------osg::Program-----------------------------
	osg::Program* program = new osg::Program;
	program->setName("LINESTRIPE");
	program->addShader(osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole.vert"));
	program->addShader(osgDB::readShaderFile(osg::Shader::GEOMETRY, shader_dir() + "/line_hole/line_hole.geom"));
	program->addShader(osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole.frag"));

	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	//-----------attribute  addBindAttribLocation
	program->addBindAttribLocation("a_pos", 0);
	program->addBindAttribLocation("a_color", 1);
	program->addBindAttribLocation("a_id", 2);

	//-----------------------------------------------uniform
	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);
}

osg::Geometry* LineHole::createFinalHudTextureQuad(osg::ref_ptr<osg::Program> program, const osg::Vec3& corner,
	const osg::Vec3& widthVec, const osg::Vec3& heightVec, float l /*= 0*/, float b /*= 0*/, float r /*= 1*/, float t /*= 1*/)
{
	using namespace osg;
	Geometry* geom = new Geometry;

	Vec3Array* coords = new Vec3Array(4);
	(*coords)[0] = corner + heightVec;
	(*coords)[1] = corner;
	(*coords)[2] = corner + widthVec;
	(*coords)[3] = corner + widthVec + heightVec;
	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	Vec2Array* tcoords = new Vec2Array(4);
	(*tcoords)[0].set(l, t);
	(*tcoords)[1].set(l, b);
	(*tcoords)[2].set(r, b);
	(*tcoords)[3].set(r, t);
	geom->setVertexAttribArray(1, tcoords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_uv", 1);

	DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
	elems->push_back(0);
	elems->push_back(1);
	elems->push_back(2);

	elems->push_back(2);
	elems->push_back(3);
	elems->push_back(0);
	geom->addPrimitiveSet(elems);

	return geom;
}

osg::Geometry* LineHole::myCreateTexturedQuadGeometry2(osg::Camera* camera, int id,
	const osg::Vec3& corner, const osg::Vec3& widthVec, const osg::Vec3& heightVec,
	float l /*= 0*/, float b /*= 0*/, float r /*= 1*/, float t /*= 1*/)
{
	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->setName("LINE_HOLE_FACE");
	using namespace osg;
	Geometry* geom = new Geometry;

	Vec3Array* coords = new Vec3Array(4);
	(*coords)[0] = corner + heightVec;
	(*coords)[1] = corner;
	(*coords)[2] = corner + widthVec;
	(*coords)[3] = corner + widthVec + heightVec;
	geom->setVertexArray(coords);
	geom->setVertexAttribArray(0, coords, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_pos", 0);

	Vec4Array* colors = new Vec4Array();
	osg::ref_ptr<osg::UIntArray> a_id = new osg::UIntArray;

	for (int i = 0; i < 4; i++)
	{
		a_id->push_back(id);
		colors->push_back(osg::Vec4(0, 1, 0, 1));
	}

	geom->setVertexAttribArray(1, colors, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_color", 1);

	geom->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	geom->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);
	program->addBindAttribLocation("a_id", 2);

	DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
	elems->push_back(0);
	elems->push_back(1);
	elems->push_back(2);

	elems->push_back(2);
	elems->push_back(3);
	elems->push_back(0);
	geom->addPrimitiveSet(elems);

	osg::Shader* vert = osgDB::readShaderFile(osg::Shader::VERTEX, shader_dir() + "/line_hole/line_hole_face.vert");
	osg::Shader* frag = osgDB::readShaderFile(osg::Shader::FRAGMENT, shader_dir() + "/line_hole/line_hole_face.frag");
	program->addShader(vert);
	program->addShader(frag);

	auto ss = geom->getOrCreateStateSet();

	osg::Uniform* u_MVP(new osg::Uniform(osg::Uniform::FLOAT_MAT4, "u_MVP"));
	u_MVP->setUpdateCallback(new MVPCallback(camera));
	ss->addUniform(u_MVP);
	ss->setAttributeAndModes(program, osg::StateAttribute::ON);

	osg::BlendFunc* blend = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	//ss->setAttributeAndModes(blend, osg::StateAttribute::ON);

	osg::ColorMask* colorMask = new osg::ColorMask(false, false, false, false);
	ss->setAttributeAndModes(colorMask, osg::StateAttribute::ON);

	//让面远离视角
	osg::PolygonOffset* offset = new osg::PolygonOffset(1, 1);
	ss->setAttributeAndModes(offset, osg::StateAttribute::ON);

	return geom;
}

osg::Geometry* LineHole::createLine2(const std::vector<osg::Vec3>& allPTs, const std::vector<osg::Vec4>& colors,
	const std::vector<int>& ids, osg::Camera* camera, osg::PrimitiveSet::Mode mode /*= osg::PrimitiveSet::LINE_LOOP*/)
{
	cout << "osg::getGLVersionNumber" << osg::getGLVersionNumber() << endl;

	//传递给shader
	osg::ref_ptr<osg::Vec4Array> a_color = new osg::Vec4Array;
	osg::ref_ptr<osg::UIntArray> a_id = new osg::UIntArray;

	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> a_pos = new osg::Vec3Array;

	for (int i = 0; i < allPTs.size(); i++)
	{
		a_pos->push_back(allPTs[i]);
	}

	osg::ref_ptr<osg::ElementBufferObject> ebo = new osg::ElementBufferObject;
	osg::ref_ptr<osg::DrawElementsUInt>	indices = new osg::DrawElementsUInt(mode);

	for (unsigned int i = 0; i < allPTs.size(); i++)
	{
		indices->push_back(i);

		if (i < colors.size())
			a_color->push_back(colors[i]);
		else
			a_color->push_back(colors.back());

		if (i < ids.size())
			a_id->push_back(ids[i]);
		else
			a_id->push_back(ids.back());
	}

	indices->setElementBufferObject(ebo);
	pGeometry->addPrimitiveSet(indices.get());
	pGeometry->setUseVertexBufferObjects(true);

	//-----------attribute  addBindAttribLocation
	pGeometry->setVertexArray(a_pos);
	pGeometry->setVertexAttribArray(0, a_pos, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(0, osg::Geometry::BIND_PER_VERTEX);

	pGeometry->setVertexAttribArray(1, a_color, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(1, osg::Geometry::BIND_PER_VERTEX);

	pGeometry->setVertexAttribArray(2, a_id, osg::Array::BIND_PER_VERTEX);
	pGeometry->setVertexAttribBinding(2, osg::Geometry::BIND_PER_VERTEX);

	return pGeometry.release();
}

#define SCENE1

#ifdef  SCENE1
//虚线 区分内外
osg::Node* LineHole::create_lines(osgViewer::Viewer& view)
{	
	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	g_hidden_line_geode = new osg::Geode;
	root->addChild(geode);
	root->addChild(g_hidden_line_geode);
	vector<osg::Vec3>		 PTs, COLORs;

	setUpStateset(geode->getOrCreateStateSet(), view.getCamera());
	setUpHiddenLineStateset(g_hidden_line_geode->getOrCreateStateSet(), view.getCamera());

	float z;
	//-------------------------------------------------
	osg::Geode* geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1.5, -1.5, -0.5), osg::Vec3(3, 0, 0), osg::Vec3(0, 3, 0));
		geode2->addChild(geom);
		root->addChild(geode2);

		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(1, "RenderBin"); //面

		PTs.clear();
		z = -0.5;
		PTs.push_back(osg::Vec3(-1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1.5, 1.5, z));
		
		osg::Geometry* n4 = createLine2(PTs, { osg::Vec4(1, 0, 0, 1) }, { 3 }, view.getCamera());
		g_hidden_line_geode->addDrawable(n4);
		ss = n4->getOrCreateStateSet();
		ss->setRenderBinDetails(20, "RenderBin"); //虚线
		
		n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { 3 }, view.getCamera());
		n4->setName("LINE4");
		geode->addDrawable(n4);
		ss = n4->getOrCreateStateSet();
		ss->setRenderBinDetails(3, "RenderBin"); //实线
	}

	   	
	//-------------------------------------------------
	geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1, -1, 0), osg::Vec3(2.5, 0, 0), osg::Vec3(0, 2.5, 0));
		geode2->addChild(geom);
		root->addChild(geode2);
		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(1, "RenderBin"); //面

		PTs.clear();
		z = 0;
		PTs.push_back(osg::Vec3(-1, -1, z));
		PTs.push_back(osg::Vec3(1.5, -1, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1, 1.5, z));

		osg::Geometry* n5 = createLine2(PTs, { osg::Vec4(1, 0, 0, 1) }, { 2 }, view.getCamera());
		g_hidden_line_geode->addDrawable(n5);
		ss = n5->getOrCreateStateSet();
		ss->setRenderBinDetails(20, "RenderBin"); //虚线

		n5 = createLine2(PTs, { osg::Vec4(1, 1, 1, 1) }, { 2 }, view.getCamera());
		n5->setName("LINE5");
		geode->addDrawable(n5);
			   
		ss = n5->getOrCreateStateSet();
		ss->setRenderBinDetails(3, "RenderBin"); //实线
	}

	vector<int> index1(100);
	vector<int> index2(100);
	g_textureBuffer1 = create_tbo(index1);
	g_textureBuffer2 = create_tbo(index2);

	osg::ComputeBoundsVisitor cbbv;
	root->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return root.release();
}

#endif

#if 0


osg::Node* LineHole::create_lines(osgViewer::Viewer& view)
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<float> rd(0, 1);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	root->addChild(geode);
	vector<osg::Vec3>		 PTs, COLORs;

	setUpStateset(geode->getOrCreateStateSet(), view.getCamera());

	float z = 0.5f;
	PTs.push_back(osg::Vec3(2, 0, z));
	PTs.push_back(osg::Vec3(-2, 0, z));
	osg::Geometry* n = createLine2(PTs, { osg::Vec4(1, 0, 0, 1) }, { 1 }, view.getCamera());
	n->setName("LINE1");
	//geode->addDrawable(n);
	//g_hidden_line_geode->addDrawable(n);

	PTs.clear();
	z = 0.35f;
	PTs.push_back(osg::Vec3(0, 2, z));
	PTs.push_back(osg::Vec3(0, -2, z));
	n = createLine2(PTs, { osg::Vec4(1, 1, 0, 1) }, { 4 }, view.getCamera());
	n->setName("LINE2");
	//geode->addDrawable(n);
	//g_hidden_line_geode->addDrawable(n);

	//-------------------------------------------------
	//PTs.clear();
	//z = 0;
	//PTs.push_back(osg::Vec3(-1, -1.3, z));
	//PTs.push_back(osg::Vec3(1, -1, z));
	//PTs.push_back(osg::Vec3(1, 1, z));
	//PTs.push_back(osg::Vec3(-1, 1.3, z));
	//osg::Geometry* n2 = createLine2(PTs, { osg::Vec3(0, 1, 0) }, { 2 }, view.getCamera());
	//n2->setName("LINE3");
	//geode->addDrawable(n2);

	//-------------------------------------------------
	osg::Geode* geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1.5, -1.5, -0.5), osg::Vec3(3, 0, 0), osg::Vec3(0, 3, 0));
		geode2->addChild(geom);
		root->addChild(geode2);

		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(-100, "RenderBin"); //面

		PTs.clear();
		z = -0.5;
		PTs.push_back(osg::Vec3(-1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1.5, 1.5, z));

		osg::Geometry* n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { 3 }, view.getCamera());
		n4->setName("LINE4");
		geode->addDrawable(n4);
		g_hidden_line_geode->addDrawable(n4);
	}


	//-------------------------------------------------
	geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1, -1, 0), osg::Vec3(2.5, 0, 0), osg::Vec3(0, 2.5, 0));
		geode2->addChild(geom);
		root->addChild(geode2);

		PTs.clear();
		z = 0;
		PTs.push_back(osg::Vec3(-1, -1, z));
		PTs.push_back(osg::Vec3(1.5, -1, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1, 1.5, z));
		osg::Geometry* n5 = createLine2(PTs, { osg::Vec4(0, 1, 1, 1) }, { 888 }, view.getCamera());
		n5->setName("LINE5");
		geode->addDrawable(n5);
		g_hidden_line_geode->addDrawable(n5);

		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(-100, "RenderBin"); //面
	}


#if 0
	vector<int> index1(100);
	index1[1] = 1;
	index1[2] = 0;
	index1[3] = 4;
	index1[4] = 7;

	vector<int> index2(100);
	//id == 1
	index2[1] = 3;
	index2[2] = 4;
	index2[3] = 0;

	//id == 3
	index2[4] = 1;
	index2[5] = 4;
	index2[6] = 0;

	//id == 4
	index2[7] = 1;
	index2[8] = 3;
	index2[9] = 0;
#else

	vector<int> index1(100);
	index1[1] = 1;
	index1[2] = 0;
	index1[3] = 3;
	index1[4] = 0;

	vector<int> index2(100);
	//id == 1
	index2[1] = 3;
	index2[2] = 0;

	//id == 3
	index2[3] = 1;
	index2[4] = 0;

#endif

	g_textureBuffer1 = create_tbo(index1);
	g_textureBuffer2 = create_tbo(index2);

	//uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
	//uniform->set(osg::Vec4(0, 1, 0, 1.));
	//n2->getOrCreateStateSet()->addUniform(uniform);

	osg::ComputeBoundsVisitor cbbv;
	geode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return root.release();
}

#endif