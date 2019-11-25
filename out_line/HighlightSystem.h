#pragma once
#include "C\thCoreHeader.h"
#include "C\thCoreHeader.h"
#include <vector>
#include <osg/vec3>
#include <osg/vec3d>
#include <osg/vec4>
#include <osg/Object>
#include <osg/Matrixd>

class CViewControlData;

namespace osg
{
	class Node;
};

class TH_CORE_API HighlightSystem
{
public:
	HighlightSystem(CViewControlData* pOSG);
	~HighlightSystem();

	//����ֱ�� ���֣�isNeedEdgeDetectionΪFALSEЧ������
	void addHighlight(osg::Node* pNode, bool isNeedEdgeDetection = true);
	void removeHighlight(osg::Node* pNode);
	bool isHighlight(osg::Node* pNode);
	void enable(bool flag);
	void updateViewmatrix(const osg::Matrixd& mat);
	void updateProjectionMatrix(const osg::Matrixd& mat);

	//����λ��ʾ͸����
	void setColor(const osg::Vec4& color);
	//�������ϵ��
	void setOutLineFactor(float f = 0.005);

private:
	struct Impl;
	Impl* impl;
};

