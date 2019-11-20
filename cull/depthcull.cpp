#include "pch.h"
#include "../common/common.h"
#include <osg/io_utils>
#include <osg/KdTree>
#include <random>
#include "osg/MatrixTransform"
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>

osg::Node* CreateTileModel()
{
	std::string								   tilePath("0-0-3\\tile-pt.txt");
	osg::ref_ptr<osg::Vec3dArray>			   pVertexs		 = new osg::Vec3dArray;
	osg::ref_ptr<osg::DrawElementsUInt>		   pVertexIndexs = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
	std::vector<osg::ref_ptr<osg::Vec3dArray>> masks;

	test::CReadTestArrayVisitor reader(tilePath);
	reader.apply(pVertexs);

	mono::CTriangulationUtility::grid_cull_triangulate(pVertexs, pVertexIndexs, masks, 1789.0, 10.0);
	if (pVertexIndexs->empty())
		return nullptr;

	size_t						 total		 = pVertexs->size();
	osg::ref_ptr<osg::Vec3Array> pNewVertexs = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> pNormals	= new osg::Vec3Array;
	pNewVertexs->resize(total);
	pNormals->push_back(osg::Vec3(1.0, 0.0, 0.0));

	for (int i = 0; i < total; i++)
	{
		const osg::Vec3d& tmpPt = pVertexs->at(i);
		pNewVertexs->at(i).set(tmpPt[0], tmpPt[1], tmpPt[2]);
	}

	osg::ref_ptr<osg::Geometry> pGeo = new osg::Geometry;  // 裙边几何模型实体采用osg原生Geometry就足够
	pGeo->setVertexArray(pNewVertexs.get());
	pGeo->setNormalArray(pNormals.get());
	pGeo->setNormalBinding(osg::Geometry::BIND_OVERALL);
	pGeo->addPrimitiveSet(pVertexIndexs.get());
	osg::ref_ptr<osg::Geode> pGeode = new osg::Geode;
	pGeode->addDrawable(pGeo.get());

	return pGeode.release();
}

osg::Geometry* createExtrusion(osg::Vec3Array*  vertices,
							   const osg::Vec3& direction, float length)
{
	osg::ref_ptr<osg::Vec3Array> newVertices = new osg::Vec3Array;
	newVertices->insert(newVertices->begin(), vertices->begin(),
						vertices->end());

	unsigned int numVertices = vertices->size();
	osg::Vec3	offset		 = direction * length;
	for (osg::Vec3Array::reverse_iterator ritr =
			 vertices->rbegin();
		 ritr != vertices->rend(); ++ritr)
	{
		newVertices->push_back((*ritr) + offset);
	}

	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0));

	osg::ref_ptr<osg::Geometry> extrusion = new osg::Geometry;
	extrusion->setVertexArray(newVertices.get());
	extrusion->setColorArray(colors.get());
	extrusion->setColorBinding(osg::Geometry::BIND_OVERALL);
	extrusion->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON,
												   0, numVertices));
	extrusion->addPrimitiveSet(new osg::DrawArrays(GL_POLYGON,
												   numVertices, numVertices));
	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(
		osgUtil::Tessellator::TESS_TYPE_POLYGONS);
	tessellator.setWindingType(
		osgUtil::Tessellator::TESS_WINDING_ODD);
	tessellator.retessellatePolygons(*extrusion);

	osg::ref_ptr<osg::DrawElementsUInt> sideIndices =
		new osg::DrawElementsUInt(GL_QUAD_STRIP);
	for (unsigned int i = 0; i < numVertices; ++i)
	{
		sideIndices->push_back(i);
		sideIndices->push_back((numVertices - 1 - i) + numVertices);
	}
	sideIndices->push_back(0);
	sideIndices->push_back(numVertices * 2 - 1);
	extrusion->addPrimitiveSet(sideIndices.get());

	osgUtil::SmoothingVisitor::smooth(*extrusion);
	return extrusion.release();
}

osg::Node* CreateCullModel()
{
	//osg::ref_ptr<osg::Group> group = new osg::Group;
	osg::ref_ptr<osg::MatrixTransform> trans = new osg::MatrixTransform;
	
	std::string								   digRoot("0-0-3\\");
	std::string								   digFile;
	std::vector<osg::ref_ptr<osg::Vec3dArray>> masks;
	for (int i = 1; i < 30; i++)
	{
		digFile.assign(digRoot).append(std::to_string(i)).append(".txt"); 
		osg::ref_ptr<osg::Vec3dArray> points = new osg::Vec3dArray;
		test::CReadTestArrayVisitor   r(digFile);
		r.apply(points);
		masks.push_back(points);
	}

	osg::Vec3d origin = masks.front()->front();
	trans->setMatrix(osg::Matrix::translate(origin));

	osg::ref_ptr<osg::Vec3Array> pm;
	osg::ref_ptr<osg::Geode>	 geode;
	osg::Vec3					 direction(0.0f, 0.0f, 1.0f);
	float						 length = 500.0f;
	for (osg::ref_ptr<osg::Vec3dArray>& mask : masks)
	{
		pm = new osg::Vec3Array(mask->size());
		for (size_t i = 0; i < mask->size(); i++)
		{
			pm->at(i).set(osg::Vec3d(mask->at(i).x(), mask->at(i).y(), 1789.0)  - origin);
		}
		geode = new osg::Geode;
		geode->addDrawable(createExtrusion(pm.get(), direction,
										   length));
		trans->addChild(geode.get());
	}

	return trans.release();
}

int main()
{
	// t2
	osg::ref_ptr<osgViewer::Viewer>			   viewer = new osgViewer::Viewer;
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
	traits->x										  = 40;
	traits->y										  = 40;
	traits->width									  = 1024;
	traits->height									  = 800;
	traits->windowDecoration						  = true;
	traits->doubleBuffer							  = true;
	traits->sharedContext							  = 0;

	osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	camera->setGraphicsContext(gc.get());
	camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
	GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
	camera->setDrawBuffer(buffer);
	camera->setReadBuffer(buffer);

	// add this slave camera to the viewer, with a shift left of the projection matrix
	viewer->addSlave(camera.get());
	osg::ref_ptr<osg::Group> root = new osg::Group;

	osg::ref_ptr<osg::Group> cullRoot = new osg::Group;
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
	osg::Node* pTile = CreateTileModel();
	{
		cullRoot->addChild(pTile);
		osg::StateSet* ss = pTile->getOrCreateStateSet();
		ss->setRenderBinDetails(10, "RenderBin");
	}
	osg::Node* pCull = CreateCullModel();
	{
		cullRoot->addChild(pCull);
		osg::StateSet* ss = pCull->getOrCreateStateSet();
		ss->setRenderBinDetails(9, "RenderBin");
		ss->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
		ss->setMode(GL_DEPTH_WRITEMASK, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	}

	root->addChild(cullRoot);

	viewer->setSceneData(root.get());
	viewer->realize();
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->addEventHandler(new osgViewer::StatsHandler);  //状态信息
	return viewer->run();
}