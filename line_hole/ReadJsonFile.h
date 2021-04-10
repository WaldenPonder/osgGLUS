#pragma once
#include <string>
#include <osg/Vec3>
#include <vector>
using namespace std;

struct LINE
{
	osg::Vec3 StartPoint;
	osg::Vec3 EndPoint;
	int Width;
};

struct ARC
{
	osg::Vec3 StartPoint;
	osg::Vec3 EndPoint;
	osg::Vec3 Center;
	int Width;
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
	vector<vector<TRIANGLE>> triangles;
};

struct MEPElement
{
	osg::Vec3 Color;
	vector<uint64_t> ConnectedElementId;
	string GUID;
	Geometrys Geometry;
	bool HasGeometry;
	uint64_t Id;
	string Name;
	bool isParticipation;
	bool isSymbol;

	enum Type
	{
		QIAOJIA = 0, //桥架
		DAOXIAN = 1, //导线
		JIDIAN_SHEBEI = 2, //机电设备
		OTHERS = 3, //其它  如 建筑、结构的构件以及机电水暖的线性构件 等
	} type;
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
	class MatrixTransform;
};

class ReadJsonFile
{
public:
	static void read(const std::string& fileName);

	static osg::MatrixTransform* createScene(ElementGroup& root);
};

