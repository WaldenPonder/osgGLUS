
void test1(osg::Group* root, osg::Node* node)
{
	int N = 1;
	for (int i = 0; i < N; i++)
	{
		float offset = 1e-3;

		const char* VS =
			"#version 330 compatibility \n"
			"uniform float outlineWidth; \n"
			"void main()"
			"{ \n"
			"    vec4 clip = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
			"    vec3 normal = normalize(gl_NormalMatrix * gl_Normal); \n"
			"    normal.x *= gl_ProjectionMatrix[0][0]; \n"
			"    normal.y *= gl_ProjectionMatrix[1][1]; \n"
			"    clip.xy += normal.xy * clip.w * outlineWidth;\n"
			"    gl_Position = clip; \n"
			"} \n";

		std::stringstream fs_;
		fs_ << "#version 330\n"
			"void main() \n"
			"{ \n"
			<< "float a = " << ((1.f / N) * (N - i)) << " ;\n"
			<< " gl_FragColor = vec4(1,0,0, 1);\n"
			<< "}\n";

		string		ss = fs_.str();
		const char* FS = ss.c_str();

		osg::Program* program = new osg::Program();
		program->addShader(new osg::Shader(osg::Shader::VERTEX, VS));
		program->addShader(new osg::Shader(osg::Shader::FRAGMENT, FS));

		osg::AutoTransform* at = new osg::AutoTransform;
		at->setAutoRotateMode(osg::AutoTransform::NO_ROTATION);
		root->addChild(at);

		osg::Group* outline = new osg::Group();
		{
			osg::Program* program = new osg::Program();
			program->addShader(new osg::Shader(osg::Shader::VERTEX, VS));
			program->addShader(new osg::Shader(osg::Shader::FRAGMENT, FS));

			osg::StateSet* outlineSS = outline->getOrCreateStateSet();
			outlineSS->setRenderBinDetails(1, "RenderBin");
			outlineSS->setAttributeAndModes(
				new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
			outlineSS->setMode(GL_CULL_FACE, 0);
			outlineSS->setAttribute(program);
			outlineSS->addUniform(new osg::Uniform("outlineWidth", offset));
			outlineSS->setAttributeAndModes(new osg::LineWidth(2),
				osg::StateAttribute::ON);
			outline->addChild(node);

			outlineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			outlineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
			// outlineSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			outlineSS->setAttributeAndModes(
				new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
					osg::BlendFunc::ONE_MINUS_SRC_ALPHA),
				osg::StateAttribute::ON);
		}

		osg::Group* outline2 = new osg::Group();
		{
			osg::StateSet* outlineSS = outline2->getOrCreateStateSet();
			outlineSS->setRenderBinDetails(1, "RenderBin");
			outlineSS->setAttributeAndModes(
				new osg::Depth(osg::Depth::LESS, 0.0, 1.0, false));
			outlineSS->setMode(GL_CULL_FACE, 0);
			outlineSS->setAttribute(program);
			outlineSS->addUniform(new osg::Uniform("outlineWidth", -offset));
			outlineSS->setAttributeAndModes(new osg::LineWidth(2),
				osg::StateAttribute::ON);
			outline2->addChild(node);

			outlineSS->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
			outlineSS->setMode(GL_BLEND, osg::StateAttribute::ON);
			// outlineSS->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			outlineSS->setAttributeAndModes(
				new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
					osg::BlendFunc::ONE_MINUS_SRC_ALPHA),
				osg::StateAttribute::ON);
		}

		at->addChild(outline);
		at->addChild(outline2);
	}

	osg::Group* model = new osg::Group();
	model->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");
	model->addChild(node);
	root->addChild(model);
}
