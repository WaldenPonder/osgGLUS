#pragma once
#include <vector>
#include <osg/vec3>
#include <osg/vec3d>
#include <osg/vec4>
#include <osg/Object>
#include <osg/Matrixd>
#include <osgViewer/View>
//class CViewControlData;

namespace osg
{
	class Node;
};

class HighlightSystem
{
public:
	HighlightSystem(osgViewer::View* pOSG);
	~HighlightSystem();

	void addHighlight(osg::Node* pNode);
	void removeHighlight(osg::Node* pNode);
	bool isHighlight(osg::Node* pNode);
	void enable(bool flag);
	void updateViewmatrix(const osg::Matrixd& mat);
	void updateProjectionMatrix(const osg::Matrixd& mat);

	//第四位表示透明度
	void setColor(const osg::Vec4& color);
	//轮廓检测系数
	void setOutLineFactor(float f = 0.005);

	void reset();

private:
	struct Impl;
	Impl* impl;
};

