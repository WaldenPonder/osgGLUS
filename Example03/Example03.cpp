// Example03.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//#undef  max
//#undef min

#include "pch.h"
#include "../common/common.h"
#include <random>
#include <osg/io_utils>

osg::Geode* createTriangle()
{
	std::default_random_engine eng(time(NULL));
	std::uniform_real_distribution<> urd(0., 1.);
	
	osg::Geode* geode = new osg::Geode;

	std::vector<osg::Vec3d> PTs;
	PTs.push_back(osg::Vec3());
	PTs.push_back(osg::Vec3(100, 0, 0));
	PTs.push_back(osg::Vec3(0, 100, 0));
	osg::Geometry* geom = createLine(PTs, osg::Vec4(1, 0, 0,1), osg::PrimitiveSet::LINE_LOOP);
	geode->addDrawable(geom);

	std::vector<osg::Vec3d> PTs2;
	for (int i = 0; i < 1000000; i++)
	{
		double r1 = urd(eng);
		double r2 = urd(eng);
		double sqrt_r1 = std::sqrt(r1);
		
		double u = 1 - sqrt_r1;
		double v = r2 * sqrt_r1;
		double w = 1 - u - v;

		osg::Vec3d p = PTs[0] * u + PTs[1] * v + PTs[2] * w;
		PTs2.push_back(p);
	}

	geom = createLine(PTs2, osg::Vec4(0, 1, 1, 1), osg::PrimitiveSet::POINTS);
	geode->addDrawable(geom);

	configureShaders(geode->getOrCreateStateSet());
	return geode;
}

osg::Matrix mat;

void next(int ox, int oy, int len, int type)
{
	if (ox < 0 || ox > len || oy < 0 || oy > len)
	{
		return;
	}

	if (type == 0) //right
	{
		int c;
		for (c = oy; c < oy + len - 1; c++)
		{
			cout << mat(ox, c) << "\t";
		}
		cout << "\n";

		next(ox, c-1, len, 1);
	}
	else if(type == 1)
	{
		int r;
		for (r = ox; r < ox + len - 1; r++)
		{
			cout << mat(r, oy) << "\t";
		}
		cout << "\n";
		next(r-1, oy, len, 2);
	}
	else if(type == 2)
	{
		int c;
		for (c = ox; c > ox - len + 1; c--)
		{
			cout << mat(ox, c) << "\t";
		}
		cout << "\n";
		next(ox, c + 1, len, 3);
	}
	else if(type == 3)
	{
		int r;
		for (r = ox; r > ox - len + 1; r--)
		{
			cout << mat(r, oy) << "\t";
		}
		cout << "loop end \n\n";
		next(r + 2, oy + 1, len - 2, 0);
		
	}
}


osg::Node* test()
{
	string path = "F:\\FileRecv\\汽车";

	osg::Node* n = osgDB::readNodeFile(path + "/汽车.fbx");

	osg::Program* p = new osg::Program;

	p->addShader(osgDB::readShaderFile(shader_dir() + "/car.vert"));
	p->addShader(osgDB::readShaderFile(shader_dir() + "/car.frag"));
	n->getOrCreateStateSet()->setAttributeAndModes(p, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
		
	auto create_texture2d = [](const string& img_path) {
		osg::Image* img = osgDB::readImageFile(img_path);
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(img);
		texture->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
		texture->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
		texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
		texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
		return texture;
	};

	int id = 0;
	n->getOrCreateStateSet()->setTextureAttributeAndModes(id, create_texture2d(path + "/Bangkok_GenericSaloon2_White_DIFF.tga"));
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("baseTexture", id)); id += 1;
		
	n->getOrCreateStateSet()->setTextureAttributeAndModes(id, create_texture2d(path + "/Bangkok_GenericSaloon2_S.tga"));
	n->getOrCreateStateSet()->addUniform(new osg::Uniform("specularTexture", id)); id += 1;

	//n->getOrCreateStateSet()->setTextureAttributeAndModes(id, ibl->irradiance_map);
	//n->getOrCreateStateSet()->addUniform(new osg::Uniform("irradiance_map", id));

	//id += 1;
	//n->getOrCreateStateSet()->setTextureAttributeAndModes(id, ibl->prefilter_map);
	//n->getOrCreateStateSet()->addUniform(new osg::Uniform("prefilter_map", id));

	//id += 1;
	//n->getOrCreateStateSet()->setTextureAttributeAndModes(id, ibl->brdf_map);
	//n->getOrCreateStateSet()->addUniform(new osg::Uniform("brdf_map", id));

	return n;
}


osg::Vec3 rot(osg::Vec3 u, float theta, float phi)
{
	float tmp = sqrt(1 - u.z() * u.z());
	float x = u.x() * cos(theta) + sin(theta) * (u.x() * u.z() * cos(phi) - u.y() * sin(phi)) / tmp;
	float y = u.y() * cos(theta) + sin(theta) * (u.y()*u.z()*cos(phi) + u.x()*sin(phi)) / tmp;
	float z = u.z() * cos(theta) - tmp * sin(theta) * cos(phi);

	return osg::Vec3(x, y, z);
}

osg::Node* test2()
{
	osg::Group* root = new osg::Group;
	osg::Vec3 p1(0, 0, 0);
	osg::Vec3 p2(1, 0, 0);

	root->addChild(createLine({ p1, p2 }, osg::Vec4(1, 0, 0, 1), osg::PrimitiveSet::LINES));
	root->addChild(createLine({ p1, rot(p2, osg::PI_2, osg::PI_4) }, osg::Vec4(1, 0, 1, 1), osg::PrimitiveSet::LINES));

	return root;
}

int main(int argc, char** argv)
{

	// construct the viewer.
	osgViewer::Viewer viewer;

	osg::Group* root = new osg::Group;
	//root->addChild(osgDB::readNodeFile("cow.osg"));
	
	//root->addChild(createTriangle());
	root->addChild(test2());
	add_event_handler(viewer);
	viewer.setSceneData(root);
	viewer.realize();
	
	osg::setNotifyLevel(osg::NotifySeverity::WARN);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);
	//viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);
	return viewer.run();
}
