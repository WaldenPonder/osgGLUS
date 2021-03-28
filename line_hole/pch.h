// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H
#include "windows.h"
// TODO: 添加要在此处预编译的标头

#endif //PCH_H






#if 0
// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(osg::Vec2 p, osg::Vec2 q, osg::Vec2 r)
{
	if (q.x() <= max(p.x(), r.x()) && q.x() >= min(p.x(), r.x()) &&
		q.y() <= max(p.y(), r.y()) && q.y() >= min(p.y(), r.y()))
		return true;
	return false;
}
//https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(osg::Vec2 p, osg::Vec2 q, osg::Vec2 r) {
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
	// for details of below formula.
	float val1 = (q.y() - p.y()) * (r.x() - q.x());

	float val2 = (q.x() - p.x()) * (r.y() - q.y());


	// if (all(equal(vec2(val1 - val2, val2 - val1), vec2(0))))
		// return 0;  // colinear

	if (abs(val1 - val2) < 0.0001) return 0;

	return (val1 - val2 > 0) ? 1 : 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(osg::Vec2 p1, osg::Vec2 q1,
	osg::Vec2 p2, osg::Vec2 q2)
{
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
	{
		if (o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0) //点在线上不认为相交
			return true;
	}

#if 1
	int count = 0;
	if (o1 == 0 && o2 == 0 && o3 == 0 && o4 == 0)
	{
		//点在线上不认为相交
// Special Cases
// p1, q1 and p2 are colinear and p2 lies on segment p1q1
		if (o1 == 0 && onSegment(p1, p2, q1)) count++;

		// p1, q1 and q2 are colinear and q2 lies on segment p1q1
		if (o2 == 0 && onSegment(p1, q2, q1)) count++;

		// p2, q2 and p1 are colinear and p1 lies on segment p2q2
		if (o3 == 0 && onSegment(p2, p1, q2)) count++;

		// p2, q2 and q1 are colinear and q1 lies on segment p2q2
		if (o4 == 0 && onSegment(p2, q1, q2)) count++;
	}

	if (count >= 2) return true;
#endif

	return false; // Doesn't fall in any of the above cases
}


void main()
{
	bool flag = doIntersect(
		osg::Vec2(-1, 0), osg::Vec2(1, 0),
		osg::Vec2(0, -1), osg::Vec2(0, 1));

	bool flag2 = doIntersect(
		osg::Vec2(-1, 0), osg::Vec2(1, 0),
		osg::Vec2(0, 0), osg::Vec2(0, 1));

	bool flag3 = doIntersect(
		osg::Vec2(-1, 0), osg::Vec2(1, 0),
		osg::Vec2(0, 0), osg::Vec2(2, 0));
	getchar();

}
#endif



#if 0
osg::Node* LineHole::create_lines0(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	g_hidden_line_geode = new osg::Geode;
	root->addChild(geode);
	root->addChild(g_hidden_line_geode);
	vector<osg::Vec3>		 PTs, COLORs;

	setUpStateset(geode->getOrCreateStateSet(), view.getCamera());
	setUpHiddenLineStateset(g_hidden_line_geode->getOrCreateStateSet(), view.getCamera());

	float z;
	//-------------------------------------------------
	osg::Geode* geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1.5, -1.5, -0.5), osg::Vec3(3, 0, 0), osg::Vec3(0, 3, 0));
		geode2->addChild(geom);
		root->addChild(geode2);

		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(1, "RenderBin"); //面

		PTs.clear();
		z = -0.5;
		PTs.push_back(osg::Vec3(-1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, -1.5, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1.5, 1.5, z));

		osg::Geometry* n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { 1 }, view.getCamera());
		g_hidden_line_geode->addDrawable(n4);
		ss = n4->getOrCreateStateSet();
		ss->setRenderBinDetails(20, "RenderBin"); //虚线

		n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { 1 }, view.getCamera());
		n4->setName("LINE4");
		geode->addDrawable(n4);
		ss = n4->getOrCreateStateSet();
		ss->setRenderBinDetails(3, "RenderBin"); //实线
	}


	//-------------------------------------------------
	geode2 = new osg::Geode;
	{
		auto geom = myCreateTexturedQuadGeometry2(view.getCamera(), 0, osg::Vec3(-1, -1, 0), osg::Vec3(2.5, 0, 0), osg::Vec3(0, 2.5, 0));
		geode2->addChild(geom);
		root->addChild(geode2);
		//越小越先画，默认0, 面要最先画, 虚线第二画， 实体线最后画
		osg::StateSet* ss = geom->getOrCreateStateSet();
		ss->setRenderBinDetails(1, "RenderBin"); //面

		PTs.clear();
		z = 0;
		PTs.push_back(osg::Vec3(-1, -1, z));
		PTs.push_back(osg::Vec3(1.5, -1, z));
		PTs.push_back(osg::Vec3(1.5, 1.5, z));
		PTs.push_back(osg::Vec3(-1, 1.5, z));

		osg::Geometry* n5 = createLine2(PTs, { osg::Vec4(1, 1, 1, 1) }, { 2 }, view.getCamera());
		g_hidden_line_geode->addDrawable(n5);
		ss = n5->getOrCreateStateSet();
		ss->setRenderBinDetails(20, "RenderBin"); //虚线

		n5 = createLine2(PTs, { osg::Vec4(1, 1, 1, 1) }, { 2 }, view.getCamera());
		n5->setName("LINE5");
		geode->addDrawable(n5);

		ss = n5->getOrCreateStateSet();
		ss->setRenderBinDetails(3, "RenderBin"); //实线
	}

	vector<int> index1(100);
	vector<int> index2(100);
	g_textureBuffer1 = create_tbo(index1);
	g_textureBuffer2 = create_tbo(index2);

	osg::ComputeBoundsVisitor cbbv;
	root->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return root.release();
}

osg::Node* LineHole::create_lines1(osgViewer::Viewer& view)
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<float> rd(0, 1);

	osg::ref_ptr<osg::Group> root = new osg::Group;
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	root->addChild(geode);
	vector<osg::Vec3>		 PTs, COLORs;

	setUpStateset(geode->getOrCreateStateSet(), view.getCamera());

	float z = 0.5f;
	PTs.push_back(osg::Vec3(2, 0, z));
	PTs.push_back(osg::Vec3(-2, 0, z));
	osg::Geometry* n = createLine2(PTs, { osg::Vec4(1, 0, 0, 1) }, { 1 }, view.getCamera());
	n->setName("LINE1");
	geode->addDrawable(n);

	PTs.clear();
	z = 0.35f;
	PTs.push_back(osg::Vec3(0, 2, z));
	PTs.push_back(osg::Vec3(0, -2, z));
	n = createLine2(PTs, { osg::Vec4(1, 1, 0, 1) }, { 4 }, view.getCamera());
	n->setName("LINE2");
	geode->addDrawable(n);

	PTs.clear();
	z = -0.5;
	PTs.push_back(osg::Vec3(-1.5, -1.5, z));
	PTs.push_back(osg::Vec3(1.5, -1.5, z));
	PTs.push_back(osg::Vec3(1.5, 1.5, z));
	PTs.push_back(osg::Vec3(-1.5, 1.5, z));

	osg::Geometry* n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { 3 }, view.getCamera());
	n4->setName("LINE4");
	geode->addDrawable(n4);

	PTs.clear();
	z = 0;
	PTs.push_back(osg::Vec3(-1, -1, z));
	PTs.push_back(osg::Vec3(1.5, -1, z));
	PTs.push_back(osg::Vec3(1.5, 1.5, z));
	PTs.push_back(osg::Vec3(-1, 1.5, z));
	osg::Geometry* n5 = createLine2(PTs, { osg::Vec4(0, 1, 1, 1) }, { 888 }, view.getCamera());
	n5->setName("LINE5");
	geode->addDrawable(n5);

#if 0
	vector<int> index1(100);
	index1[1] = 1;
	index1[2] = 0;
	index1[3] = 4;
	index1[4] = 7;

	vector<int> index2(100);
	//id == 1
	index2[1] = 3;
	index2[2] = 4;
	index2[3] = 0;

	//id == 3
	index2[4] = 1;
	index2[5] = 4;
	index2[6] = 0;

	//id == 4
	index2[7] = 1;
	index2[8] = 3;
	index2[9] = 0;
#else

	vector<int> index1(100);
	index1[1] = 1;
	index1[2] = 0;
	index1[3] = 3;
	index1[4] = 0;

	vector<int> index2(100);
	//id == 1
	index2[1] = 3;
	index2[2] = 0;

	//id == 3
	index2[3] = 1;
	index2[4] = 0;

#endif

	g_textureBuffer1 = create_tbo(index1);
	g_textureBuffer2 = create_tbo(index2);

	osg::ComputeBoundsVisitor cbbv;
	geode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return root.release();
}

osg::Node* LineHole::create_lines2(osgViewer::Viewer& view)
{
	osg::ref_ptr<osg::Group> root = new osg::Group;

	g_hidden_line_geode = new osg::Geode;
	root->addChild(g_hidden_line_geode);
	vector<osg::Vec3>		 PTs, COLORs;

	setUpHiddenLineStateset(g_hidden_line_geode->getOrCreateStateSet(), view.getCamera());
	float z;
	int id = 1;
	for (int i = 0; i < 10; i++)
	{
		for (int s = 1; s < 10; s++)
		{
			osg::ref_ptr<osg::Geode> geode = new osg::Geode;
			root->addChild(geode);
			vector<osg::Vec3>		 PTs, COLORs;

			setUpStateset(geode->getOrCreateStateSet(), view.getCamera());


			setUpStateset(geode->getOrCreateStateSet(), view.getCamera());

			float z = 0.5 + i + s;
			PTs.push_back(osg::Vec3(2 * s, 0, z));
			PTs.push_back(osg::Vec3(-2 * s, 0, z));
			osg::Geometry* n = createLine2(PTs, { osg::Vec4(1, 0, 0, 1) }, { id++ }, view.getCamera());
			n->setName("LINE1");
			geode->addDrawable(n);

			PTs.clear();
			z = 0.35f;
			PTs.push_back(osg::Vec3(0, 2 * s, z));
			PTs.push_back(osg::Vec3(0, -2 * s, z));
			n = createLine2(PTs, { osg::Vec4(1, 1, 0, 1) }, { id++ }, view.getCamera());
			n->setName("LINE2");
			geode->addDrawable(n);

			PTs.clear();
			z = -0.5 + 1 + i + s;
			PTs.push_back(osg::Vec3(-1.5 * s, -1.5 * s, z));
			PTs.push_back(osg::Vec3(1.5 * s, -1.5 * s, z));
			PTs.push_back(osg::Vec3(1.5 * s, 1.5 * s, z));
			PTs.push_back(osg::Vec3(-1.5 * s, 1.5 * s, z));

			osg::Geometry* n4 = createLine2(PTs, { osg::Vec4(0, 0, 1, 1) }, { id++ }, view.getCamera());
			n4->setName("LINE4");
			geode->addDrawable(n4);

			PTs.clear();
			z = i + s;
			PTs.push_back(osg::Vec3(-1 * s, -1 * s, z));
			PTs.push_back(osg::Vec3(1.5 * s, -1 * s, z));
			PTs.push_back(osg::Vec3(1.5 * s, 1.5 * s, z));
			PTs.push_back(osg::Vec3(-1 * s, 1.5 * s, z));
			osg::Geometry* n5 = createLine2(PTs, { osg::Vec4(0, 1, 1, 1) }, { id++ }, view.getCamera());
			n5->setName("LINE5");
			geode->addDrawable(n5);
		}
	}

	vector<int> index1(100);
	vector<int> index2(100);
	g_textureBuffer1 = create_tbo(index1);
	g_textureBuffer2 = create_tbo(index2);

	osg::ComputeBoundsVisitor cbbv;
	root->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return root.release();
}

#endif
