#pragma once
#include <osg/Camera>
#include <osg/PositionAttitudeTransform>
#include <unordered_map>
#include <vector>
#include "DrawBoundingBox.h"
#include "KdTree.h"

#define NM_NO_SHOW 1
#define NM_ALL ~0

extern KdTree g_kdtree;

class ModelManager : public osg::Referenced
{
 public:
	ModelManager(osg::Camera* camere, osg::Group* parent);
	~ModelManager();

	osg::Group* getModelRoot() const { return modelRoot_; }
	osg::Group* getBoundingbox() const { return boundingbox_; }

	void buildBoundingbox();

public:
	DrawBoundingBox drawBoundingBox_;
	
 private:
	osg::ref_ptr<osg::Group>					 parent_;
	osg::ref_ptr<osg::Group>					 modelRoot_;
	std::vector<osg::ref_ptr<osg::Node>>		 models_;
	std::unordered_map<osg::Node*, unsigned int> modelsFrame_;
	osg::ref_ptr<osg::PositionAttitudeTransform> boundingbox_;
};

extern bool g_cullActive;
extern bool g_showBoundingbox;