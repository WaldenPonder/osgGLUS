﻿#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"
#include "osgDB/FileUtils"
#include "CascadeShadowMap.h"
#include "osg/BlendFunc"

osgViewer::Viewer* g_viewer = nullptr;

osg::Group*					   g_terrain = new osg::Group;
osg::Group*					   g_group2	 = new osg::Group;
osg::Group*					   g_group3	 = new osg::Group;
osg::Group*					   g_root	 = new osg::Group;
osg::MatrixTransform*		   g_trans	 = new osg::MatrixTransform;
float						   h		 = 330;
osg::ref_ptr<CascadeShadowMap> shadow_map;

//读取osgb文件
void readTerrainNode()
{
	std::vector<std::string> f;

	std::string path("F:\\FileRecv\\K10-K22（osgb）\\Data\\");
	f	  = osgDB::getDirectoryContents(path.c_str());
	int i = 0;
	for (auto& s : f)
	{
		i++;
		//if(i > 30) break;
		string file = path + s + "/" + s + ".osgb";
		if (osgDB::fileExists(file))
		{
			osg::Node* n = osgDB::readNodeFile(file);
			g_terrain->addChild(n);
		}
	}

	osg::Program* p = new osg::Program;
	p->addShader(osgDB::readShaderFile(shader_dir() + "/gis_cull.vert"));
	p->addShader(osgDB::readShaderFile(shader_dir() + "/gis_cull.frag"));
	g_terrain->getOrCreateStateSet()->setAttributeAndModes(p);
	g_terrain->getOrCreateStateSet()->addUniform(new osg::Uniform("baseTexture", 0));
}

void ff(osgViewer::Viewer& view)
{
	{
		osg::ComputeBoundsVisitor cbv;
		g_terrain->accept(cbv);

		CascadeShadowMap::Param param;
		param.bbTerrain	 = cbv.getBoundingBox();
		param.mainCamera = view.getCamera();
		param.root		 = g_root;
		for (int i = 1; i <= 3; i++)
		{
			param.far_distance_split_names.push_back(std::string("u_zgis_CULL") + std::to_string(i));
			param.shadow_mat_names.push_back(std::string("u_GISCullRegionMat") + std::to_string(i));
			param.shadow_texture_names.push_back(std::string("u_GISCullTexture") + std::to_string(i));
		}
		shadow_map					= new CascadeShadowMap(param);
		osg::ref_ptr<osg::Light> l1 = new osg::Light;
		l1->setPosition(osg::Vec4(0, 0, 1, 0));
		shadow_map->setLight(l1);
		int id;
		shadow_map->applyStateset(g_terrain->getOrCreateStateSet(), 1, id);

		osg::Node* node = osgDB::readNodeFile("E:/cull_region.osg");
		g_trans->addChild(node);
		g_group2->addChild(g_trans);

		g_root->addChild(g_group2);
		shadow_map->addNode(g_trans);

		osg::StateSet* ss = g_terrain->getOrCreateStateSet();
		ss->setMode(GL_BLEND, osg::StateAttribute::ON);
		ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		ss->setAttributeAndModes(new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
	}
}

class EventCallback : public osgGA::GUIEventHandler
{
 public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/,
						osg::Object* object, osg::NodeVisitor* nv)
	{
		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == 'a')
			{
				g_group2->setNodeMask(~g_group2->getNodeMask());
			}
			else if (ea.getKey() == 'b')
			{
			}
			else if (ea.getKey() == '2')
			{
				g_group3->setNodeMask(~g_group3->getNodeMask());
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F5)
			{
				h -= 5;
				cout << h << "\n";
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F6)
			{
				h += 5;
				cout << h << "\n";
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F7)
			{
				g_viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::ComputeNearFarMode::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F8)
			{
				g_viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::ComputeNearFarMode::COMPUTE_NEAR_USING_PRIMITIVES);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F9)
			{
				g_viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::ComputeNearFarMode::DO_NOT_COMPUTE_NEAR_FAR);
				osg::Matrix mat = g_viewer->getCamera()->getProjectionMatrix();

				float fovy, aspectRatio, zNear, zFar;
				mat.getPerspective(fovy, aspectRatio, zNear, zFar);
				g_viewer->getCamera()->setProjectionMatrixAsPerspective(fovy, aspectRatio, 100, 100000);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F1)
			{
				g_viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F2)
			{
				g_viewer->setThreadingModel(osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F3)
			{
				g_viewer->setThreadingModel(osgViewer::ViewerBase::CullDrawThreadPerContext);
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F4)
			{
				g_viewer->setThreadingModel(osgViewer::ViewerBase::DrawThreadPerContext);
			}

			g_trans->setMatrix(osg::Matrix::translate(0, 0, h));
		}
		return false;
	}
};

void f()
{
	vector<int> arr = { 2, 1, 5, 3, 6, 4, 9, 8, 7 };

	int temp;

	for (int i = 1; i < arr.size(); i++)
	{
		//待排元素小于有序序列的最后一个元素时，向前插入
		if (arr[i] < arr[i - 1])
		{
			temp = arr[i];
			for (int j = i; j >= 0; j--)
			{
				if (j > 0 && arr[j - 1] > temp)
				{
					arr[j] = arr[j - 1];
				}
				else
				{
					arr[j] = temp;
					break;
				}
			}
		}
	}
}

int main()
{

	osgViewer::Viewer view;
	osg::DisplaySettings::instance()->setNumMultiSamples(4);

	g_viewer = &view;
	view.realize();
	float f = 155.0f / 255.f;
	view.getCamera()->setClearColor(osg::Vec4(f, f, f, 1));
	g_root->addChild(g_terrain);
	g_root->addChild(g_group2);
	g_root->addChild(g_group3);

	readTerrainNode();

	ff(*g_viewer);

	// g_group3->addChild(debugTexture(800, 600, shadow_map->getTexture(1), 0.5));

	view.setSceneData(g_root);
	view.addEventHandler(new EventCallback);
	add_event_handler(view);
	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);

	view.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

	return view.run();
}
