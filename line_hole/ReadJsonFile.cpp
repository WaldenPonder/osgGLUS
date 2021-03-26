#include "pch.h"
#include "ReadJsonFile.h"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include "LineHole.h"
#include <unordered_map>

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

void parseGeometry(const Json::Value& value, Geometrys& geometry)
{
	auto t = value.type();
	if (t == Json::arrayValue)
	{
		int size = value.size();
		for (int i = 0; i < size; i++)
		{
			parseGeometry(value[i], geometry);
		}

		return;
	}

	string type = value["Type"].asString();
	if (type == "Line")
	{
		LINE line;
		parsePoint(value["StartPoint"], line.StartPoint);
		parsePoint(value["EndPoint"], line.EndPoint);
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
		TRIANGLE triangle;
		parsePoint(value["FirstPoint"], triangle.FirstPoint);
		parsePoint(value["SecondPoint"], triangle.SecondPoint);
		parsePoint(value["ThirdPoint"], triangle.ThirdPoint);
		geometry.triangles.push_back(triangle);
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

		parseGeometry(ELEMENT["Geometry"], element.Geometry);

		element.HasGeometry = ELEMENT["HasGeometry"].asBool();
		element.Id = ELEMENT["Id"].asUInt64();
		element.Name = ELEMENT["Name"].asString();
		g_elementRoot.MEPElements.push_back(element);
	}
}

osg::Group* handleGeometry(const MEPElement& element, int id)
{
	osg::ref_ptr<osg::Group> root = new osg::Group;
	//-----------------------------------------------------------line
	{
		std::vector<osg::Vec3> allPTs;

		for (auto& line : element.Geometry.lines)
		{
			allPTs.push_back(line.StartPoint);
			allPTs.push_back(line.EndPoint);
		}

		{
			osg::Geometry* geometry = LineHole::createLine2(allPTs, { osg::Vec4(element.Color, 1) }, { id }, g_viewer->getCamera(), osg::PrimitiveSet::LINES);
			osg::Geode* geode = new osg::Geode;
			geode->setNodeMask(NM_LINE);
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
			geode->setNodeMask(NM_LINE);
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

	//-----------------------------------------------------------triangle
	{
		std::vector<osg::Vec3> allPTs;

		for (auto& triangle : element.Geometry.triangles)
		{
			allPTs.push_back(triangle.FirstPoint);
			allPTs.push_back(triangle.SecondPoint);
			allPTs.push_back(triangle.ThirdPoint);
		}

		osg::Geometry* geometry = LineHole::createTriangles(allPTs, { osg::Vec4(184.0 / 255, 213. / 255., 220.0 / 255, 1) }, { id }, g_viewer->getCamera());
		osg::Geode* geode = new osg::Geode;
		geode->setNodeMask(NM_FACE);
		geode->addDrawable(geometry);
		root->addChild(geode);
		geometry->setNodeMask(NM_FACE);

		//越小越先画，默认0, 面要最先画,  实体线第二   虚线最后
		geode->getOrCreateStateSet()->setRenderBinDetails(RenderPriority::FACE, "RenderBin"); //面
		LineHole::setUpStateset(geode->getOrCreateStateSet(), g_viewer->getCamera(), false);
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
		IDMAP[ELEMENT.Id] = i + 1;
		IDMAP2[i + 1] = ELEMENT.Id;
	}

	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		auto& ELEMENT = root.MEPElements[i];
		osg::ref_ptr<osg::Group> elementGroup = new osg::Group;
		rootNode->addChild(elementGroup);

		ELEMENT.Id = IDMAP[ELEMENT.Id];

		for (uint64_t& id : ELEMENT.ConnectedElementId)
		{
			id = IDMAP[id];
		}

		elementGroup->addChild(handleGeometry(ELEMENT, ELEMENT.Id));
	}

	vector<int> index1(root.MEPElements.size() + 100);
	vector<int> index2;
	index2.push_back(0);
	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		vector<uint64_t> idConnected = root.MEPElements[i].ConnectedElementId;
		if (idConnected.size() == 0)
		{
			index1[i] = 0; //说明第i个构建，没有连接关系
		}
		else
		{
			index1[i] = index2.size();
			index2.insert(index2.end(), idConnected.begin(), idConnected.end());
			index2.push_back(0);
		}
	}

	g_textureBuffer1 = LineHole::create_tbo(index1);
	g_textureBuffer2 = LineHole::create_tbo(index2);

	auto to_long = [&](int i) {
		return IDMAP2[i];
	};

	for (int i = 0; i < root.MEPElements.size(); i++)
	{
		auto& element = root.MEPElements[i];
		cout << to_long(element.Id) << "  :  \n";

		if(element.ConnectedElementId.empty())
			continue;

		for (int j : element.ConnectedElementId)
		{
			cout << to_long(j) << "\n";
		}

		cout << "\n\n";
	}
	//getchar();
	osg::ComputeBoundsVisitor cbbv;
	rootNode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return rootNode.release();
}