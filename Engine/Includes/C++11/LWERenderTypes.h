#ifndef LWERENDERTYPES_H
#define LWERENDERTYPES_H
#include <LWCore/LWAllocator.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWSMatrix.h>
#include <LWCore/LWUnicodeIterator.h>

class LWERenderer;

class LWERenderFrame;

class LWERenderPass;

class LWERendererBlockGeometry;

static const uint32_t LWEMaxGeometryBuckets = 16;
static const uint32_t LWEMaxPasses = 16;
static const uint32_t LWEMaxBindableTextures = 5;
static const uint32_t LWEMaxMaterialData = 4;
static const uint32_t LWEMaxBucketSize = 4096 * 4;
static const uint32_t LWEMaxBoneCount = 4096 * 4;
static const uint32_t LWEMaxLights = 2048;

//PBR const's:
static const uint32_t LWEPBRNormalTexID = 0;
static const uint32_t LWEPBROcclusionTexID = 1;
static const uint32_t LWEPBREmissiveTexID = 2;

static const uint32_t LWEPBRNormalTexNameHash = 0x60094341; //PBRNormalTex
static const uint32_t LWEPBROcclusionTexNameHash = 0xbd9252a8; //PBROcclusionTex
static const uint32_t LWEPBREmissiveTexNameHash = 0x0438b365; //PBREmissiveTex

static const uint32_t LWEPBREmissiveFactorID = 2;

//PBRUNLIT:
static const uint32_t LWEPBRUnlitColorTexID = 3;
static const uint32_t LWEPBRUnlitColorTexNameHash = 0x17a8b46b; //PBRColorTex

static const uint32_t LWEPBRUnlitColorFactorID = 0;

//PBRMetallicRoughness:
static const uint32_t LWEPBRMRAlbedoTexID = 3;
static const uint32_t LWEPBRMRFactorTexID = 4;

static const uint32_t LWEPBRMRAlbedoTexNameHash = 0x8fb8726f; //PBRAlbedoTex
static const uint32_t LWEPBRMRTexNameHash = 0x14e0d9dd; //PBRMRTex

static const uint32_t LWEPBRMRBaseColorID = 0;
static const uint32_t LWEPBRMRFactorID = 1;

//PBRSpecularGlossiness:
static const uint32_t LWEPBRSGDiffuseTexID = 3;
static const uint32_t LWEPBRSGSpecularTexID = 4;

static const uint32_t LWEPBRSGDiffuseTexNameHash = 0x8974cbc2; //PBRSGDiffuseTex
static const uint32_t LWEPBRSGSpecularTexNameHash = 0x5e617a6d; //PBRSGSpecularTex

static const uint32_t LWEPBRSGDiffuseID = 0;
static const uint32_t LWEPBRSGSpecularID = 1;

struct LWEPassFrameData {};

struct LWERenderMaterialTexture {
	uint32_t m_ResourceName = LWUTF8I::EmptyHash;
	uint32_t m_TextureID = 0;
	uint32_t m_TextureState = 0;

	LWERenderMaterialTexture(const LWUTF8I &ResourceName, uint32_t TextureID, uint32_t TextureState = 0);

	LWERenderMaterialTexture(uint32_t ResourceName, uint32_t TextureID, uint32_t TextureState = 0);

	LWERenderMaterialTexture() = default;
};


//Helper class for applications to manage video buffer's, allocates a local data reserve, upload's to Renderer when requested.
struct LWERenderVideoBuffer {
	uint32_t m_ID = 0;
	uint32_t m_TypeSize = 0;
	uint32_t m_Count = 0;
	uint32_t m_BufferType = 0;
	uint32_t m_BufferUsage = 0;
	uint8_t *m_Data = nullptr;

	//if DiscardData is set to true then m_Data is deallocated after successful upload.
	bool UploadData(LWERenderer *Renderer, bool DiscardData = true);

	//If Renderer is not null, delete's the ID'd videobuffer from renderer.
	bool ReleaseData(LWERenderer *Renderer);

	template<class Type>
	Type *Create(uint32_t Count, LWAllocator &Allocator) {
		m_TypeSize = sizeof(Type);
		m_Count = Count;
		uint8_t *pData = m_Data;
		m_Data = Allocator.Allocate<uint8_t>(m_TypeSize * Count);
		LWAllocator::Destroy(pData);
		return (Type*)m_Data;
	}

	template<class Type>
	Type *As(void) {
		return (Type*)m_Data;
	}

	LWERenderVideoBuffer(uint32_t BufferType, uint32_t BufferUsage);

	LWERenderVideoBuffer() = default;

	~LWERenderVideoBuffer();
};


struct LWERenderMaterial {
	uint32_t m_PipelineName = LWUTF8I::EmptyHash;
	LWERenderMaterialTexture m_TextureList[LWEMaxBindableTextures];
	uint32_t m_TextureCount = 0;

	uint32_t Hash(void) const;

	uint32_t PushTexture(const LWERenderMaterialTexture &Texture, uint32_t TextureID); //Returns the bit of TextureID if can be inserted, or 0 if not.

	LWERenderMaterial(const LWUTF8I &PipelineName, LWERenderMaterialTexture *TextureList, uint32_t TextureCount);

	LWERenderMaterial(uint32_t PipelineNameHash, LWERenderMaterialTexture *TextureList, uint32_t TextureCount);

	LWERenderMaterial() = default;
};


struct LWEGeometryRenderable {
	static const uint32_t BufferVideoBuffer = 0x80000000; //Adds bit to DrawCount if BlockBufferNameHash should instead be interpreted as video buffer id's(with the upper 16 bit being vertex buffer id, and lower 16 being index buffer id).

	LWERenderMaterial m_Material;
	uint64_t m_BlockBufferNameHash = (uint64_t)LWUTF8I::EmptyHash;
	uint32_t m_DrawCount = 0;

	LWEGeometryRenderable(uint64_t BlockBufferNameHash, const LWERenderMaterial &Material, uint32_t DrawCount);

	LWEGeometryRenderable() = default;
};

//The following data structure is added to a vertex shader when including <Structures> when the #STATICVERTICE define is set.
struct LWEStaticVertice {
	LWVector4f m_Position = LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
};

//The following data structure is added to a vertex shader when include <Structures> when the #SKELETONVERTICE define is set.
struct LWESkeletonVertice {
	LWVector4f m_Position = LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	LWVector4f m_BoneWeights;
	LWVector4i m_BoneIndices;
};

//The following data structure is added to a vertex shader's vertice data(after positon attributes) when including <Structures> if #PBRVERTICE define is set.
struct LWEVerticePBR {
	LWVector4f m_TexCoord;
	LWVector4f m_Tangent;
	LWVector4f m_Normal;
};

//Debug writer's write Color to MaterialData[0].
//The following data structures are added to a shader when including <Structures> when the #MODELDATA define is set. (ShaderName: 
struct alignas(32) LWEGeometryModelData {
	LWSMatrix4f m_Transform;
	LWSVector4f m_MaterialData[LWEMaxMaterialData];
	LWVector4f m_SubTextures[LWEMaxBindableTextures];
	uint32_t m_HasTextureFlag = 0;
	uint32_t m_BoneID = 0;
	uint32_t m_Pad[2];

	LWEGeometryModelData(const LWSMatrix4f &Transform, uint32_t BoneID = 0);

	LWEGeometryModelData() = default;
};

//The following data structure is added to a shader when including <Structures> while the #LIGHTDATA define is set.
struct alignas(16) LWEShaderLightData {
	LWBitField32(ShadowPassIDBits, 8, 0);
	LWBitField32(ShadowSubPassIdxBits, 8, ShadowPassIDBitsOffset + 8);
	LWBitField32(ShadowLayerBits, 8, ShadowSubPassIdxBitsOffset + 8);

	LWSVector4f m_Position; //w==0 means direction light, 1 means point, >1 means spot light(Theta-1), <0 means ambient light.
	LWSVector4f m_Direction; //w for spot = Length, x for point is interior radius(where max brightness is, y is falloff from 1-0 radius).
	LWVector4f m_Color; //Color of light(w = intensity of light, default: 1).
	LWVector4i m_ShadowIndexs = LWVector4i(-1, -1, -1, -1); //each light can have a max of 4 shadow maps(first 8 bits is the pass id for the shadow map, next 8 is the sub pass id for the light, and the next 8 bits is the layer for the shadow). 

	static uint32_t MakeShadowFlag(uint32_t PassID, uint32_t SubPassIdx, uint32_t LayerID);

	//Ambient global light
	LWEShaderLightData(float Intensity, const LWVector4f &Color = LWVector4f(1.0f));

	//Directional Light:
	LWEShaderLightData(const LWSVector4f &Direction, const LWVector4f &Color = LWVector4f(1.0f));

	//Spot Light:
	LWEShaderLightData(const LWSVector4f &Position, const LWSVector4f &Direction, float Theta, float Length, const LWVector4f &Color = LWVector4f(1.0f));

	//Point light, uses modified formula where InteriorRadi is max brightness, and FalloffRadi is linear 1-0 falloff.
	LWEShaderLightData(const LWSVector4f &Position, float InteriorRadi, float FalloffRadi, const LWVector4f &Color = LWVector4f(1.0f));

	LWEShaderLightData() = default;
};

//The following data structures are added to a shader when including <Structures> while the #GLOBALDATA define is set.
struct LWEShaderGeometryBucketData {
	LWSMatrix4f m_ProjViewTransform;
	LWSMatrix4f m_ViewTransform;
	LWSVector4f m_Frustum[6];
	LWSVector4f m_FrustumPoints[6]; //Order: 0 = Near top left, 1 = top right, 2 = bottom left.  3 = Far top left, 4 = top right, 5 = bottom left, last 2 points can be calculated from: TL + (BL-TL) + (TR-TL).
};

struct LWEShaderGlobalPassData {
	LWVector4f m_PassData[4];
	LWVector4f m_SubPassData[LWEMaxPasses];
	LWVector4f m_FrameSize[LWEMaxPasses];
	uint32_t m_GeometryBucketOffset = 0;
	uint32_t m_Pad[3];
};

//UniformBlockName: LWEGlobal
struct LWEShaderGlobalData {
	LWEShaderGeometryBucketData m_BucketList[LWEMaxGeometryBuckets];
	LWEShaderGlobalPassData m_PassData[LWEMaxPasses];
	LWVector2f m_WindowSize;
	uint32_t m_LightCount = 0;
	uint32_t m_ShadowCount = 0;
	uint32_t m_ShadowPassID = 0;
	float m_Time = 0.0f;
	float m_SinTime = 0.0f;
	uint32_t m_Pad[1] = {};

	LWEShaderGlobalData() = default;
};

//UniformBlockName: LWEPass
struct LWEShaderPassData {
	uint32_t m_PassID = 0;
	uint32_t m_SubPassID = 0;
	uint32_t m_Pad = 0;

	LWEShaderPassData(uint32_t PassID, uint32_t SubPassID=0);

	LWEShaderPassData() = default;
};

//BoneMatrix(LWMatrix4 array) are added to a shader when including <Structures> while the #BONEDATA define is set.

#endif