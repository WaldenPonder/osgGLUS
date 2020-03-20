#include "pch.h"
#include "ModelManager.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/KdTree>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/IntersectionVisitor>
#include <random>
#include "osgUtil/LineSegmentIntersector"
#include <osgDB/WriteFile>
#include <osg/io_utils>
#include <cassert>

using namespace std;
bool g_cullActive = true;
KdTree g_kdtree;

osg::Vec3 g_normals[] = { osg::X_AXIS, osg::Y_AXIS, osg::Z_AXIS };

class CameraPredrawCallback : public osg::Camera::DrawCallback
{
 public:
	CameraPredrawCallback(osg::Camera* camere, osg::Group* box,
						  std::vector<osg::ref_ptr<osg::Node>>& models, std::unordered_map<osg::Node*, unsigned int>& modelsFrame)
		: camera_(camere),
		  boundingbox_(box),
		  models_(models),
		  modelsFrame_(modelsFrame)
	{
	}

	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		if (models_.size() <= 0) return;

		if (g_cullActive == false)
		{
			for (osg::Node* n : models_)
				n->setNodeMask(~0);
			return;
		}

		assert(models_.size() == modelsFrame_.size());
		frameNumber_++;

		const int kRayCountPerFrame				= 200;
		const int kFrameNoIntersectionDisappear = 400;
				
		{
			std::default_random_engine			   generator(time(NULL));
			std::uniform_real_distribution<double> distribution(-1., 1.);
			osg::Matrix mat = osg::Matrix::inverse(camera_->getViewMatrix() * camera_->getProjectionMatrix());

			vector<osg::Vec3> starts, ends;

			clock_t t = clock();
			for (int i = 0; i < kRayCountPerFrame; i++)
			{
				double x = distribution(generator);
				double y = distribution(generator);
				starts.push_back(osg::Vec3(x, y, -1) * mat);
				ends.push_back(osg::Vec3(x, y, 1) * mat);
			}

#if 0
			osg::ref_ptr<osgUtil::IntersectorGroup> pickGroup = new osgUtil::IntersectorGroup;

			for (int i = 0; i < kRayCountPerFrame; i++)
			{
				pickGroup->addIntersector(new osgUtil::LineSegmentIntersector(starts[i], ends[i]));
			}
			
			osgUtil::IntersectionVisitor iv(pickGroup);			
			boundingbox_->accept(iv);
			OSG_WARN << "TIME: " << (clock() - t) << "\n";

			vector<osg::Node*> vv = boundingbox_->getParentalNodePaths()[0];

			int ii = 0;

			for (osg::ref_ptr<osgUtil::Intersector>& is : pickGroup->getIntersectors())
			{
				osgUtil::LineSegmentIntersector* picker = dynamic_cast<osgUtil::LineSegmentIntersector*>(is.get());
				if (picker && picker->containsIntersections())
				{
					for (osgUtil::LineSegmentIntersector::Intersection intersection : picker->getIntersections())
					{
						if (intersection.drawable->getName() == "BOUNDINGBOX_FOR_CULL")
						{
							unsigned int								  index = intersection.primitiveIndex;

							osg::ref_ptr<osg::Node>						  node = models_[index / 6];
							modelsFrame_[node.get()] = frameNumber_;
							//OSG_WARN << index << "\t" << ii++ << "\n";
						}
					}
				}
			}
#else
			for (int i = 0; i < kRayCountPerFrame; i++)
			{
				Ray ray;
				ray.start = starts[i];
				ray.dir = ends[i] - starts[i];
								
				for (int i = 0; i < 3; i++)
				{
					ray.precomputedNumerator[i] = g_normals[i] * ray.start;
					ray.precomputedDenominator[i] = g_normals[i] * ray.dir;
				}

				std::vector<osg::Node*> nodes = g_kdtree.intersection(ray);
				for (osg::Node* n : nodes)
				{
					modelsFrame_[n] = frameNumber_;
				}
			}
			OSG_WARN << "TIME: " << (clock() - t) << "\n";
#endif
		}

		int count = 0;
		for (osg::Node* n : models_)
		{
			if (!n) continue;
			unsigned int  frame = modelsFrame_[n];
			unsigned long delta = frameNumber_ - frame;

			if (delta >= kFrameNoIntersectionDisappear) count++;
			n->setNodeMask(delta > kFrameNoIntersectionDisappear ? 0 : ~0);
		}

		OSG_WARN << "CULL DRAWABLE: " << count << std::endl;
	}

	mutable unsigned int	  frameNumber_ = 0;
	osg::ref_ptr<osg::Camera> camera_;
	osg::ref_ptr<osg::Group>  boundingbox_;

	std::vector<osg::ref_ptr<osg::Node>>&		  models_;
	std::unordered_map<osg::Node*, unsigned int>& modelsFrame_;
};

ModelManager::ModelManager(osg::Camera* camere, osg::Group* parent)
{
	parent_ = parent;
	modelRoot_ = new osg::Group;
	parent->addChild(modelRoot_);

	boundingbox_ = new osg::PositionAttitudeTransform;
	parent->addChild(boundingbox_);

	camere->addPreDrawCallback(new CameraPredrawCallback(camere, boundingbox_, models_, modelsFrame_));
}

ModelManager::~ModelManager()
{
}

void ModelManager::buildBoundingbox()
{
#pragma region 
	class CollectGeometryVisitor : public osg::NodeVisitor
	{
	public:
		CollectGeometryVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
		{
		}
		~CollectGeometryVisitor()
		{
		}

		virtual void apply(osg::Geometry& geometry)
		{
			objs.push_back(&geometry);
		}

		std::vector<osg::Geometry*> objs;
	};
#pragma endregion 

	CollectGeometryVisitor cgv;
	modelRoot_->accept(cgv);

	models_.resize(cgv.objs.size());

	for (int i = 0; i < cgv.objs.size(); i++)
	{
		osg::Node*	 c	   = cgv.objs[i];
		if(!c) continue;

		models_[i]		= c;
		modelsFrame_[c] = 0;

		osg::ComputeBoundsVisitor cbv;
		c->accept(cbv);
		osg::BoundingBox bb = cbv.getBoundingBox();
		osg::BoundingBox bbWorld;

		osg::Matrix mat = c->getWorldMatrices()[0];

		for (int j = 0; j <= 7; j++)
		{
			bbWorld.expandBy(bb.corner(j) * mat);
			OSG_WARN << "PTS: " << bb.corner(j) * mat << std::endl;
		}
		bb = bbWorld;

		if(std::abs(bb.corner(i).z()) > 1e6 || std::abs(bbWorld.corner(i).z()) > 1e6) continue;

		if (!bb.valid() || !bbWorld.valid()) continue;

		g_kdtree.add(bb, c);
		drawBoundingBox_.add(bb);
	}
	
	osg::Geode* geode = drawBoundingBox_.get();
	geode->getChild(0)->setName("BOUNDINGBOX_FOR_CULL");
	boundingbox_->addChild(geode);
	
	osg::ref_ptr<osg::KdTreeBuilder> kdtreebuild = new osg::KdTreeBuilder;
	geode->accept(*kdtreebuild);

	//osgDB::writeNodeFile(*boundingbox_, "F:\\box.osg");
}
