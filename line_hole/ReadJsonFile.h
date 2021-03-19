#pragma once
#include <string>
#include <osg/Vec3>
#include <vector>
using namespace std;

struct LINE
{
	osg::Vec3 StartPoint;
	osg::Vec3 EndPoint;
};

struct ARC
{
	osg::Vec3 StartPoint;
	osg::Vec3 EndPoint;
	osg::Vec3 Center;
};

struct TRIANGLE
{
	osg::Vec3 FirstPoint;
	osg::Vec3 SecondPoint;
	osg::Vec3 ThirdPoint;
};

struct Geometrys
{
	vector<LINE> lines;
	vector<ARC> arcs;
	vector<TRIANGLE> triangles;
};

struct MEPElement
{
	osg::Vec3 Color;
	string ConnectedElementId;
	string GUID;
	Geometrys Geometry;
	bool HasGeometry;
	string Id;
	string Name;
};

struct ElementGroup
{
	string DetailLevel;
	vector<MEPElement> MEPElements;
};

extern ElementGroup g_elementRoot;

namespace osg
{
	class Node;
};

class ReadJsonFile
{
public:
	static void read(const std::string& fileName);

	static osg::Node* createScene(const ElementGroup& root);
};

