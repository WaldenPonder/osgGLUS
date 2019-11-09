#include "osg_lib.h"
#include <iostream>

using namespace std;
#include <osg/Geode>
#include <osg/Material>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgUtil/ShaderGen>

#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>

#include <osg/ComputeBoundsVisitor>
#include <osg/CullFace>
#include <osg/Math>
#include <osg/NodeVisitor>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgSim/ShapeAttribute>
#include <osgText/Text>
#include <osgUtil/Optimizer>
#include <osgViewer/ViewerEventHandlers>
#include <osgUtil/PolytopeIntersector>
#include <osgUtil/PlaneIntersector>
#include <osg/LineWidth>
#include <osg/PrimitiveSet>
#include <osg/PrimitiveRestartIndex>
#include <osg/PositionAttitudeTransform>
#include <osg/KdTree>
#include <osg/Point>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include "event_handler.inc"

#include <stdio.h>
#include <io.h>

inline string shader_dir()
{
	if (!access("../shader", 0))
	{
		return "../shader";
	}
	else if (!access("../../shader", 0))
	{
		return "../../shader";
	}

	return "";
}

inline void configureShaders(osg::StateSet* stateSet)
{
	const std::string vertexSource =
		"#version 140 \n"
		" \n"
		"uniform mat4 osg_ModelViewProjectionMatrix; \n"
		"uniform mat3 osg_NormalMatrix; \n"
		//"uniform vec3 ecLightDir; \n"
		" \n"
		"in vec4 osg_Vertex; \n"
		"in vec3 osg_Normal; \n"
		"in vec4 osg_Color; \n"
		"out vec4 color; \n"
		" \n"
		"void main() \n"
		"{ \n"
		//"    vec3 ecNormal = normalize( osg_NormalMatrix * osg_Normal ); \n"
		//"    float diffuse = max( dot( ecLightDir, ecNormal ), 0. ); \n"
		"    color = osg_Color;//vec4( vec3( diffuse ), 1. ); \n"
		" \n"
		"    gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex; \n"
		"} \n";
	osg::Shader* vShader = new osg::Shader(osg::Shader::VERTEX, vertexSource);

	const std::string fragmentSource =
		"#version 140 \n"
		" \n"
		"in vec4 color; \n"
		"out vec4 fragData; \n"
		" \n"
		"void main() \n"
		"{ \n"
		"    fragData = color; \n"
		"} \n";
	osg::Shader* fShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentSource);

	osg::Program* program = new osg::Program;
	program->addShader(vShader);
	program->addShader(fShader);
	stateSet->setAttribute(program);

	//osg::Vec3f lightDir(0., 0.5, 1.);
	//lightDir.normalize();
	//stateSet->addUniform(new osg::Uniform("ecLightDir", lightDir));
}

inline osg::Geometry* createLine(const std::vector<osg::Vec3d>& allPTs, osg::Vec4 color, osg::PrimitiveSet::Mode model, float nWidth = 2)
{
	int nCount = allPTs.size();

	osg::ref_ptr<osg::Geometry> pGeometry = new osg::Geometry();

	osg::ref_ptr<osg::Vec3Array> pyramidVertices = new osg::Vec3Array;
	pGeometry->setVertexArray(pyramidVertices.get());
	for (int i = 0; i < allPTs.size(); i++)
	{
		pyramidVertices->push_back(allPTs[i]);
	}

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(color);
	pGeometry->setColorArray(colors.get());
	pGeometry->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::DrawArrays> pyramidBase = NULL;
	pyramidBase = new osg::DrawArrays(model, 0, allPTs.size());
	pGeometry->addPrimitiveSet(pyramidBase.get());

	//osgUtil::SmoothingVisitor::smooth(*pGeometry);

	pGeometry->setUseVertexBufferObjects(true);

	{
		osg::StateSet* stateset = pGeometry->getOrCreateStateSet();
		stateset->setAttributeAndModes(new osg::LineWidth(nWidth), osg::StateAttribute::ON);
		stateset->setAttributeAndModes(new osg::Point(3));
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
	}

	return pGeometry.release();
}

