#pragma once
#include <osg/BoundingBox>
#include <osg/Node>
#include <memory>
#include <vector>
#include <unordered_map>
#include "DrawBoundingBox.h"

struct KdTreeNode
{
	//drawable index, [左闭右开)
	int startIndex = -1;
	int endIndex = -1;
	int level = -1;
	osg::BoundingBox bb;

	std::shared_ptr<KdTreeNode> leftNode;
	std::shared_ptr<KdTreeNode> rightNode;
};

struct Ray
{
	osg::Vec3 start;
	osg::Vec3 dir;

	float precomputedNumerator[3];
	float precomputedDenominator[3];
};

class KdTree
{
public:
	KdTree();
	~KdTree();

	void add(osg::BoundingBox& bb, osg::ref_ptr<osg::Node> node);

	void build();

	std::vector<osg::Node*> intersection(const Ray& ray);

	DrawBoundingBox drawBoundingBox_;


private:
	void buildHelper(KdTreeNode* node);
	void intersectionHelper(const Ray& ray, KdTreeNode* node, std::vector<osg::Node*> results);

	void computeBoudingBox(KdTreeNode* node);

private:
	std::shared_ptr<KdTreeNode>						 root_;

	std::vector<osg::ref_ptr<osg::Node>>			 drawables_;
	//存储单独osg 对象对应的保卫框
	std::unordered_map<osg::Node*, osg::BoundingBox> map_;
};

bool intersectionBoundingBox(const Ray& ray, const osg::BoundingBox& bb);