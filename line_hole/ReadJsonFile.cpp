#include "pch.h"
#include "ReadJsonFile.h"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include "LineHole.h"
#include <unordered_map>
#include <osg/CullFace>

#pragma warning(disable: 4996)

ElementGroup g_elementRoot;
static int g_id = 1;

void parseColor(const Json::Value& value, osg::Vec3& color)
{
	color.x() = value[0].asInt();
	color.y() = value[1].asInt();
	color.z() = value[2].asInt();
	color /= (float)value[3].asInt();
}

void parsePoint(const Json::Value& value, osg::Vec3& pt)
{
	pt.x() = value[0].asFloat();
	pt.y() = value[1].asFloat();
	pt.z() = value[2].asFloat();
}

void parseGeometry(const Json::Value& value, Geometrys& geometry, int level)
{
	auto t = value.type();
	if (t == Json::arrayValue)
	{
		int size = value.size();
		for (int i = 0; i < size; i++)
		{
			parseGeometry(value[i], geometry, level + 1);
		}

		return;
	}

	string type = value["Type"].asString();
	if (type == "Line")
	{
		LINE line;
		parsePoint(value["StartPoint"], line.StartPoint);
		parsePoint(value["EndPoint"], line.EndPoint);
		line.Width = value["Width"].asInt();
		geometry.lines.push_back(line);
	}
	else if (type == "Arc")
	{
		ARC arc;
		parsePoint(value["StartPoint"], arc.StartPoint);
		parsePoint(value["EndPoint"], arc.EndPoint);
		parsePoint(value["Center"], arc.Center);
		geometry.arcs.push_back(arc);
	}
	else if (type == "Triangle")
	{
		if (geometry.triangles.size() <= level)
			geometry.triangles.resize(level + 1);

		TRIANGLE triangle;
		parsePoint(value["FirstPoint"], triangle.FirstPoint);
		parsePoint(value["SecondPoint"], triangle.SecondPoint);
		parsePoint(value["ThirdPoint"], triangle.ThirdPoint);

		geometry.triangles[level].push_back(triangle);
	}
}

void ReadJsonFile::read(const std::string& fileName)
{
	g_elementRoot.MEPElements.clear();

	Json::Reader reader;
	Json::Value root;

	ifstream in(fileName, ios::binary);

	if (!in.is_open())
	{
		cout << "Error opening file\n";
		return;
	}

	reader.parse(in, root);
	g_elementRoot.DetailLevel = root["DetailLevel"].asString();

	int size = root["MEPElements"].size();
	cout << size << "  :  MEPElements.size() \n";
	int delta = max(size / 100, 1);
	for (int i = 0; i < size; i++)
	{
		if (i % delta == 0)
			cout << i << "  --- " << size << "\n";

		MEPElement element;
		Json::Value ELEMENT = root["MEPElements"][i];

		parseColor(ELEMENT["Color"], element.Color);

		int connectedSize = ELEMENT["ConnectedElementId"].size();
		for (int j = 0; j < connectedSize; j++)
		{
			uint64_t val = ELEMENT["ConnectedElementId"][j].asUInt64();

			element.ConnectedElementId.push_back(val);
		}

		element.GUID = ELEMENT["GUID"].asString();

		parseGeometry(ELEMENT["Geometry"], element.Geometry, 0);

		element.HasGeometry = ELEMENT["HasGeometry"].asBool();
		element.Id = ELEMENT["Id"].asUInt64();
		element.Name = ELEMENT["Name"].asString();
		g_elementRoot.MEPElements.push_back(element);
	}
}

void handleWideLine111(const MEPElement& element, int id, const LINE& line, std::vector<osg::Vec3>& wideLines)
{
	osg::Quat quat1(osg::PI_2f, osg::Z_AXIS);
	osg::Quat quat2(-osg::PI_2f, osg::Z_AXIS);

	osg::Vec3 v = line.EndPoint - line.StartPoint;
	osg::Vec3 dir1 = quat1 * v;
	osg::Vec3 dir2 = quat2 * v;

	dir1.normalize();
	dir2.normalize();

	osg::Vec3 p1 = line.StartPoint + dir1 * line.Width;
	osg::Vec3 p2 = line.StartPoint + dir2 * line.Width;

	osg::Vec3 v2 = -v;
	osg::Vec3 dir3 = quat2 * v2;
	osg::Vec3 dir4 = quat1 * v2;

	dir3.normalize();
	dir4.normalize();

	osg::Vec3 p3 = line.EndPoint + dir3 * line.Width;
	osg::Vec3 p4 = line.EndPoint + dir4 * line.Width;

	wideLines.push_back(p1);
	wideLines.push_back(p3);
	wideLines.push_back(p2);

	wideLines.push_back(p2);
	wideLines.push_back(p4);
	wideLines.push_back(p3);

	//std::reverse(allPTs.begin(), allPTs.end());
	//osg::Geometry* geometry = LineHole::createTriangles(wideLines, { osg::Vec4(element.Color, 1) }, { id }, g_viewer->getCamera());
	//osgDB::writeNodeFile(*geometry, "F:\\aa.osg");
	//wideLines.push_back(geometry);
}

void handleWideLine(const MEPElement& element, int id, const LINE& line, std::vector<osg::Vec3>& wideLines)
{
	osg::Quat quat1(osg::PI_2f, osg::Z_AXIS);
	osg::Quat quat2(-osg::PI_2f, osg::Z_AXIS);

	osg::Vec3 v = line.EndPoint - line.StartPoint;
	v.normalize();

	osg::Vec3 dir1 = osg::Vec3(-v.y(), v.x(), 0);

	osg::Vec3 p1 = line.StartPoint + dir1 * line.Width;
	osg::Vec3 p2 = line.StartPoint - dir1 * line.Width;

	osg::Vec3 p3 = line.EndPoint + dir1 * line.Width;
	osg::Vec3 p4 = line.EndPoint - dir1 * line.Width;

	wideLines.push_back(p2);
	wideLines.push_back(p3);
	wideLines.push_back(p1);

	wideLines.push_back(p2);
	wideLines.push_back(p4);
	wideLines.push_back(p3);
}

osg::Group* handleGeometry(MEPElement& element, int id)
{
	std::vector<osg::Vec3> wideLines;
	osg::ref_ptr<osg::Group> root = new osg::Group;
	//-----------------------------------------------------------line
	{
		std::vector<osg::Vec3> allPTs;

		for (auto& line : element.Geometry.lines)
		{
			if (line.Width > 5)
			{
				handleWideLine(element, id, line, wideLines);
				continue;
			}
			allPTs.push_back(line.StartPoint);
			allPTs.push_back(line.EndPoint);
		}

		{
			osg::Geometry* geometry = LineHole::createLine2(allPTs, { osg::Vec4(element.Color, 1) }, { id }, g_viewer->getCamera(), osg::PrimitiveSet::LINES);
			osg::Geode* geode = new osg::Geode;
			geode->setNodeMask(g_is_daoxian_file ? NM_CABLE : NM_LINE);
			geode->addDrawable(geometry);
			root->addChild(geode);
			LineHole::setUpStateset(geode->getOrCreateStateSet(), g_viewer->getCamera());
			geode->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::LINE, "RenderBin"); //实线

			osg::Geode* geodeHidden = new osg::Geode;
			geodeHidden->setNodeMask(NM_HIDDEN_LINE);
			geodeHidden->addDrawable(geometry);
			root->addChild(geodeHidden);
			LineHole::setUpHiddenLineStateset(geodeHidden->getOrCreateStateSet(), g_viewer->getCamera());
			geodeHidden->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::DOT_LINE, "RenderBin"); //虚线
		}
	}

	//-----------------------------------------------------------arc
	{
		std::vector<osg::Vec3> allPTs;

		for (auto& arc : element.Geometry.arcs)
		{
			int r = (arc.Center - arc.EndPoint).length();
			osg::Vec3 dir = arc.StartPoint - arc.Center;
			dir.normalize();
			float start = acos(dir * osg::X_AXIS);

			dir = arc.EndPoint - arc.Center;
			dir.normalize();
			float end = acos(dir * osg::X_AXIS);
			float delta = (end - start) / 20.f;

			if (abs(end - start) < 0.001) continue;

			for (float f = start; f <= end; f += delta)
			{
				osg::Vec3 pt1 = arc.Center + osg::Vec3(cos(f), sin(f), 0) * r;

				if (f + delta <= end)
				{
					osg::Vec3 pt2 = arc.Center + osg::Vec3(cos(f + delta), sin(f + delta), 0) * r;
					allPTs.push_back(pt1);
					allPTs.push_back(pt2);
				}
			}
		}

		{
			osg::Geometry* geometry = LineHole::createLine2(allPTs, { osg::Vec4(element.Color, 1) }, { id }, g_viewer->getCamera(), osg::PrimitiveSet::LINES);
			osg::Geode* geode = new osg::Geode;
			geode->setNodeMask(g_is_daoxian_file ? NM_CABLE : NM_LINE);
			geode->addDrawable(geometry);
			root->addChild(geode);
			LineHole::setUpStateset(geode->getOrCreateStateSet(), g_viewer->getCamera());
			geode->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::LINE, "RenderBin"); //实线

			osg::Geode* geodeHidden = new osg::Geode;
			geodeHidden->setNodeMask(NM_HIDDEN_LINE);
			geodeHidden->addDrawable(geometry);
			root->addChild(geodeHidden);
			LineHole::setUpHiddenLineStateset(geodeHidden->getOrCreateStateSet(), g_viewer->getCamera());
			geodeHidden->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::DOT_LINE, "RenderBin"); //虚线
		}
	}

	if (wideLines.size() && element.Geometry.triangles.size() == 0)
	{
		element.Geometry.triangles.resize(1);
	}

	for (int i = 0; i + 2 < wideLines.size(); i += 3)
	{
		TRIANGLE tri;
		tri.FirstPoint = wideLines[i];
		tri.SecondPoint = wideLines[i + 1];
		tri.ThirdPoint = wideLines[i + 2];
		element.Geometry.triangles[0].push_back(tri);
	}

	
	//-----------------------------------------------------------triangle
	for (auto& triangleArr : element.Geometry.triangles)
	{
		std::vector<osg::Vec3> allPTs;
		for (auto& triangle : triangleArr)
		{
			allPTs.push_back(triangle.FirstPoint);
			allPTs.push_back(triangle.SecondPoint);
			allPTs.push_back(triangle.ThirdPoint);
		}

		//osg::Geometry* geometry = LineHole::createTriangles(allPTs, { osg::Vec4(184.0 / 255, 213. / 255., 220.0 / 255, 1) }, { id }, g_viewer->getCamera());
		osg::Geometry* geometry = LineHole::createTriangles(allPTs, { osg::Vec4(element.Color, 1) }, { id }, g_viewer->getCamera());
		osg::Geode* geode = new osg::Geode;
		geode->setNodeMask(g_is_daoxian_file ? NM_QIAOJIA_JIDIANSHEBEI : NM_FACE);
		geode->addDrawable(geometry);
		root->addChild(geode);
		geometry->setNodeMask(g_is_daoxian_file ? NM_QIAOJIA_JIDIANSHEBEI : NM_FACE);

		//越小越先画，默认0, 面要最先画,  实体线第二   虚线最后
		geode->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::FACE, "RenderBin"); //面
		LineHole::setUpStateset(geode->getOrCreateStateSet(), g_viewer->getCamera(), false);
		geode->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
	}

	return root.release();
}

osg::MatrixTransform* ReadJsonFile::createScene(ElementGroup& root)
{
	osg::ref_ptr<osg::MatrixTransform> rootNode = new osg::MatrixTransform;

	sort(root.MEPElements.begin(), root.MEPElements.end(), [](const MEPElement& elemt1, const MEPElement& elemt2) {
		return elemt1.Id < elemt2.Id;
		});

	unordered_map<uint64_t, int> IDMAP;//长ID到短ID的映射
	unordered_map<int, uint64_t> IDMAP2; //短到长的映射
	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		auto& ELEMENT = root.MEPElements[i];
		IDMAP[ELEMENT.Id] = i + 20;
		IDMAP2[i + 20] = ELEMENT.Id;
	}

	//把Id  ConnectedElementId都变成短的ID
	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		auto& element = root.MEPElements[i];
		osg::ref_ptr<osg::Group> elementGroup = new osg::Group;
		rootNode->addChild(elementGroup);

		element.Id = IDMAP[element.Id];

		for (uint64_t& id : element.ConnectedElementId)
		{
			id = IDMAP[id];
		}

		elementGroup->addChild(handleGeometry(element, element.Id));
	}

	vector<int> index1(root.MEPElements.size() + 100);
	vector<int> index2;
	index2.push_back(0);
	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		vector<uint64_t> idConnected = root.MEPElements[i].ConnectedElementId;
		const int id = root.MEPElements[i].Id;

		if (idConnected.size() == 0)
		{
			index1[id] = 0; //说明第i个构建，没有连接关系
		}
		else
		{
			index1[id] = index2.size();
			index2.insert(index2.end(), idConnected.begin(), idConnected.end());
			index2.push_back(0);
		}
	}
	index2.push_back(0);

	g_textureBuffer1 = LineHole::create_tbo(index1);
	g_textureBuffer2 = LineHole::create_tbo(index2);

#if 0
	auto to_long = [&](int i) {
		return IDMAP2[i];
	};

	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		auto& element = root.MEPElements[i];
		cout << to_long(element.Id) << "  :  \n";

		if (element.ConnectedElementId.empty())
			continue;

		for (int j : element.ConnectedElementId)
		{
			cout << to_long(j) << "\n";
}

		cout << "\n\n";
	}
#endif
	osg::ComputeBoundsVisitor cbbv;
	rootNode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return rootNode.release();
}