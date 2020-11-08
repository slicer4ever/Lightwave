#ifndef SCENE_H
#define SCENE_H
#include "Model.h"
#include "Skeleton.h"
#include "Renderer.h"

struct SceneNode {
	LWEGLTFAnimTween m_Animation;
	uint32_t m_ModelID;
	uint32_t m_SkeletonID;
	uint32_t m_ParentID;
	std::vector<uint32_t> m_Children;
};

class Camera;

class Scene {
public:
	enum {
		MaxModels = 1024,
		MaxMaterials = 1024,
		MaxSkeletons = 1024,
		MaxNodes = 1024,
		MaxTextures = 1024,
		MaxImages = 1024,
		MaxLights = 1024
	};

	static Scene *LoadGLTF(const LWUTF8Iterator &Path, LWVideoDriver *Driver, LWAllocator &Allocator);

	Scene &Update(float deltaTime);

	uint32_t DrawScene(uint64_t lCurrentTime, const LWMatrix4f &SceneTransform, Renderer *R, lFrame &F, Camera &C, LWVideoDriver *Driver);

	uint32_t DrawDebugSkeleton(Renderer *R, lFrame &F, Skeleton &Skel, const ModelData &SourceMdl, LWVideoDriver *Driver);

	bool PushModel(Model &M);

	bool PushImage(LWTexture *Image);

	bool PushMaterial(const LWEGLTFMaterial &Mat);

	bool PushSkeleton(Skeleton &Skel);

	bool PushLight(const Light &L);

	bool PushNode(SceneNode &N);

	bool PushTexture(LWEGLTFTexture &Tex);

	void TransformAABB(LWVector3f &AAMin, LWVector3f &AAMax, const LWMatrix4f &Transform, bool RebuildSceneAABB);

	Model &GetModel(uint32_t i);

	LWEGLTFMaterial &GetMaterial(uint32_t i);

	SceneNode &GetNode(uint32_t i);

	LWVector3f GetAAMin(void) const;

	LWVector3f GetAAMax(void) const;

	Skeleton &GetSkeleton(uint32_t i);

	Light &GetLight(uint32_t i);

	LWTexture *GetImage(uint32_t i);

	LWEGLTFTexture &GetTexture(uint32_t i);

	uint32_t GetModelCount(void) const;

	uint32_t GetMaterialCount(void) const;

	uint32_t GetSkeletonCount(void) const;

	uint32_t GetNodeCount(void) const;

	uint32_t GetLightCount(void) const;

	uint32_t GetTextureCount(void) const;

	uint32_t GetImageCount(void) const;

	Scene(LWVideoDriver *Driver);

	~Scene();
private:
	LWVideoDriver *m_Driver;
	std::vector<uint32_t> m_RootNodes;
	Model m_ModelList[MaxModels];
	LWEGLTFMaterial m_MaterialList[MaxMaterials];
	Skeleton m_SkeletonList[MaxSkeletons];
	SceneNode m_NodeList[MaxNodes];
	LWEGLTFTexture m_TextureList[MaxTextures];
	Light m_LightList[MaxLights];
	LWTexture *m_ImageList[MaxImages];
	LWVector3f m_AABBMin;
	LWVector3f m_AABBMax;
	uint64_t m_StartTime = 0;
	uint32_t m_ModelCount = 0;
	uint32_t m_MaterialCount = 0;
	uint32_t m_SkeletonCount = 0;
	uint32_t m_NodeCount = 0;
	uint32_t m_LightCount = 0;
	uint32_t m_ImageCount = 0;
	uint32_t m_TextureCount = 0;
};

#endif
