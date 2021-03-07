
#if 1

osg::Node* create_lines(osgViewer::Viewer& view)
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<float> rd(0, 1);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3>		 PTs, COLORs;

	float z = 0.5f;
	PTs.push_back(osg::Vec3(2, 0, z));
	PTs.push_back(osg::Vec3(-2, 0, z));
	osg::Geometry* n = createLine2(PTs, { osg::Vec3(1, 0, 0) }, { 1 }, view.getCamera());
	n->setName("LINE1");
	geode->addDrawable(n);

	PTs.clear();
	z = 0.35f;
	PTs.push_back(osg::Vec3(0, 2, z));
	PTs.push_back(osg::Vec3(0, -2, z));
	n = createLine2(PTs, { osg::Vec3(1, 1, 0) }, { 4 }, view.getCamera());
	n->setName("LINE1");
	geode->addDrawable(n);

	//-------------------------------------------------
	PTs.clear();
	z = 0;
	PTs.push_back(osg::Vec3(-1, -1.3, z));
	PTs.push_back(osg::Vec3(1, -1, z));
	PTs.push_back(osg::Vec3(1, 1, z));
	PTs.push_back(osg::Vec3(-1, 1.3, z));
	osg::Geometry* n2 = createLine2(PTs, { osg::Vec3(0, 1, 0) }, { 2 }, view.getCamera());
	n2->setName("LINE2");
	geode->addDrawable(n2);

	PTs.clear();
	z = -0.5;
	PTs.push_back(osg::Vec3(-1.5, -1.5, z));
	PTs.push_back(osg::Vec3(1.5, -1.5, z));
	PTs.push_back(osg::Vec3(1.5, 1.5, z));
	PTs.push_back(osg::Vec3(-1.5, 1.5, z));

	osg::Geometry* n3 = createLine2(PTs, { osg::Vec3(0, 0, 1) }, { 3 }, view.getCamera());
	n3->setName("LINE3");
	geode->addDrawable(n3);

	vector<int> index1(100);
	index1[1] = 1;
	index1[2] = 0;
	index1[3] = 4;
	index1[4] = 7;
	g_textureBuffer1 = create_tbo(index1);

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

	g_textureBuffer2 = create_tbo(index2);

	//uniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, "u_color");
	//uniform->set(osg::Vec4(0, 1, 0, 1.));
	//n2->getOrCreateStateSet()->addUniform(uniform);

	osg::ComputeBoundsVisitor cbbv;
	geode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return geode.release();
}

#else



osg::Node* create_lines(osgViewer::Viewer& view)
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<float> rd(-2, 2);

	vector<osg::Vec3> myColor;
	myColor.push_back(osg::Vec3(1, 0, 0));
	myColor.push_back(osg::Vec3(0, 1, 0));
	myColor.push_back(osg::Vec3(0, 0, 1));

	myColor.push_back(osg::Vec3(1, 1, 0));
	myColor.push_back(osg::Vec3(1, 0, 1));
	myColor.push_back(osg::Vec3(0, 1, 1));
	myColor.push_back(osg::Vec3(1, 1, 1));

	myColor.push_back(osg::Vec3(0, 0.4, 1));
	myColor.push_back(osg::Vec3(0.7, 0, 1));

	vector<int> myZ{-1, 1, 2, -2};

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	vector<osg::Vec3>		 PTs, COLORs;
	vector<int> ids;

	int id = 1;
	int size = 100000;
	float z = 0;

	for (int i = 1; i < size; i++)
	{
		int sz = i;
		int cindex = rand() % myColor.size();
		int zindex = rand() %  myZ.size();
		z = rd(eng);

		PTs.push_back(osg::Vec3(-sz, -sz, z));
		PTs.push_back(osg::Vec3(sz, -sz, z));

		PTs.push_back(osg::Vec3(sz, -sz, z));
		PTs.push_back(osg::Vec3(sz, sz, z));

		PTs.push_back(osg::Vec3(sz, sz, z));
		PTs.push_back(osg::Vec3(-sz, sz, z));

		PTs.push_back(osg::Vec3(-sz, sz, z));
		PTs.push_back(osg::Vec3(-sz, -sz, z));

		for (int j = 0; j < 8; j++)
		{
			COLORs.push_back(myColor[cindex]);
			ids.push_back(i);
		}
	}

	osg::Geometry* n = createLine2(PTs, COLORs, ids, view.getCamera(), osg::PrimitiveSet::LINES);
	n->setName("LINE1");
	geode->addDrawable(n);
	size = 100;
	PTs.clear();
	z = 0.5f;
	PTs.push_back(osg::Vec3(size, 0, z));
	PTs.push_back(osg::Vec3(-size, 0, z));
	n = createLine2(PTs, { osg::Vec3(1, 0, 0) }, { 999 }, view.getCamera());
	n->setName("LINE2");
	geode->addDrawable(n);

	PTs.clear();
	z = 0.35f;
	PTs.push_back(osg::Vec3(0, size, z));
	PTs.push_back(osg::Vec3(0, -size, z));
	n = createLine2(PTs, { osg::Vec3(1, 1, 0) }, { 888 }, view.getCamera());
	n->setName("LINE3");
	geode->addDrawable(n);

	vector<int> index1(1000);
	//index1[1] = 1;
	//index1[2] = 0;
	//index1[3] = 4;
	//index1[4] = 7;
	g_textureBuffer1 = create_tbo(index1);

	vector<int> index2(1000);
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

	g_textureBuffer2 = create_tbo(index2);

	osg::ComputeBoundsVisitor cbbv;
	geode->accept(cbbv);
	g_line_bbox = cbbv.getBoundingBox();

	return geode.release();
}

#endif

class CameraPredrawCallback : public osg::Camera::DrawCallback
{
public:
	osg::observer_ptr<osg::Camera> rttCamera;
	osg::observer_ptr<osg::Camera> mainCamera;

	CameraPredrawCallback(osg::Camera* first, osg::Camera* main) : rttCamera(first), mainCamera(main) {}
	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		osg::Viewport* vp = mainCamera->getViewport();

		if (rttCamera.get())
		{
			rttCamera->setProjectionMatrix(mainCamera->getProjectionMatrix());
			rttCamera->setViewMatrix(mainCamera->getViewMatrix());

			osg::ref_ptr<osg::RefMatrix> proMat = new osg::RefMatrix(rttCamera->getProjectionMatrix());
			renderInfo.getState()->applyProjectionMatrix(proMat);
			renderInfo.getState()->applyModelViewMatrix(rttCamera->getViewMatrix());
		}
	}
};