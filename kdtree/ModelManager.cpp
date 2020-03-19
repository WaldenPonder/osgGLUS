#include "stdafx.h"
#include "ModelManager.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/KdTree>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/IntersectionVisitor>
#include <random>
#include "osgUtil/LineSegmentIntersector"
#include "Db3DModeling/DB/DbBaseDefine.h"
#include "Db3DModeling/DB/DbGlobal.h"
#include <osgDB/WriteFile>
#include <osg/io_utils>

bool g_cullActive = true;
bool g_showBoundingbox = true;

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
		boundingbox_->setNodeMask(g_showBoundingbox ? NM_ALL : NM_NO_SHOW);

		if (models_.size() <= 0) return;

		if (g_cullActive == false)
		{
			for (osg::Node* n : models_)
				n->setNodeMask(NM_ALL);
			return;
		}

		assert(models_.size() == modelsFrame_.size());
		frameNumber_++;

		const int kRayCountPerFrame				= 200;
		const int kFrameNoIntersectionDisappear = 400;

		osg::ref_ptr<osgUtil::IntersectorGroup> pickGroup = new osgUtil::IntersectorGroup;
		{
			std::default_random_engine			   generator(time(NULL));
			std::uniform_real_distribution<double> distribution(-1., 1.);
			osg::Matrix mat = osg::Matrix::inverse(camera_->getViewMatrix() * camera_->getProjectionMatrix());

			clock_t t = clock();
			for (int i = 0; i < kRayCountPerFrame; i++)
			{
				double x = distribution(generator);
				double y = distribution(generator);
				osg::Vec3 start = osg::Vec3(x, y, -1) * mat;
				osg::Vec3 end = osg::Vec3(x, y, 1) * mat;
				pickGroup->addIntersector(new osgUtil::LineSegmentIntersector(start, end));
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
		}

		int count = 0;
		for (osg::Node* n : models_)
		{
			if (!n) continue;
			unsigned int  frame = modelsFrame_[n];
			unsigned long delta = frameNumber_ - frame;

			if (delta >= kFrameNoIntersectionDisappear) count++;
			n->setNodeMask(delta > kFrameNoIntersectionDisappear ? 0 : NM_ALL);
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
	modelRoot_ = new osg::Group;
	modelRoot_->setNodeMask(NM_PLACEHOLDER);
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

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setName("BOUNDINGBOX_FOR_CULL");
	geode->addDrawable(geometry);
	boundingbox_->addChild(geode);

	osg::ref_ptr<osg::Vec3Array> vert = new osg::Vec3Array;
	geometry->setVertexArray(vert);
	osg::ref_ptr<osg::Vec4Array> color = new osg::Vec4Array;
	geometry->setColorArray(color, osg::Array::BIND_PER_VERTEX);

	std::default_random_engine			  eng(time(NULL));
	std::uniform_real_distribution<float> urd(0, 1);

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

		float xMin = bb.xMin(), xMax = bb.xMax();
		float yMin = bb.yMin(), yMax = bb.yMax();
		float zMin = bb.zMin(), zMax = bb.zMax();

		//OSG_WARN << "DELTA: " << (xMax - xMin) << "\t" << (yMax - yMin) << "\t" << (zMax - zMin) << "\n";

		float r = urd(eng), g = urd(eng), b = urd(eng);
		for (int j = 0; j < 24; j++)
		{
			color->push_back(osg::Vec4(r, g, b, 1));
		}

		//front
		vert->push_back(osg::Vec3(xMin, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMin, yMin, zMax));

		//back
		vert->push_back(osg::Vec3(xMin, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));

		//left
		vert->push_back(osg::Vec3(xMin, yMin, zMin));
		vert->push_back(osg::Vec3(xMin, yMax, zMin));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMin, zMax));

		//right
		vert->push_back(osg::Vec3(xMax, yMin, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMin));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));

		//top
		vert->push_back(osg::Vec3(xMin, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));

		//bottom
		vert->push_back(osg::Vec3(xMin, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMin, zMax));
		vert->push_back(osg::Vec3(xMax, yMax, zMax));
		vert->push_back(osg::Vec3(xMin, yMax, zMax));
	}

	geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vert->size()));

	osg::ref_ptr<osg::PolygonMode> model = new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
	geode->getOrCreateStateSet()->setAttributeAndModes(model);

	osg::ref_ptr<osg::KdTreeBuilder> kdtreebuild = new osg::KdTreeBuilder;
	geode->accept(*kdtreebuild);

	osgDB::writeNodeFile(*boundingbox_, "F:\\box.osg");
}
