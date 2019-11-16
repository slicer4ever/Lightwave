#ifndef RENDERER_H
#define RENDERER_H
#include <LWVideo/LWVideoDriver.h>
#include <LWVideo/LWMesh.h>
#include <LWVideo/LWFont.h>
#include <LWVideo/LWVideoBuffer.h>
#include <LWVideo/LWPipeline.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWVector.h>
#include <LWEAsset.h>

class Scene;

class Camera;

struct Vertice;

struct ListData {
	static const uint32_t MaxShadows = 8;
	LWMatrix4f m_ProjViewMatrix;
	LWMatrix4f m_ShadowProjViewMatrix[MaxShadows];
	LWVector4f m_FrustumPoints[6]; //Near TL, TR, BL,  Far TL, TR, BL.
	LWVector4f m_ViewPosition;
	LWVector2f m_ScreenSize;
	LWVector2i m_ThreadDimensions;
	LWVector2i m_TileSize;
	uint32_t m_LightCount;
};

struct ModelData {
	static const uint32_t MaxBones = 64;
	LWMatrix4f m_TransformMatrix;
	LWMatrix4f m_BoneMatrixs[MaxBones];
};

struct MaterialData {
	LWVector4f m_MaterialColorA;
	LWVector4f m_MaterialColorB;
	LWVector4f m_EmissiveFactor;
	uint32_t m_HasTextureFlag;
};

struct MaterialTextureInfo {
	LWTexture *m_Texture = nullptr;
	uint32_t m_TextureFlag = 0;
};

struct MaterialInfo {
	static const uint32_t NormalTextureBit = 0x1;
	static const uint32_t OcclussionTextureBit = 0x2;
	static const uint32_t EmissiveTextureBit = 0x4;
	static const uint32_t MaterialTextureABit = 0x8;
	static const uint32_t MaterialTextureBBit = 0x10;
	MaterialTextureInfo m_NormalTexture;
	MaterialTextureInfo m_OcclussionTexture;
	MaterialTextureInfo m_EmissiveTexture;
	MaterialTextureInfo m_MaterialTextureA;
	MaterialTextureInfo m_MaterialTextureB;
	uint32_t m_Type;
	bool m_Opaque = true;
};

struct GaussianData {
	LWVector4f m_BlurWeight;
	LWVector4f m_iTexSize;
};

struct InstancePrimitive {
	LWMesh<Vertice> *m_Mesh;
	uint32_t m_MaterialID;
};

struct Light {
	LWVector4f m_Position;
	LWVector4f m_Direction;
	LWVector4f m_Color;
	LWVector4i m_ShadowIdx;

	bool MakeCamera(Camera &C);

	//Ambient global Light
	Light(const LWVector4f &Color, float Intensity);

	//Directional global Light
	Light(const LWVector3f &Direction, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs);
	
	//Point Light, Interior radius is radius where light is full brightness, Falloff is amount of radius after interior that linearly goes from 100%-0%, this is a simplified point light that is generally easier to manage.
	Light(const LWVector3f &Position, float InteriorRadius, float FalloffRadius, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs);

	//Spot light.
	Light(const LWVector3f &Position, const LWVector3f &Dir, float Length, float Theta, const LWVector4f &Color, float Intensity, const LWVector4i &ShadowIndexs);

	Light() = default;
};


struct Instance {
	static const uint32_t MaxPrimitives = 256;
	InstancePrimitive m_PrimitiveList[MaxPrimitives];
	uint32_t m_PrimitiveCount;
	bool m_HasSkin;
};

//List for each "view" so that shadow and reflection maps can use the same data structures for rendering.
struct lFrameList {
	static const uint32_t MaxListInstances = 1024;
	static const uint32_t PointView = 0x1;
	static const uint32_t ShadowView = 0x2;

	uint32_t WriteInstance(uint32_t InstanceID, bool Opaque);

	LWVector4f m_FrustumPoints[6];
	LWMatrix4f m_ProjViewMatrix;
	LWVector4f m_ViewPosition;
	uint32_t m_InstanceIDs[MaxListInstances];
	uint32_t m_OpaqueCnt = 0;
	uint32_t m_TransparentCnt = 0;
	uint32_t m_Flag = 0;
};

//uploading the instance/material data each frame is unnecessary, but since this is an sample that's also pushing the api to discover any bugs it is done this way.
struct lFrame {
	static const uint32_t MaxInstances = 1024;
	static const uint32_t MaxMaterials = 1024;
	static const uint32_t MaxLights = 1024;
	static const uint32_t MaxLists = 9;
	static const uint32_t MainView = 0;

	LWFontSimpleWriter m_FontWriter;
	lFrameList m_Lists[MaxLists];
	uint8_t *m_InstanceBuffer;
	uint8_t *m_MaterialBuffer;
	uint8_t *m_LightBuffer;
	MaterialInfo m_Materials[MaxMaterials];
	Instance m_Instances[MaxInstances];
	uint32_t m_InstanceCount;
	uint32_t m_MaterialCount;
	uint32_t m_LightCount;
	uint32_t m_ListCount;

	static uint32_t GetIDBit(uint32_t ID);

	//Returns the ID for the camera.
	uint32_t MakeList(Camera &C);

	uint32_t WriteInstance(const Instance &Inst, const ModelData &InstanceData, uint32_t ListBits, LWVideoDriver *Driver);

	uint32_t WriteMaterial(const MaterialData &Mat, const MaterialInfo &MatInfo, LWVideoDriver *Driver);

	uint32_t WriteLight(Light &L, LWVideoDriver *Driver);
};

class App;

class Renderer {
public:
	static const uint32_t FrameCount = 3;
	static const uint32_t MaxCharacters = 256;
	static const uint32_t MaxLightsPerTile = 64;
	static const LWVector2i TileSize;
	static const LWVector2i LocalThreads;

	bool WriteDebugLine(lFrame &F, const LWVector3f &APnt, const LWVector3f &BPnt, float Thickness, const LWVector4f &Color);

	bool WriteDebugPoint(lFrame &F, const LWVector3f &Pnt, float Radius, const LWVector4f &Color);

	bool WriteDebugRect(lFrame &F, const LWVector3f &Pnt, const LWVector3f &Size, const LWQuaternionf &Rot, const LWVector4f &Color);

	bool WriteDebugCone(lFrame &F, const LWVector3f &Pnt, const LWVector3f &Dir, float Theta, float Length, const LWVector4f &Color);

	lFrame *BeginFrame(void);

	Renderer &EndFrame(void);

	Renderer &SizeChanged(LWWindow *Window);

	Renderer &ApplyFrame(lFrame &F, LWWindow *Wnd);

	LWPipeline *PreparePipeline(lFrame &F, bool isPointView, bool isShadowCaster, bool isSkinned, bool isOpaque, uint32_t ListIdx, uint32_t InstanceID, uint32_t MaterialID);

	Renderer &RenderText(LWFontSimpleWriter &FontWriter);

	Renderer &RenderList(lFrame &F, uint32_t ListIdx, LWWindow *Window);

	Renderer &RenderFrame(lFrame &F, LWWindow *Window);

	Renderer &Render(LWWindow *Window);

	LWVector2i GetThreadGroups(const LWVector2i WndSize);

	Renderer(LWVideoDriver *Driver, App *A, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	~Renderer();
private:
	lFrame m_Frames[FrameCount];
	LWAllocator &m_Allocator;
	LWVideoDriver *m_Driver = nullptr;
	LWPipeline *m_FontPipeline = nullptr;
	LWVideoBuffer *m_TextUniformBuffer = nullptr;

	LWFrameBuffer *m_ShadowFB = nullptr;
	LWTexture *m_ShadowDepthBuffer;

	LWMesh<Vertice> *m_DebugCube = nullptr;;
	LWMesh<Vertice> *m_DebugPoint = nullptr;
	LWMesh<Vertice> *m_DebugCone = nullptr;
	LWMesh<LWVertexTexture> *m_DebugRect = nullptr;

	LWShader *m_StaticShader = nullptr;
	LWShader *m_StaticPointShader = nullptr;
	LWShader *m_SkeletonShader = nullptr;
	LWShader *m_SkeletonPointShader = nullptr;

	LWPipeline *m_MetallicRoughnessPipeline = nullptr;
	LWPipeline *m_SpecularGlossinessPipeline = nullptr;
	LWPipeline *m_UnlitPipeline = nullptr;
	LWPipeline *m_ShadowPipeline = nullptr;
	LWPipeline *m_LightCullPipeline = nullptr;

	LWVideoBuffer *m_ListUniform = nullptr;
	LWVideoBuffer *m_LightArrayBuffer = nullptr;
	LWVideoBuffer *m_ModelUniform = nullptr;
	LWVideoBuffer *m_MaterialUniform = nullptr;
	LWVideoBuffer *m_LightIndexBuffer = nullptr;

	uint32_t m_ReadFrame = 0;
	uint32_t m_WriteFrame = 0;
	bool m_SizeChanged = true;
};

#endif