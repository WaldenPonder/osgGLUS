#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"
#include <osg/Stencil>
#include <osg/StencilTwoSided>
#include "ModelManager.h"
#include "osgDB/FileUtils"
#include "KdTree.h"

ModelManager* g_modelManager;
osg::Group*   g_root = new osg::Group;

string file_path = "F:\\BaiduYunDownload";
//string file_path = "D:\\osgEarth\\models\\quannan";

void createScene()
{
	std::vector<std::string> files, f;
	std::string				 str;

	str = file_path + "\\隧道模型\\";
	f   = osgDB::getDirectoryContents(str);
	for (auto& s : f) s = str + s;
	files.insert(files.end(), f.begin(), f.end());

	str = file_path + "\\桥梁模型\\";
	f   = osgDB::getDirectoryContents(str);
	for (auto& s : f) s = str + s;
	files.insert(files.end(), f.begin(), f.end());

	//str = file_path + "\\路线模型\\";
	//f = osgDB::getDirectoryContents(str);
	//for (auto& s : f) s = str + s;
	//files.insert(files.end(), f.begin(), f.end());

	//str = file_path + "\\路面模型\\";
	//f = osgDB::getDirectoryContents(str);
	//for (auto& s : f) s = str + s;
	//files.insert(files.end(), f.begin(), f.end());

	//str = file_path + "\\路基模型\\";
	//f = osgDB::getDirectoryContents(str);
	//for (auto& s : f) s = str + s;
	//files.insert(files.end(), f.begin(), f.end());

	//str = file_path + "\\交安模型\\";
	//f = osgDB::getDirectoryContents(str);
	//for (auto& s : f) s = str + s;
	//files.insert(files.end(), f.begin(), f.end());

	for (auto& str : files)
	{
		osg::Node* n = osgDB::readNodeFile(str);
		if (n)
			g_modelManager->getModelRoot()->addChild(n);
	}
}

vector<osg::BoundingBox> g_bbs;

void ff()
{
	int j = 0;
	for (int i = 0; i < 20; i++)
	{
		//for (int j = 0; j < 5; j++)
		{
			osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
			g_root->addChild(pat);
			pat->setPosition(osg::Vec3(i * 15, j * 15, 0));

			osg::Node* cow = osgDB::readNodeFile("cow.osg");
			pat->addChild(cow);

			osg::Matrix mat = cow->getWorldMatrices()[0];

			osg::ComputeBoundsVisitor cbv;
			cow->accept(cbv);

			osg::BoundingBox bb, bbNew;
			bb = cbv.getBoundingBox();

			for (int j = 0; j <= 7; j++)
			{
				bbNew.expandBy(bb.corner(j) * mat);
			}
			g_kdtree.add(bbNew, pat);
		}	
	}
}

//-------------------------------------------------------------EventCallback
class EventCallback : public osgGA::GUIEventHandler
{
 public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa /*aa*/,
						osg::Object* object, osg::NodeVisitor* nv)
	{
		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F1)
			{
				osg::Node::NodeMask ms = g_modelManager->getBoundingbox()->getNodeMask();
				if (ms == NM_NO_SHOW)
				{
					g_modelManager->getBoundingbox()->setNodeMask(NM_ALL);
				}
				else
				{
					g_modelManager->getBoundingbox()->setNodeMask(NM_NO_SHOW);
				}
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F2)
			{
				g_cullActive = !g_cullActive;
			}
			else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F3)
			{
				osg::Node* n = g_kdtree.drawBoundingBox_.get();
				n->setNodeMask(~n->getNodeMask());
			}
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			float precomputedNumerator[3];
			float precomputedDenominator[3];

			float x = ea.getXnormalized(), y = ea.getYnormalized();

			osg::Matrix mat	= view->getCamera()->getViewMatrix() * view->getCamera()->getProjectionMatrix();
			osg::Matrix matNew = osg::Matrix::inverse(mat);
			osg::Vec3   start  = osg::Vec3(x, y, -1) * matNew;
			osg::Vec3   end	= osg::Vec3(x, y, 1) * matNew;

			osg::Vec3 normals[] = { osg::X_AXIS, osg::Y_AXIS, osg::Z_AXIS };
			osg::Vec3 dir		= end - start;

			Ray ray;
			ray.start = start, ray.dir = dir;

			for (uint8_t i = 0; i < 3; i++)
			{
				ray.precomputedNumerator[i]   = normals[i] * start;
				ray.precomputedDenominator[i] = normals[i] * dir;
			}

			std::cout << "**********\n\n";
			int i = 0;
			for (auto& bb : g_bbs)
			{
				if (intersectionBoundingBox(ray, bb))
				{
					cout << i << endl;
				}
				i++;
			}
		}

		return false;
	}
};

int main()
{
	osgViewer::Viewer view;

	g_modelManager = new ModelManager(view.getCamera(), g_root);
	createScene();
	g_modelManager->buildBoundingbox();

	//ff();

	g_kdtree.build();
	g_root->addChild(g_kdtree.drawBoundingBox_.get());
	
	view.setSceneData(g_root);
	add_event_handler(view);
	view.addEventHandler(new EventCallback);
	osg::Camera* camera = view.getCamera();
	camera->setCullMask(~NM_NO_SHOW);
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	auto ss = camera->getOrCreateStateSet();

	osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
	view.realize();
	return view.run();
}
