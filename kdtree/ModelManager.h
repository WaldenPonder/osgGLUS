#pragma once
#include <osg/Camera>
#include <osg/PositionAttitudeTransform>
#include <unordered_map>
#include <vector>

class ModelManager : public osg::Referenced
{
 public:
	ModelManager(osg::Camera* camere, osg::Group* parent);
	~ModelManager();
	osg::Group* getModelRoot() const { return modelRoot_; }
	void buildBoundingbox();

 private:
	osg::ref_ptr<osg::Group> modelRoot_;
	std::vector<osg::ref_ptr<osg::Node>> models_;
	std::unordered_map<osg::Node*, unsigned int> modelsFrame_;
	osg::ref_ptr<osg::PositionAttitudeTransform> boundingbox_;
};

extern bool g_cullActive;
extern bool g_showBoundingbox;