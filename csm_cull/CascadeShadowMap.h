#pragma once
#include <osg/Camera>
#include <osg/Material>
#include <osg/Depth>
#include <osg/ClipPlane>

#include <osgShadow/ShadowTechnique>

//copy from osgShadow::ParallelSplitShadowMap
class CascadeShadowMap : public osg::Referenced
{
 public:
	struct Param
	{
		//size == _number_of_splits
		std::vector<std::string> shadow_mat_names;
		std::vector<std::string> far_distance_split_names;
		std::vector<std::string> shadow_texture_names;

		int _resolution		  = 4096;
		int _number_of_splits = 3;

		osg::ref_ptr<osg::Camera> mainCamera;
		osg::ref_ptr<osg::Group>  root;
		osg::BoundingBox		  bbTerrain;
	};

	CascadeShadowMap(Param& param);

	/** Get the texture resolution */
	inline unsigned int getTextureResolution() const { return _resolution; }

	/** Set the max far distance */
	inline void setMaxFarDistance(double farDist)
	{
		_setMaxFarDistance	 = farDist;
		_isSetMaxFarDistance = true;
	}

	/** Get the max far distance */
	inline double getMaxFarDistance() const { return _setMaxFarDistance; }

	/** Set the factor for moving the virtual camera behind the real camera*/
	inline void setMoveVCamBehindRCamFactor(double distFactor) { _move_vcam_behind_rcam_factor = distFactor; }

	/** Get the factor for moving the virtual camera behind the real camera*/
	inline double getMoveVCamBehindRCamFactor() const { return _move_vcam_behind_rcam_factor; }

	/** Set min near distance for splits */
	inline void setMinNearDistanceForSplits(double nd) { _split_min_near_dist = nd; }

	/** Get min near distance for splits */
	inline double getMinNearDistanceForSplits() const { return _split_min_near_dist; }

	/** set a user defined light for shadow simulation (sun light, ... )
		 *    when this light get passed to pssm, the scene's light are no longer collected
		 *    and simulated. just this user passed light, it needs to be a directional light.
		 */
	inline void setLight(osg::Light* light) { _light = light; }

	/** get the user defined light for shadow simulation */
	inline const osg::Light* getLight() const { return _light.get(); }

	enum SplitCalcMode
	{
		SPLIT_LINEAR,
		SPLIT_EXP
	};

	/** set split calculation mode */
	inline void setSplitCalculationMode(SplitCalcMode scm = SPLIT_EXP) { _SplitCalcMode = scm; }

	/** get split calculation mode */
	inline SplitCalcMode getSplitCalculationMode() const { return _SplitCalcMode; }

	//添加需要显示阴影的节点
	void addNode(osg::Node* n);

	osg::Group* shadowedScene() const { return shadowedScene_; }

	osg::Texture2D* getTexture(int index);

	/*
	  1> lastTextureunit返回为当前使用到的最后一个纹理单元， 等于 firstTextureunit + _number_of_splits - 1
	*/
	void applyStateset(osg::StateSet* ss, const int firstTextureunit, int& lastTextureunit);

	static void applyJittering(osg::StateSet* ss, unsigned int unit);

 public:
	virtual ~CascadeShadowMap() {}

	struct PSSMShadowSplitTexture
	{
		osg::ref_ptr<osg::Camera>	 _camera;
		osg::ref_ptr<osg::Texture2D> _texture;

		double _split_far;

		// Light (SUN)
		osg::Vec3d _lightCameraSource;
		osg::Vec3d _lightCameraTarget;
		osg::Vec3d _frustumSplitCenter;
		osg::Vec3d _lightDirection;
		double	   _lightNear;
		double	   _lightFar;

		osg::Matrix _cameraView;
		osg::Matrix _cameraProj;

		unsigned int _splitID;
		unsigned int _resolution;

		osg::ref_ptr<osg::Uniform> u_far_distance_split;
		osg::ref_ptr<osg::Uniform> u_shadow_mat;
	};

 private:
	friend class MainCameraCullingCallback;
	void calculateFrustumCorners(PSSMShadowSplitTexture& pssmShadowSplitTexture, osg::Vec3d* frustumCorners);
	void calculateLightInitialPosition(PSSMShadowSplitTexture& pssmShadowSplitTexture, osg::Vec3d* frustumCorners);
	void calculateLightNearFarFormFrustum(PSSMShadowSplitTexture& pssmShadowSplitTexture, osg::Vec3d* frustumCorners);
	void calculateLightViewProjectionFormFrustum(PSSMShadowSplitTexture& pssmShadowSplitTexture, osg::Vec3d* frustumCorners);

	void init();

 private:
	typedef std::map<unsigned int, PSSMShadowSplitTexture> PSSMShadowSplitTextureMap;
	PSSMShadowSplitTextureMap							   _PSSMShadowSplitTextureMap;

	unsigned int _number_of_splits;

	unsigned int _resolution;

	double _setMaxFarDistance;
	bool   _isSetMaxFarDistance;

	double _split_min_near_dist;

	double _move_vcam_behind_rcam_factor;

	osg::ref_ptr<osg::Light> _light;

	bool		  _GLSL_shadow_filtered;
	SplitCalcMode _SplitCalcMode;

	osg::ref_ptr<osg::Group> shadowedScene_;
	Param					 param_;
};
