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
#include "osg/PositionAttitudeTransform"
#include <osg/KdTree>


#define  SOLUTION_DIR std::string("D:\\GraphProject\\osgGLUS\\")

#include "event_handler.inc"