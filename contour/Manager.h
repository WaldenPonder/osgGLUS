#pragma once
#include <vector>
#include <osg/Group>
using namespace std;

class Manager
{
public:
	Manager(osg::Group* root, int row, int col);
	~Manager();

	void build();

	osg::Group* root_;
	std::vector<osg::ref_ptr<osg::Node>> nodes_;
	const int row_, col_;
	std::vector<std::vector<osg::Group*>> tiles_;
	std::vector<std::vector<osg::Group*>> tiles2_;
};

