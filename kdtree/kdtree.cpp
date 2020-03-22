#include "pch.h"
#include "KdTree.h"
#include <algorithm>
#include <assert.h>

KdTree::KdTree()
{
}

KdTree::~KdTree()
{
}

void KdTree::add(osg::BoundingBox& bb, osg::ref_ptr<osg::Node> node)
{
	map_[node] = bb;
	drawables_.push_back(node);
}

void KdTree::build()
{
	root_ = std::make_shared<KdTreeNode>();
	root_->startIndex = 0, root_->endIndex = drawables_.size();
	computeBoudingBox(root_.get());
	root_->level = 0;

	buildHelper(root_.get());
}

std::vector<osg::Node*> KdTree::intersection(const Ray& ray)
{
	std::vector<osg::Node*> results;
	intersectionHelper(ray, root_.get(), results);
	return results;
}

void KdTree::buildHelper(KdTreeNode* node)
{
	if (node->endIndex - node->startIndex < 4)
		return;

	if (node->level >= 32)
		return;

	drawBoundingBox_.add(node->bb);

	float deltaX = node->bb.xMax() - node->bb.xMin();
	float deltaY = node->bb.yMax() - node->bb.yMin();
	float deltaZ = node->bb.zMax() - node->bb.zMin();

	//选择最大维度的为拆分平面
	int splitAxis = 0;
	float maxAxis = deltaX;
	if (deltaY > maxAxis) { splitAxis = 1; maxAxis = deltaY; }
	if (deltaZ > maxAxis) { splitAxis = 2; maxAxis = deltaZ; }

	int mid = (node->startIndex + node->endIndex) / 2;

	auto cmp = [this, splitAxis](osg::Node* n1, osg::Node* n2) 
	{
		osg::BoundingBox bb1 = map_[n1];
		osg::BoundingBox bb2 = map_[n2];

		return bb1.center()[splitAxis] < bb2.center()[splitAxis];
	};

	std::nth_element(drawables_.begin() + node->startIndex, drawables_.begin() + mid,
		drawables_.begin() + node->endIndex, cmp);
	
	std::vector<osg::Vec3> centers;
	for (auto& d : drawables_)
	{
		centers.push_back(map_[d].center());
	}

	std::shared_ptr<KdTreeNode> leftNode = std::make_shared<KdTreeNode>();
	node->leftNode = leftNode;
	leftNode->level = node->level + 1;
	leftNode->startIndex = node->startIndex, leftNode->endIndex = mid;
	computeBoudingBox(leftNode.get());

	std::shared_ptr<KdTreeNode> rightNode = std::make_shared<KdTreeNode>();
	node->rightNode = rightNode;
	rightNode->level = node->level + 1;
	rightNode->startIndex = mid, rightNode->endIndex = node->endIndex;
	computeBoudingBox(rightNode.get());

	buildHelper(leftNode.get());
	buildHelper(rightNode.get());
}

void KdTree::intersectionHelper(const Ray& ray, KdTreeNode* node, std::vector<osg::Node*> results)
{
	assert(node->level < 32);

	if (intersectionBoundingBox(ray, node->bb))
	{
		if (node->leftNode.get() && node->rightNode.get()) //非叶节点
		{
			intersectionHelper(ray, node->leftNode.get(), results);
			intersectionHelper(ray, node->rightNode.get(), results);
		}
		else
		{
			assert(node->endIndex - node->startIndex < 4);

			for (int i = node->startIndex; i < node->endIndex; i++)
			{
				osg::Node* n = drawables_[i];
				if (intersectionBoundingBox(ray, map_[n]))
				{
					results.push_back(n);
				}
			}
		}
	}
}

void KdTree::computeBoudingBox(KdTreeNode* node)
{
	for (int i = node->startIndex; i < node->endIndex; i++)
	{
		osg::Node* n = drawables_[i];
		node->bb.expandBy(map_[n]);
	}
}

bool intersectionBoundingBox(const Ray& ray, const osg::BoundingBox& bb)
{
	float tNear = 0, tFar = FLT_MAX;

	float slabs[3][2] = { { bb.xMin(), bb.xMax() }, { bb.yMin(), bb.yMax() }, { bb.zMin(), bb.zMax() } };

	for (uint8_t i = 0; i < 3; ++i)
	{
		float tNearExtents = (slabs[i][0] - ray.precomputedNumerator[i]) / ray.precomputedDenominator[i];
		float tFarExtents  = (slabs[i][1] - ray.precomputedNumerator[i]) / ray.precomputedDenominator[i];
		if (ray.precomputedDenominator[i] < 0) std::swap(tNearExtents, tFarExtents);
		if (tNearExtents > tNear) tNear = tNearExtents;
		if (tFarExtents < tFar) tFar = tFarExtents;
		if (tNear > tFar)
		{
			return false;
		}
	}

	return true;
}
