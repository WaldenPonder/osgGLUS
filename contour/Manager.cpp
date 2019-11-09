#include "pch.h"
#include "Manager.h"
#include <osgSim/Impostor>
#include <osgSim/InsertImpostorsVisitor>
#include <osg/ComputeBoundsVisitor>
#include <osg/io_utils>
#include <iostream>

Manager::Manager(osg::Group* root, int row, int col) : root_(root), row_(row), col_(col)
{
	tiles_.resize(row, std::vector<osg::Group*>(col));
	tiles2_.resize(row, std::vector<osg::Group*>(col));

	for (int r = 0; r < row; r++)
	{
		for (int c = 0; c < col; c++)
		{
			osg::Group* n = new osg::Group;
			tiles_[r][c]  = n;

			osg::Group* n2 = new osg::Group;
			tiles2_[r][c] = n2;
		}
	}
}

Manager::~Manager()
{
}

void Manager::build()
{
	osg::BoundingBox bbRoot;

	for (int i = 0; i < nodes_.size(); i++)
	{
		osg::Node*			n  = nodes_[i];
		osg::BoundingSphere bs = n->computeBound();
		if (bs.valid())
			bbRoot.expandBy(bs);
	}

	const float width  = bbRoot.xMax() - bbRoot.xMin();
	const float height = bbRoot.yMax() - bbRoot.yMin();

	for (int i = 0; i < nodes_.size(); i++)
	{
		osg::Node* n = nodes_[i];

		osg::BoundingSphere bs = n->computeBound();
		if (!bs.valid())
		{
			continue;
		}

		osg::Vec3 center = n->computeBound().center();
		//cout << center << endl;

		const float w = center.x() - bbRoot.xMin();
		const float h = center.y() - bbRoot.yMin();
		int			r = float(row_ * w) / width;
		int			c = float(col_ * h) / height;
		tiles_[r][c]->addChild(n);
	}

	for (int r = 0; r < row_; r++)
	{
		for (int c = 0; c < col_; c++)
		{
			osg::Group* tile = tiles_[r][c];
						
			for (int i = 0; i < tile->getNumChildren() / 3; i++)
			{
				tiles2_[r][c]->addChild(tile->getChild(i));
			}
		}
	}


	//--------------------´´½¨osgSim::Impostor-----------------------------
	for (int r = 0; r < row_; r++)
	{
		for (int c = 0; c < col_; c++)
		{
			osg::Group* tile = tiles_[r][c];

			osgSim::Impostor* impostor = new osgSim::Impostor;
			root_->addChild(impostor);
			impostor->addChild(tile);
			impostor->addChild(tiles2_[r][c]);

			impostor->setRange(0, 0.0f, 1e3f);
			impostor->setRange(1, 1e3f, 1e7f);
			impostor->setCenter(tile->getBound().center());

			// impostor specific settings.
			impostor->setImpostorThresholdToBound(5.0f);
		}
	}

	// now insert impostors in the model using the InsertImpostorsVisitor.
	osgSim::InsertImpostorsVisitor ov;

	// traverse the model and collect all osg::Group's and osg::LOD's.
	// however, don't traverse the rootnode since we want to keep it as
	// the start of traversal, otherwise the insertImpostor could insert
	// and Impostor above the current root, making it nolonger a root!
	root_->accept(ov);

	// insert the Impostors above groups and LOD's
	ov.insertImpostors();
}
