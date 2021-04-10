#pragma once

//Hammersley序列， 能得到比较均匀的数据， 0.5   0.25  0.75  0.125 ...
double phi(int j)
{
	double x = 0;
	double f = 0.5;

	while (j)
	{
		x += f * (double)(j & 1);
		j /= 2;
		f *= 0.5;
	}

	return x;
}

//g_rang_i存储的是采样的数据，不会重复，能覆盖g_current_range内所有像素
void recalculat_range_i()
{
	//while (true)
	{
		//cin >> g_current_range;
		int lo = 20;
		int hi = g_current_range;

		int len = hi - lo + 1;
		vector<bool> flags(g_current_range + 100, false);

		flags[20] = flags[g_current_range] = true;

		{
			std::queue<int> temp;
			std::swap(temp, g_rang_i);
		}

		g_rang_i.push(g_current_range); //20 和g_current_range ， 哪一个放在第一个合适？？

		if (g_current_range <= 20)
			return;

		g_rang_i.push(20); //小于等于20, shader内直接暴力查询
		for (int i = 1; i <= len; i++)
		{
			double f = phi(i);

			int val = f * len + 20;
			if (flags[val])  //说明重复了, 去查找它最近的没有还没有采样的range
			{
				int j = val;
				int k = val;

				while (j > 20 && k < g_current_range)
				{
					j--; k++;
					if (j > 20 && !flags[j])
					{
						val = j;
						break;
					}

					if (k < g_current_range && !flags[k])
					{
						val = k;
						break;
					}
				}

				while (j > 20)
				{
					j--;
					if (j > 20 && !flags[j])
					{
						val = j;
						break;
					}
				}

				while (k < g_current_range)
				{
					k++;
					if (k < g_current_range && !flags[k])
					{
						val = k;
						break;
					}
				}
			}

			if (!flags[val])
			{
				g_rang_i.push(val);
				//std::cout << i << "   :   " << f << "   :  " << val << "\n";
			}

			flags[val] = true;
		}
	}
}

void precessTextureFrameByFrame()
{
	if (g_is_need_recalculate_range)
	{
		cout << "re calcu\n";

		g_is_need_recalculate_range = false;
		recalculat_range_i();

		//-----------------------------------------------------------g_cablePass
		{
			TextureFrameByFrame& texture1 = g_cablePass.frameTexture1;
			TextureFrameByFrame& texture2 = g_cablePass.frameTexture2;

			auto ss = texture1.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, g_cablePass.baseColorTexture, osg::StateAttribute::ON);
			ss = texture2.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture1.texture, osg::StateAttribute::ON);

			texture1.camera->setNodeMask(~0);
			texture2.camera->setNodeMask(~0);
			texture1.camera->attach(osg::Camera::COLOR_BUFFER0, texture1.texture, 0, 0, false);
			texture2.camera->attach(osg::Camera::COLOR_BUFFER0, texture2.texture, 0, 0, false);

			LineHole::displayTextureInHudCamera(texture2.texture, osg::Vec3(0, 0, QUAD_Z::cableZ), RenderPriority::cableQuad, NM_CABLE_PASS_QUAD);
		}

		//-----------------------------------------------------------g_linePass
		{
			auto texture1 = g_linePass.frameTexture1;
			auto texture2 = g_linePass.frameTexture2;

			auto ss = texture1.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, g_linePass.baseColorTexture, osg::StateAttribute::ON);
			ss = texture2.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture1.texture, osg::StateAttribute::ON);

			texture1.camera->setNodeMask(~0);
			texture2.camera->setNodeMask(~0);
			texture1.camera->attach(osg::Camera::COLOR_BUFFER0, texture1.texture, 0, 0, false);
			texture2.camera->attach(osg::Camera::COLOR_BUFFER0, texture2.texture, 0, 0, false);

			LineHole::displayTextureInHudCamera(texture2.texture, osg::Vec3(0, 0, QUAD_Z::lineZ), RenderPriority::lineQuad, NM_LINE_PASS_QUAD);
		}

		LineHole::displayTextureInHudCamera(g_backgroundPass.baseColorTexture, osg::Vec3(0, 0, QUAD_Z::faceZ), RenderPriority::backgroundQuad, NM_BACKGROUND_PASS_QUAD);
		return;
	}

	g_hudCamera->removeChildren(0, g_hudCamera->getNumChildren());

	//-----------------------------------------------------------g_cablePass
	{
		TextureFrameByFrame& texture1 = g_cablePass.frameTexture1;
		TextureFrameByFrame& texture2 = g_cablePass.frameTexture2;
		if (texture1.camera->getNodeMask() == 0)
		{
			auto ss = texture1.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture2.texture, osg::StateAttribute::ON);
			texture1.camera->setNodeMask(~0);
			texture2.camera->setNodeMask(0);
			texture1.camera->attach(osg::Camera::COLOR_BUFFER0, texture1.texture, 0, 0, false);
			LineHole::displayTextureInHudCamera(texture1.texture, osg::Vec3(0, 0, QUAD_Z::cableZ), RenderPriority::cableQuad, NM_CABLE_PASS_QUAD);
		}
		else
		{
			auto ss = texture2.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture1.texture, osg::StateAttribute::ON);
			texture2.camera->setNodeMask(~0);
			texture1.camera->setNodeMask(0);
			texture2.camera->attach(osg::Camera::COLOR_BUFFER0, texture2.texture, 0, 0, false);
			LineHole::displayTextureInHudCamera(texture2.texture, osg::Vec3(0, 0, QUAD_Z::cableZ), RenderPriority::cableQuad, NM_CABLE_PASS_QUAD);
		}
	}

	//-----------------------------------------------------------g_linePass
	{
		TextureFrameByFrame& texture1 = g_linePass.frameTexture1;
		TextureFrameByFrame& texture2 = g_linePass.frameTexture2;
		if (texture1.camera->getNodeMask() == 0)
		{
			auto ss = texture1.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture2.texture, osg::StateAttribute::ON);
			texture1.camera->setNodeMask(~0);
			texture2.camera->setNodeMask(0);
			texture1.camera->attach(osg::Camera::COLOR_BUFFER0, texture1.texture, 0, 0, false);
			LineHole::displayTextureInHudCamera(texture1.texture, osg::Vec3(0, 0, QUAD_Z::lineZ), RenderPriority::lineQuad, NM_LINE_PASS_QUAD);
		}
		else
		{
			auto ss = texture2.geode->getOrCreateStateSet();
			ss->setTextureAttributeAndModes(0, texture1.texture, osg::StateAttribute::ON);
			texture2.camera->setNodeMask(~0);
			texture1.camera->setNodeMask(0);
			texture2.camera->attach(osg::Camera::COLOR_BUFFER0, texture2.texture, 0, 0, false);
			LineHole::displayTextureInHudCamera(texture2.texture, osg::Vec3(0, 0, QUAD_Z::lineZ), RenderPriority::lineQuad, NM_LINE_PASS_QUAD);
		}
	}

	LineHole::displayTextureInHudCamera(g_backgroundPass.baseColorTexture, osg::Vec3(0, 0, QUAD_Z::faceZ), RenderPriority::backgroundQuad, NM_BACKGROUND_PASS_QUAD);
}

//切换场景
class MyEventHandler : public osgGA::GUIEventHandler
{
public:
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if (!viewer || !g_sceneNode) return false;

		if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
		{
			g_is_need_recalculate_range = true;
			if (ea.getKey() == 'a')
			{
				flag_start = true;
				static osg::Matrix s_projectionMatrix;
				auto* mani = g_viewer->getCameraManipulator();
				if (mani == g_trackballManipulator.get())
				{
					g_viewer->setCameraManipulator(g_twodimManipulator);
					s_projectionMatrix = g_viewer->getCamera()->getProjectionMatrix();

					g_is_orth_camera = true;
				}
				else
				{
					g_viewer->setCameraManipulator(g_trackballManipulator);
					g_viewer->getCamera()->setProjectionMatrix(s_projectionMatrix);
					g_is_orth_camera = false;
				}
			}
			else if (ea.getKey() == 'b')
			{
				g_is_top_view = !g_is_top_view;
				g_viewer->getCameraManipulator()->home(0);
			}
			else if (ea.getKey() == 'c')
			{
				g_always_dont_connected = !g_always_dont_connected;
			}
			else if (ea.getKey() == 'd')
			{
				osg::Camera* camera = g_linePass.rttCamera;
				if (camera->getCullMask() & NM_HIDDEN_LINE)
					camera->setCullMask(camera->getCullMask() & ~NM_HIDDEN_LINE);
				else
					camera->setCullMask(camera->getCullMask() | NM_HIDDEN_LINE);
			}
			else if (ea.getKey() == 'e')
			{
				g_line_hole_enable = !g_line_hole_enable;
			}
			else if (ea.getKey() == 'f')
			{
				unsigned mask = g_linePass.rttCamera->getCullMask();
				if (mask & NM_FACE)
					g_linePass.rttCamera->setCullMask(mask & ~NM_FACE);
				else
					g_linePass.rttCamera->setCullMask(mask | NM_FACE);

				unsigned mask2 = g_backgroundPass.rttCamera->getCullMask();
				if (mask2 & NM_FACE)
					g_backgroundPass.rttCamera->setCullMask(mask2 & ~NM_FACE);
				else
					g_backgroundPass.rttCamera->setCullMask(mask2 | NM_FACE);
			}
			else if (ea.getKey() == 'h')
			{
				if (g_convexRoot)
					g_convexRoot->removeChildren(0, g_convexRoot->getNumChildren());

				ConvexHullVisitor chv;
				chv.setTraversalMask(NM_FACE | NM_QIAOJIA_JIDIANSHEBEI);
				g_sceneNode->accept(chv);

				g_sceneNode->addChild(g_convexRoot);
			}
			else if (ea.getKey() == 'i')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_BACKGROUND_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_BACKGROUND_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_BACKGROUND_PASS_QUAD);
			}
			else if (ea.getKey() == 'j')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_LINE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_LINE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_LINE_PASS_QUAD);
			}
			else if (ea.getKey() == 'k')
			{
				unsigned mask = g_viewer->getCamera()->getCullMask();
				if (mask & NM_CABLE_PASS_QUAD)
					g_viewer->getCamera()->setCullMask(mask & ~NM_CABLE_PASS_QUAD);
				else
					g_viewer->getCamera()->setCullMask(mask | NM_CABLE_PASS_QUAD);
			}
			else if (ea.getKey() == 't')
			{
				g_always_intersection = !g_always_intersection;
			}
			else if (ea.getKey() == 'u')
			{
			}
			else if (ea.getKey() == 'y')
			{
				system("cls");
				osg::Matrix mvp = g_viewer->getCamera()->getViewMatrix() * g_viewer->getCamera()->getProjectionMatrix();

				for (int i = 0; i < g_elementRoot.MEPElements.size(); i++)
				{
					auto& element = g_elementRoot.MEPElements[i];

					for (const LINE& line : element.Geometry.lines)
					{
						osg::Vec4 pt = osg::Vec4(line.StartPoint, 1) * mvp;
						bool b1 = abs(pt.x()) < 1 && abs(pt.y()) < 1;

						osg::Vec4 pt2 = osg::Vec4(line.EndPoint, 1) * mvp;
						bool b2 = abs(pt2.x()) < 1 && abs(pt2.y()) < 1;

						if (b1 && b2)
							cout << "  ---   " << pt << "\t" << pt2 << "\n";
					}
				}
			}
		}
		else if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME && flag_start)
		{
			precessTextureFrameByFrame();
		}
		return __super::handle(ea, aa);
	}
};

int init(osgViewer::Viewer& viewer)
{
	//-----------------------------------------------------------窗口
	osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
	traits->x = 420; traits->y = 10;
	traits->width = TEXTURE_SIZE1; traits->height = TEXTURE_SIZE2;
	traits->windowDecoration = true;
	traits->doubleBuffer = true;
	traits->readDISPLAY();
	traits->setUndefinedScreenDetailsToDefaultScreen();
	osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext(traits.get());
	if (!gc.valid())
	{
		osg::notify(osg::FATAL) << "Unable to create OpenGL v" << " context." << std::endl;
		return -1;
	}

	//-----------------------------------------------------------初始化各种相机
	osg::Camera* cam = viewer.getCamera();
	cam->setGraphicsContext(gc.get());
	cam->setViewport(new osg::Viewport(0, 0, TEXTURE_SIZE1, TEXTURE_SIZE2));

	osg::Camera* mainCamera = g_viewer->getCamera();
	mainCamera->setClearColor(CLEAR_COLOR);

	g_linePass.type = RenderPass::LINE_PASS;
	g_backgroundPass.type = RenderPass::BACKGROUND_PASS;
	g_cablePass.type = RenderPass::CABLE_PASS;

	LineHole::createRttCamera(&viewer, g_linePass);
	LineHole::createRttCamera(&viewer, g_backgroundPass);
	LineHole::createRttCamera(&viewer, g_cablePass);

	g_root->addChild(g_backgroundPass.rttCamera);
	g_root->addChild(g_linePass.rttCamera);
	g_root->addChild(g_cablePass.rttCamera);

	g_linePass.rttCamera->setCullMask(NM_LINE | NM_OUT_LINE | NM_HIDDEN_LINE | NM_FACE);
	//底图
	g_backgroundPass.rttCamera->setCullMask(NM_FACE | NM_QIAOJIA_JIDIANSHEBEI);
	//导线pass, 不需要绘制隐藏线
	g_cablePass.rttCamera->setCullMask(NM_CABLE | NM_QIAOJIA_JIDIANSHEBEI | NM_OUT_LINE);

	g_sceneNode = ReadJsonFile::createScene(g_elementRoot);
	g_linePass.rttCamera->addChild(g_sceneNode);
	g_backgroundPass.rttCamera->addChild(g_sceneNode);
	g_cablePass.rttCamera->addChild(g_sceneNode);

	LineHole::createHudCamera(&viewer);
	g_root->addChild(g_hudCamera);

	return 0;
}

void ReadFile()
{
	TEXTURE_SIZE1 = TEXTURE_SIZE2 = 1024;
	std::string file_name;

	bool bReadFileSuccess = true;
	ifstream IF(shader_dir() + "/line_hole/config.ini");

	if (!IF.is_open())
	{
		bReadFileSuccess = false;
		return;
	}

	do
	{
		string buf;
		getline(IF, buf);
		if (buf == "FILE_PATH:")
		{
			getline(IF, buf);
			if (!buf.empty())
			{
				g_is_daoxian_file = false;
				file_name = buf;
			}
		}
		else if (buf == "DAOXIAN_FILE:")
		{
			getline(IF, buf);
			if (!buf.empty())
			{
				file_name = buf;
				g_is_daoxian_file = true;
			}
		}
	} while (!IF.fail());

	ReadJsonFile::read(file_name);
}
