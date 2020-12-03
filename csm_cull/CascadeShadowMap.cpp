#include "pch.h"
#include "CascadeShadowMap.h"
#include <osgShadow/ShadowedScene>
#include <osg/Notify>
#include <osg/ComputeBoundsVisitor>
#include <osg/PolygonOffset>
#include <osg/CullFace>
#include <osg/io_utils>
#include <iostream>
#include <sstream>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ReadFile>
#include <osg/Texture1D>
#include <osg/Depth>
#include <osg/ShadeModel>
#include <osgViewer/Viewer>
#include <osgUtil/PlaneIntersector>
#include <osg/Texture3D>
#include <random>
#include "DrawBoundingBox.h"
#include <thread>
#include <mutex>

#define ZNEAR_MIN_FROM_LIGHT_SOURCE 15.0
#define MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR 10.0

//////////////////////////////////////////////////////////////////////////
// clamp variables of any type
template <class Type>
inline Type Clamp(Type A, Type Min, Type Max)
{
	if (A < Min) return Min;
	if (A > Max) return Max;
	return A;
}

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

std::mutex mutex;

osg::Matrixd g_view, g_proj;

class FirstCameraPredrawCallback : public osg::Camera::DrawCallback
{
 public:
	FirstCameraPredrawCallback() {}
	virtual void operator()(osg::RenderInfo& renderInfo) const
	{
		osg::Camera*				 camera = renderInfo.getCurrentCamera();
		osg::ref_ptr<osg::RefMatrix> proMat = new osg::RefMatrix(camera->getProjectionMatrix());
		renderInfo.getState()->applyProjectionMatrix(proMat);
		renderInfo.getState()->applyModelViewMatrix(camera->getViewMatrix());

		osg::Matrixd m2 = camera->getProjectionMatrix();

		if (m2 != g_view)
		{
			std::cout << "AAA";
		}

		osg::Matrixd m = camera->getProjectionMatrix();
		if (m != g_proj)
		{
			std::cout << "BBB";
		}

		{
			std::lock_guard<std::mutex> lg(mutex);
			auto						num = renderInfo.getView()->getFrameStamp()->getFrameNumber();
			std::cout << "AA1:  " << num << "\t"
					  << "   time: " << clock() << "\t" << camera->getViewMatrix() << std::endl;
			std::cout << "AA2:  " << num << "\t"
					  << "   time: " << clock() << "\t" << camera->getProjectionMatrix() << std::endl;
		}

		for (auto csm : csmArray_)
		{
			osg::Vec3 lightDirection;

			if (csm->getLight())
			{
				osg::Vec4 dir = csm->getLight()->getPosition();
				lightDirection.set(dir.x(), dir.y(), dir.z());
			}
			else
			{
				OSG_WARN << ("CascadeShadowMap 没设置光照, 如果不是一直报，说明light是延迟设置\n");
				continue;
			}

			//std::cout << "\n";

			for (auto it = csm->map_.begin(); it != csm->map_.end(); it++)
			{
				CascadeShadowMap::PSSMShadowSplitTexture info = it->second;
				lightDirection.normalize();
				info._lightDirection = -lightDirection;
				info._cameraView	 = mainCamera_->getViewMatrix();
				info._cameraProj	 = mainCamera_->getProjectionMatrix();

				// Calculate corner points of frustum split
				//
				// To avoid edge problems, scale the frustum so
				// that it's at least a few pixels larger
				//
				osg::Vec3d pCorners[8];
				csm->calculateFrustumCorners(info, pCorners);

				// Init Light (Directional Light)
				//
				csm->calculateLightInitialPosition(info, pCorners);

				// Calculate near and far for light view
				//
				csm->calculateLightNearFarFormFrustum(info, pCorners);

				// Calculate view and projection matrices
				//
				csm->calculateLightViewProjectionFormFrustum(info, pCorners);
			}
		}
	}

	std::vector<osg::ref_ptr<CascadeShadowMap>> csmArray_;
	osg::ref_ptr<osg::Camera>					mainCamera_;
};

class MainCameraCullingCallback : public osg::NodeCallback
{
 public:
	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
#if 0
		{
			auto						num = nv->getFrameStamp()->getFrameNumber();
			std::lock_guard<std::mutex> lg(mutex);
			std::cout << "BB1:  " << num << "\t"
					  << "   time: " << clock() << "\t" << mainCamera_->getViewMatrix() << std::endl;
			std::cout << "BB2:  " << num << "\t"
					  << "   time: " << clock() << "\t" << mainCamera_->getProjectionMatrix() << std::endl;

			g_view = mainCamera_->getViewMatrix();
			g_proj = mainCamera_->getProjectionMatrix();
		}
#endif

		for (auto csm : csmArray_)
		{
			osg::Vec3 lightDirection;

			if (csm->getLight())
			{
				osg::Vec4 dir = csm->getLight()->getPosition();
				lightDirection.set(dir.x(), dir.y(), dir.z());
			}
			else
			{
				OSG_WARN << ("CascadeShadowMap 没设置光照, 如果不是一直报，说明light是延迟设置\n");
				continue;
			}

			//std::cout << "\n";

			for (auto it = csm->map_.begin(); it != csm->map_.end(); it++)
			{
				CascadeShadowMap::PSSMShadowSplitTexture info = it->second;
				lightDirection.normalize();
				info._lightDirection = -lightDirection;
				info._cameraView	 = mainCamera_->getViewMatrix();
				info._cameraProj	 = mainCamera_->getProjectionMatrix();

				// Calculate corner points of frustum split
				//
				// To avoid edge problems, scale the frustum so
				// that it's at least a few pixels larger
				//
				osg::Vec3d pCorners[8];
				csm->calculateFrustumCorners(info, pCorners);

				// Init Light (Directional Light)
				//
				csm->calculateLightInitialPosition(info, pCorners);

				// Calculate near and far for light view
				//
				csm->calculateLightNearFarFormFrustum(info, pCorners);

				// Calculate view and projection matrices
				//
				csm->calculateLightViewProjectionFormFrustum(info, pCorners);
			}
		}

		node->traverse(*nv);
	}

	std::vector<osg::ref_ptr<CascadeShadowMap>> csmArray_;
	osg::ref_ptr<osg::Camera>					mainCamera_;
};

//////////////////////////////////////////////////////////////////////////
CascadeShadowMap::CascadeShadowMap(Param& param)
	: _setMaxFarDistance(1000.0),
	  _isSetMaxFarDistance(false),
	  _split_min_near_dist(ZNEAR_MIN_FROM_LIGHT_SOURCE),
	  _move_vcam_behind_rcam_factor(MOVE_VIRTUAL_CAMERA_BEHIND_REAL_CAMERA_FACTOR),
	  _light(NULL),
	  _GLSL_shadow_filtered(true),
	  param_(param)
{
	shadowedScene_	  = new osg::Group;
	_number_of_splits = param._number_of_splits;
	_resolution		  = param._resolution;

	setSplitCalculationMode(SPLIT_EXP);
	init();

	osg::Camera* mainCamera = param.mainCamera;

#if 0
	FirstCameraPredrawCallback* callback = dynamic_cast<FirstCameraPredrawCallback*>(mainCamera->getUserData());

	if (!callback)
	{
		callback = new FirstCameraPredrawCallback;
		mainCamera->setUserData(callback);
		callback->mainCamera_ = mainCamera;
		mainCamera->addPreDrawCallback(callback);
	}

#else

	MainCameraCullingCallback* callback = dynamic_cast<MainCameraCullingCallback*>(mainCamera->getUserData());
	if (!callback)
	{
		callback = new MainCameraCullingCallback;
		mainCamera->setUserData(callback);
		callback->mainCamera_ = mainCamera;
		mainCamera->addCullCallback(callback);
	}

#endif

	callback->csmArray_.push_back(this);
}

void CascadeShadowMap::init()
{
	for (unsigned int i = 0; i < _number_of_splits; i++)
	{
		PSSMShadowSplitTexture info;
		info._splitID	 = i;
		info._resolution = _resolution;

		// set up the texture to render into
		info._texture = new osg::Texture2D;
		info._texture->setTextureSize(info._resolution, info._resolution);
		info._texture->setInternalFormat(GL_DEPTH_COMPONENT);
		info._texture->setSourceFormat(GL_DEPTH_COMPONENT);
		info._texture->setSourceType(GL_FLOAT);
#if 1
		info._texture->setShadowComparison(true);
		info._texture->setShadowCompareFunc(osg::Texture::LEQUAL);
		info._texture->setShadowTextureMode(osg::Texture2D::LUMINANCE);
#endif
		info._texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
		info._texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
		info._texture->setBorderColor(osg::Vec4(1.0, 1.0, 1.0, 1.0));
		info._texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
		info._texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);

		// create the camera
		info._camera = new osg::Camera;
		param_.root->addChild(info._camera);

		info._camera->setReadBuffer(GL_NONE);
		info._camera->setDrawBuffer(GL_NONE);
		info._camera->addChild(shadowedScene_);
		info._camera->setClearMask(GL_DEPTH_BUFFER_BIT);
		info._camera->setClearColor(osg::Vec4());
		info._camera->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

		// set view
		info._camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

		// set the camera to render before the main camera.
		info._camera->setRenderOrder(osg::Camera::PRE_RENDER);

		// set viewport
		info._camera->setViewport(0, 0, info._resolution, info._resolution);

		// tell the camera to use OpenGL frame buffer object where supported.
		info._camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

		// attach the texture and use it as the color buffer.
		info._camera->attach(osg::Camera::DEPTH_BUFFER, info._texture.get());

		osg::StateSet* stateset = info._camera->getOrCreateStateSet();

		std::string name		  = param_.far_distance_split_names[i];
		info.u_far_distance_split = new osg::Uniform(name.c_str(), 1.0f);

		std::string name2 = param_.shadow_mat_names[i];
		info.u_shadow_mat = new osg::Uniform(osg::Uniform::FLOAT_MAT4, name2.c_str());

		map_.insert(PSSMShadowSplitTextureMap::value_type(i, info));
	}
}

//////////////////////////////////////////////////////////////////////////
// Computes corner points of a frustum
//
//
//unit box representing frustum in clip space
const osg::Vec3d const_pointFarTR(1.0, 1.0, 1.0);
const osg::Vec3d const_pointFarBR(1.0, -1.0, 1.0);
const osg::Vec3d const_pointFarTL(-1.0, 1.0, 1.0);
const osg::Vec3d const_pointFarBL(-1.0, -1.0, 1.0);
const osg::Vec3d const_pointNearTR(1.0, 1.0, -1.0);
const osg::Vec3d const_pointNearBR(1.0, -1.0, -1.0);
const osg::Vec3d const_pointNearTL(-1.0, 1.0, -1.0);
const osg::Vec3d const_pointNearBL(-1.0, -1.0, -1.0);

void CascadeShadowMap::addNode(osg::Node* n)
{
	shadowedScene_->addChild(n);
}

osg::Texture2D* CascadeShadowMap::getTexture(int index)
{
	return map_[index]._texture;
}

void CascadeShadowMap::applyStateset(osg::StateSet* ss, const int firstTextureunit, int& lastTextureunit)
{
	lastTextureunit = firstTextureunit + map_.size() - 1;

	for (int i = 0; i < map_.size(); i++)
	{
		std::string name = param_.shadow_texture_names[i];
		ss->setTextureAttributeAndModes(i + firstTextureunit, getTexture(i));
		ss->addUniform(new osg::Uniform(name.c_str(), i + firstTextureunit));

		ss->addUniform(map_[i].u_far_distance_split);
		ss->addUniform(map_[i].u_shadow_mat);
	}
}

// Return random float between -0.5 and 0.5
static float jitter()
{
	static std::default_random_engine			 generator;
	static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);
	return distrib(generator);
}

void CascadeShadowMap::applyJittering(osg::StateSet* ss, unsigned int unit)
{
	const int	SAMPLESU	  = 4;
	const int	SAMPLESV	  = 8;
	const int	JITTERMAPSIZE = 8;
	const float RADIUS		  = 7.0f;

	// create a 3D texture with hw mipmapping
	osg::Texture3D* texture = new osg::Texture3D;
	texture->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture3D::NEAREST);
	texture->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture3D::NEAREST);
	texture->setWrap(osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT);
	texture->setWrap(osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT);
	texture->setWrap(osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT);
	texture->setUseHardwareMipMapGeneration(true);
	ss->setUserData(texture);
	const unsigned int size	 = 16;
	const unsigned int gridW = 8;
	const unsigned int gridH = 8;
	unsigned int	   R	 = (gridW * gridH / 2);
	texture->setTextureSize(size, size, R);

	// then create the 3d image to fill with jittering data
	osg::Image*	   image   = new osg::Image;
	unsigned char* data	   = new unsigned char[size * size * R * 4];
	int			   samples = SAMPLESU * SAMPLESV;

	for (unsigned int i = 0; i < size; ++i)
	{
		for (unsigned int j = 0; j < size; ++j)
		{
			for (unsigned int k = 0; k < R; ++k)
			{
				int x1, y1, x2, y2;
				x1 = k % (SAMPLESU);
				y1 = (samples - 1 - k) / SAMPLESU;
				x2 = (k + 1) % SAMPLESU;
				y2 = (samples - 1 - k - 1) / SAMPLESU;

				osg::Vec4 v;
				// Center on grid and jitter
				v.x() = (x1 + 0.5f) + jitter();
				v.y() = (y1 + 0.5f) + jitter();
				v.z() = (x2 + 0.5f) + jitter();
				v.w() = (y2 + 0.5f) + jitter();

				// Scale between 0 and 1
				v.x() /= SAMPLESU;
				v.y() /= SAMPLESV;
				v.z() /= SAMPLESU;
				v.w() /= SAMPLESV;

				// Warp to disk
				int cell	   = ((k / 2) * size * size + j * size + i) * 4;
				data[cell + 0] = sqrtf(v.y()) * cosf(2 * osg::PI * v.x());
				data[cell + 1] = sqrtf(v.y()) * sinf(2 * osg::PI * v.x());
				data[cell + 2] = sqrtf(v.w()) * cosf(2 * osg::PI * v.z());
				data[cell + 3] = sqrtf(v.w()) * sinf(2 * osg::PI * v.z());
			}
		}
	}

	image->setImage(size, size, R, GL_RGBA4, GL_RGBA, GL_FLOAT, data, osg::Image::USE_NEW_DELETE);

	texture->setImage(image);

	ss->setTextureAttributeAndModes(unit, texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
	ss->setTextureMode(unit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
	ss->setTextureMode(unit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
	ss->setTextureMode(unit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
}

void get_far_by_terrain_boundingbox(CascadeShadowMap::Param& param, double camFar, float& my_far)
{
	osg::BoundingBox bbTerrain	= param.bbTerrain;
	osg::Camera*	 mainCamera = param.mainCamera;

	osg::BoundingBox bb;
	for (int i = 0; i < 8; i++)
	{
		auto p = bbTerrain.corner(i) * mainCamera->getViewMatrix() * mainCamera->getProjectionMatrix();
		bb.expandBy(p);
	}

	DrawBoundingBox drawBB;
	drawBB.add(bbTerrain);

	osg::ref_ptr<osg::Geode> geode = drawBB.getOrCreate();

	std::vector<osg::Vec3d> intersectionPTs;

	osg::Polytope polytope;
	polytope.setToUnitFrustum();
	polytope.transformProvidingInverse((mainCamera->getViewMatrix() * mainCamera->getProjectionMatrix()));

	for (osg::Plane& p : polytope.getPlaneList())
	{
		osgUtil::IntersectionVisitor			visitor;
		osg::ref_ptr<osgUtil::PlaneIntersector> intersector = new osgUtil::PlaneIntersector(p);
		visitor.setIntersector(intersector);
		geode->accept(visitor);

		osgUtil::PlaneIntersector::Intersections& intersections = intersector->getIntersections();

		for (osgUtil::PlaneIntersector::Intersection& intersection : intersections)
		{
			intersectionPTs.insert(intersectionPTs.end(), intersection.polyline.begin(), intersection.polyline.end());
		}
	}

	for (int i = 0; i < 8; i++)
	{
		intersectionPTs.push_back(bbTerrain.corner(i));
	}

	for (osg::Vec3d& p : intersectionPTs)
	{
		osg::Vec4d pt  = osg::Vec4d(p, 1) * mainCamera->getViewMatrix();
		osg::Vec4d pt2 = pt * mainCamera->getProjectionMatrix();

		if (pt.z() < 0)
		{
			my_far = max(my_far, fabs(pt.z()));
		}
	}
}

void CascadeShadowMap::calculateFrustumCorners(PSSMShadowSplitTexture& info, osg::Vec3d* frustumCorners)
{
	// get user cameras
	double fovy, aspectRatio, camNear, camFar;
	info._cameraProj.getPerspective(fovy, aspectRatio, camNear, camFar);

	//TH_INFO("NEAR FAR: {0}, {1}\n", camNear, camFar);

	// force to max far distance to show shadow, for some scene it can be solve performance problems.
	if ((_isSetMaxFarDistance) && (_setMaxFarDistance < camFar))
		camFar = _setMaxFarDistance;

	// build camera matrix with some offsets (the user view camera)
	osg::Matrixd viewMat;
	osg::Vec3d	 camEye, camCenter, camUp;
	info._cameraView.getLookAt(camEye, camCenter, camUp);
	osg::Vec3d viewDir = camCenter - camEye;
	//viewDir.normalize(); //we can assume that viewDir is still normalized in the viewMatrix
	camEye = camEye - viewDir * _move_vcam_behind_rcam_factor;
	camFar += _move_vcam_behind_rcam_factor * viewDir.length();
	viewMat.makeLookAt(camEye, camCenter, camUp);

	float my_far = -FLT_MAX;
	get_far_by_terrain_boundingbox(param_, camFar, my_far);

	if (my_far > 100)
		camFar = min(camFar, my_far);

	/// CALCULATE SPLIT
	double maxFar = camFar;
	// double minNear = camNear;
	double camNearFar_Dist = maxFar - camNear;
	if (_SplitCalcMode == SPLIT_LINEAR)
	{
		camFar	= camNear + (camNearFar_Dist) * (( double )(info._splitID + 1)) / (( double )(_number_of_splits));
		camNear = camNear + (camNearFar_Dist) * (( double )(info._splitID)) / (( double )(_number_of_splits));
	}
	else
	{
		// Exponential split scheme:
		//
		// Ci = (n - f)*(i/numsplits)^(bias+1) + n;
		//
		static double fSplitSchemeBias[2] = { 0.25f, 0.66f };
		fSplitSchemeBias[1]				  = Clamp(fSplitSchemeBias[1], 0.0, 3.0);
		double* pSplitDistances			  = new double[_number_of_splits + 1];

		for (int i = 0; i < ( int )_number_of_splits; i++)
		{
			double fIDM		   = ( double )(i) / ( double )(_number_of_splits);
			pSplitDistances[i] = camNearFar_Dist * (pow(fIDM, fSplitSchemeBias[1] + 1)) + camNear;
		}
		// make sure border values are right
		pSplitDistances[0]				   = camNear;
		pSplitDistances[_number_of_splits] = camFar;

		camNear = pSplitDistances[info._splitID];
		camFar	= pSplitDistances[info._splitID + 1];

		delete[] pSplitDistances;
	}

	info._split_far = camFar;

	//////////////////////////////////////////////////////////////////////////
	/// TRANSFORM frustum corners (Optimized for Orthogonal)

	osg::Matrixd projMat;
	projMat.makePerspective(fovy, aspectRatio, camNear, camFar);
	osg::Matrixd projViewMat(viewMat * projMat);
	osg::Matrixd invProjViewMat;
	invProjViewMat.invert(projViewMat);

	//transform frustum vertices to world space
	frustumCorners[0] = const_pointFarBR * invProjViewMat;
	frustumCorners[1] = const_pointNearBR * invProjViewMat;
	frustumCorners[2] = const_pointNearTR * invProjViewMat;
	frustumCorners[3] = const_pointFarTR * invProjViewMat;
	frustumCorners[4] = const_pointFarTL * invProjViewMat;
	frustumCorners[5] = const_pointFarBL * invProjViewMat;
	frustumCorners[6] = const_pointNearBL * invProjViewMat;
	frustumCorners[7] = const_pointNearTL * invProjViewMat;

	//std::cout << "camFar : "<<pssmShadowSplitTexture._splitID << " / " << camNear << "," << camFar << std::endl;
}

// compute directional light initial position;
void CascadeShadowMap::calculateLightInitialPosition(PSSMShadowSplitTexture& info, osg::Vec3d* frustumCorners)
{
	info._frustumSplitCenter = frustumCorners[0];
	for (int i = 1; i < 8; i++)
	{
		info._frustumSplitCenter += frustumCorners[i];
	}
	//    pssmShadowSplitTexture._frustumSplitCenter /= 8.0;
	info._frustumSplitCenter *= 0.125;
}

void CascadeShadowMap::calculateLightNearFarFormFrustum(
	PSSMShadowSplitTexture& info,
	osg::Vec3d*				frustumCorners)
{

	//calculate near, far
	double zFar(-DBL_MAX);

	// calculate zFar (as longest distance)
	for (int i = 0; i < 8; i++)
	{
		double dist_z_from_light = fabs(info._lightDirection * (frustumCorners[i] - info._frustumSplitCenter));
		if (zFar < dist_z_from_light) zFar = dist_z_from_light;
	}

	// update camera position and look at center
	info._lightCameraSource = info._frustumSplitCenter - info._lightDirection * (zFar + _split_min_near_dist);
	info._lightCameraTarget = info._frustumSplitCenter + info._lightDirection * (zFar);

	// calculate [zNear,zFar]
	zFar = (-DBL_MAX);
	double zNear(DBL_MAX);
	for (int i = 0; i < 8; i++)
	{
		double dist_z_from_light = fabs(info._lightDirection * (frustumCorners[i] - info._lightCameraSource));
		if (zFar < dist_z_from_light) zFar = dist_z_from_light;
		if (zNear > dist_z_from_light) zNear = dist_z_from_light;
	}
	// update near - far plane
	info._lightNear = max(zNear - _split_min_near_dist - 0.01, 0.01);
	info._lightFar	= zFar;
}

void CascadeShadowMap::calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture& info, osg::Vec3d* frustumCorners)
{
	// calculate the camera's coordinate system
	osg::Vec3d camEye, camCenter, camUp;
	info._cameraView.getLookAt(camEye, camCenter, camUp);
	osg::Vec3d viewDir(camCenter - camEye);
	osg::Vec3d camRight(viewDir ^ camUp);

	// we force to have normalized vectors (camera's view)
	camUp.normalize();
	viewDir.normalize();
	camRight.normalize();

	// use quaternion -> numerical more robust
	osg::Quat qRot;
	qRot.makeRotate(viewDir, info._lightDirection);
	osg::Vec3d top	 = qRot * camUp;
	osg::Vec3d right = qRot * camRight;

	// calculate the camera's frustum right,right,bottom,top parameters
	double maxRight(-DBL_MAX), maxTop(-DBL_MAX);
	double minRight(DBL_MAX), minTop(DBL_MAX);

	for (int i(0); i < 8; i++)
	{
		osg::Vec3d diffCorner(frustumCorners[i] - info._frustumSplitCenter);
		double	   lright(diffCorner * right);
		double	   lTop(diffCorner * top);

		if (lright > maxRight) maxRight = lright;
		if (lTop > maxTop) maxTop = lTop;

		if (lright < minRight) minRight = lright;
		if (lTop < minTop) minTop = lTop;
	}

	//info._lightCameraSource.z() = 1000;
	//info._lightCameraTarget = info._lightCameraSource - osg::Vec3(0, 0, 1);
	osg::Matrixd viewMat = osg::Matrixd::lookAt(info._lightCameraSource, info._lightCameraTarget, top);
	// make the camera view matrix
	info._camera->setViewMatrix(viewMat);
	float factor = 1.0;
	//std::cout << "EYE: " << clock() << info._lightCameraSource << "\t" << info._lightCameraTarget << "\n";
	osg::Matrixd projMat = osg::Matrixd::ortho(minRight * factor, maxRight * factor, minTop * factor, maxTop * factor, info._lightNear, info._lightFar);
	//projMat = osg::Matrixd::ortho(-9156.33 ,   9316.54 ,- 5722.19,    5671.82 ,   87.1397  , 8924.03);
	//std::cout << "NF:\t" << clock() << "\t"  << minRight << "    "<<  maxRight << "    " <<  minTop << "    "
	//	<< maxTop << "    " << info._lightNear << "\t" << info._lightFar << "\n";
	//projMat = osg::Matrix::perspective(60, 1, info._lightNear, info._lightFar);
	// use ortho projection for light (directional light only supported)
	info._camera->setProjectionMatrix(projMat);

	// get user cameras
	osg::Vec3d vProjCamFraValue = (camEye + viewDir * info._split_far) * (info._cameraView * info._cameraProj);
	info.u_far_distance_split->set(( float )vProjCamFraValue.z());

	osg::Matrix mat = osg::Matrixd::inverse(param_.mainCamera->getViewMatrix()) * viewMat * projMat;
	info.u_shadow_mat->set(mat);
}