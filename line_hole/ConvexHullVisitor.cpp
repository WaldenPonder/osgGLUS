#include "pch.h"
#include "ConvexHullVisitor.h"
#include "LineHole.h"

osg::ref_ptr<osg::Group> g_convexRoot;

osg::Vec3 s_minPt;
//��������С��0˵��p1��p2����ʱ�뷽��(�ұ�)����p0p1�ļ��Ǵ���p0p2�ļ���
double cross_product(const osg::Vec3& p0, const osg::Vec3& p1, const osg::Vec3& p2)
{
	return (p1.x() - p0.x()) * (p2.y() - p0.y()) - (p2.x() - p0.x()) * (p1.y() - p0.y());
}

//�������
double dis(const osg::Vec3& p1, const osg::Vec3& p2)
{
	return sqrt((p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y()));
}

bool com(const osg::Vec3& p1, const osg::Vec3& p2)
{
	double temp = cross_product(s_minPt, p1, p2);
	if (fabs(temp) < 1e-6)//������Ȱ��վ����С��������
	{
		return dis(s_minPt, p1) < dis(s_minPt, p2);
	}
	else
	{
		return temp > 0;
	}
}

vector<osg::Vec3> graham_scan(vector<osg::Vec3>& p)
{
	vector<osg::Vec3> ch;
	int top = 2;
	int index = 0;
	for (int i = 1; i < p.size(); ++i)//ѡ��Y������С�ĵ㣬��Y������ȣ�ѡ��X����С�ĵ�
	{
		if (p[i].y() < p[index].y() || (p[i].y() == p[index].y() && p[i].x() < p[index].x()))
		{
			index = i;
		}
	}
	swap(p[0], p[index]);
	s_minPt = p.front();

	ch.push_back(p[0]);
	//����������
	sort(p.begin() + 1, p.end(), com);
	ch.push_back(p[1]);
	ch.push_back(p[2]);
	for (int i = 3; i < p.size(); ++i)
	{
		while (top > 0 && cross_product(ch[top - 1], p[i], ch[top]) >= 0)
		{
			--top;
			ch.pop_back();
		}
		ch.push_back(p[i]);
		++top;
	}
	return ch;
}

ConvexHullVisitor::ConvexHullVisitor()
	: osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
}

void ConvexHullVisitor::apply(osg::Geode& geode)
{
	if (geode.getNodeMask() != NM_FACE)
		return;

	if (geode.getNumChildren() == 0)
		return;

	osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode.getDrawable(0));

	if (!geometry || geometry->getNodeMask() != NM_FACE) return;

	int id = 0;
	bool flag = geometry->getUserValue("ID", id);

	osg::Vec3Array* ptOrig = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());

	if (!ptOrig || ptOrig->asVector().size() < 3)
		return;

	if (!g_convexRoot)
		g_convexRoot = new osg::Group;

	{
		osg::ref_ptr<osg::Vec3Array> tempPts = new osg::Vec3Array;

		std::vector<osg::Vec3> allPTs;

		if (g_is_top_view)
		{
			tempPts->asVector() = ptOrig->asVector();
			allPTs = graham_scan(tempPts->asVector());
			for (auto& pt : allPTs)
				pt.z() = 10000;
		}
		else
		{
			tempPts->asVector() = ptOrig->asVector();
			for (auto& pt : tempPts->asVector())
			{
				pt = osg::Vec3(pt.x(), pt.z(), pt.y());
			}
			allPTs = graham_scan(tempPts->asVector());
			for (auto& pt : allPTs)
				pt = osg::Vec3(pt.x(), 10000, pt.y());
		}

		if (allPTs.size() < 2) return;

		std::vector<osg::Vec3> pts;

		for (int i = 1; i < allPTs.size(); i++)
		{
			pts.push_back(allPTs[i - 1]);
			pts.push_back(allPTs[i]);
		}

		pts.push_back(allPTs.back());
		pts.push_back(allPTs.front());

		osg::Geometry* geometry = LineHole::createLine2(pts, { osg::Vec4(1, 1, 0, 1) }, { id }, g_viewer->getCamera(), osg::PrimitiveSet::LINES);
		osg::Geode* geode = new osg::Geode;
		geode->setNodeMask(NM_OUT_LINE);
		geode->addDrawable(geometry);
		g_convexRoot->addChild(geode);
		LineHole::setUpStateset(geode->getOrCreateStateSet(), g_viewer->getCamera());
		geode->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::OUT_LINE, "RenderBin"); //��
	}
}