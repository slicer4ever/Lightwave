#ifndef LWERENDERFRAME_H
#define LWERENDERFRAME_H
#include <LWVideo/LWVideoBuffer.h>
#include <LWCore/LWUnicodeIterator.h>
#include <LWCore/LWSMatrix.h>
#include <LWCore/LWSVector.h>
#include <LWETypes.h>
#include <LWERenderTypes.h>
#include <array>
#include <unordered_map>

//Set when building LWEngine to make LWERenderFrame support adding models from multiple threads.
#ifdef LWETHREADEDRENDERFRAME
typedef std::atomic<uint32_t> LWERenderFrameCounter;
#else
typedef uint32_t LWERenderFrameCounter;
#endif

class LWERenderFrame;

class LWERendererBlockGeometry;

class LWECamera;

struct LWEGeometryModelBlock {
	LWBitField64(IndiceVB, 22, 0);
	LWBitField64(VertexAttributeVB, 21, IndiceVBBitsOffset + 22);
	LWBitField64(VertexPositionVB, 21, VertexAttributeVBBitsOffset + 21);

	uint64_t m_BlockID = -1;
	uint64_t m_BufferName = (uint64_t)LWUTF8I::EmptyHash; //if BlockID is -1, then BufferName is actually interpreted to be 3 video buffers, the upper 21 bits for the VertexBuffer ID for posiiton attribute vertices, the next 21 bits for the vertexBuffer ID for the attribute vertices, and lower 22 bits for the index buffer, with 0 in either indicating no bound buffer.  Note: When rending the vertex buffer, vertex input mappings are expected to be in order of positions, followed by attributes.
	uint32_t m_Offset = 0;
	uint32_t m_Count = 0;

	uint32_t Hash(void) const;

	bool isVideoBuffer(void) const;

	LWEGeometryModelBlock(LWERendererBlockGeometry *Block, uint64_t BlockID, uint32_t Offset = 0, uint32_t Count = 0);

	LWEGeometryModelBlock(uint32_t BufferName, uint64_t BlockID, uint32_t Offset = 0, uint32_t Count = 0);

	//Because of how Rendable Indirect is generated, only indice video buffers are supported at the moment.
	LWEGeometryModelBlock(uint32_t VertexPositionBufferID, uint32_t VertexAttributeBufferID, uint32_t IndiceBufferID, uint32_t Offset = 0, uint32_t Count = 0);

	LWEGeometryModelBlock(const LWERenderVideoBuffer &VertexPositionBuffer, const LWERenderVideoBuffer &VertexAttributeBuffer, const LWERenderVideoBuffer &IndiceBuffer, uint32_t Offset = 0, uint32_t Count = 0);

	LWEGeometryModelBlock() = default;
};

struct LWEGeometryModel {
	LWBitField32(DistanceMode, 2, 0);

	static const uint32_t DefaultDistance = 0;
	static const uint32_t ForceDrawFirst = 0x1;
	static const uint32_t ForceDrawLast = 0x2;
	static const uint32_t Transparent = 0x4;

	LWERenderMaterial m_Material;
	LWEGeometryModelBlock m_GeometryBlock;
	uint32_t m_Flag = 0;

	bool isTransparent(void) const;

	uint32_t GetDistanceMode(void) const;

	LWEGeometryModel(const LWEGeometryModelBlock &Block, const LWERenderMaterial &Material, uint32_t Flag = 0);

	LWEGeometryModel() = default;
};

struct LWEGeometryBucketItem {
	static const float ForceFirstDistance;
	static const float ForceLastDistance;
	
	uint32_t m_ModelIndex = -1;
	uint32_t m_MaterialHash = LWUTF8I::EmptyHash;
	uint32_t m_BlockHash = LWUTF8I::EmptyHash;
	float m_DistanceSq = 0.0f; //-1+float_max is special value even in state sorting 

	static bool SortFrontToBack(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B);

	static bool SortBackToFront(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B);

	static bool SortState(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B);

	LWEGeometryBucketItem(uint32_t ModelIndex, uint32_t MaterialHash, uint32_t BlockHash, float DistanceSq);

	LWEGeometryBucketItem() = default;
};

struct LWEShadowBucketItem {
	uint32_t m_LightIndex = -1;
	float m_DistanceSq = 0.0f;

	static bool SortFrontToBack(const LWEShadowBucketItem &A, const LWEShadowBucketItem &B);

	static bool SortBackToFront(const LWEShadowBucketItem &A, const LWEShadowBucketItem &B);

	LWEShadowBucketItem(uint32_t LightIndex, float DistanceSq);

	LWEShadowBucketItem() = default;
};

struct LWEGeometryBucket {
	LWBitField32(OpaqueSort, 2, 0);
	LWBitField32(TransparentSort, 2, OpaqueSortBitsOffset + 2);

	static const uint32_t SortByStates = 0;
	static const uint32_t SortBackToFront = 1;
	static const uint32_t SortFrontToBack = 2;
	static const uint32_t SortNone = 3;
	static const uint32_t ShadowMapping = 0x40000000;
	static const uint32_t Initialized = 0x80000000;

	LWSVector4f m_Frustum[6];
	LWSVector4f m_FrustumPoints[6];
	LWSMatrix4f m_ViewTransform;
	LWEGeometryBucketItem m_OpaqueItems[LWEMaxBucketSize];
	LWEGeometryBucketItem m_TransparentItems[LWEMaxBucketSize];
	
	LWERenderFrameCounter m_OpaqueCount;
	LWERenderFrameCounter m_TransparentCount;
	uint32_t m_Flag = (SortBackToFront << TransparentSortBitsOffset) | (SortByStates << OpaqueSortBitsOffset);
	uint32_t m_PassBit = 0;

	bool PushModel(uint32_t ModelIndex, uint32_t ModelBlockHash, uint32_t MaterialHash, const LWEGeometryModel &Model, const LWSVector4f &Position);

	bool SphereInFrustum(const LWSVector4f &Position, float Radius);

	bool AABBInFrustum(const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds);

	bool ConeInFrustum(const LWSVector4f &Position, const LWSVector4f &Direction, float Theta, float Length);

	LWEGeometryBucket &SetOpaqueSort(uint32_t SortMode);

	LWEGeometryBucket &SetTransparentSort(uint32_t SortMode);

	//return OpaqueCount+TransparentCount for drawing this geometry batch.
	std::pair<uint32_t, uint32_t> Finalize(LWERenderer *Renderer, LWERenderFrame &Frame, LWVideoBuffer *IndirectBuffer, LWVideoBuffer *IDBuffer, LWEGeometryRenderable RenderableList[LWEMaxBucketSize]);

	void Reset(void);

	bool Initialize(const LWECamera &View);

	bool isInitialized(void) const;

	uint32_t GetTransparentSortMode(void) const;

	uint32_t GetOpaqueSortMode(void) const;

	LWEGeometryBucket() = default;
};

class LWERenderFrame {
public:
	bool InitializeBucket(const LWECamera &View, uint32_t PassBucketID, uint32_t BucketIdx = 0);

	uint32_t SpherePassBitsInBuckets(uint32_t PassBits, const LWSVector4f &Position, float Radius);

	uint32_t AABBPassBitsInBuckets(uint32_t PassBits, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds);

	bool WriteDebugLine(uint32_t PassBits, const LWSVector4f &APnt, const LWSVector4f &BPnt, float Thickness, const LWVector4f &Color, uint32_t Flags = 0);

	bool WriteDebugPoint(uint32_t PassBits, const LWSVector4f &Ctr, float Radius, const LWVector4f &Color, uint32_t Flags = 0);

	bool WriteDebugCone(uint32_t PassBits, const LWSVector4f &Pos, const LWSVector4f &Dir, float Theta, float Length, const LWVector4f &Color, uint32_t Flags = 0);

	bool WriteDebugCube(uint32_t PassBits, const LWSVector4f &Pos, const LWSVector4f &Size, const LWVector4f &Color, uint32_t Flags = 0);

	bool WriteDebugAABB(uint32_t PassBits, const LWSMatrix4f &Transform, const LWSVector4f &AAMin, const LWSVector4f &AAMax, float LineThickness, const LWVector4f &Color, uint32_t Flags = 0);
	
	bool WriteDebugAxis(uint32_t PassBits, const LWSMatrix4f &Transform, float LineLen, float LineThickness, bool IgnoreScale = true, uint32_t Flags = 0);

	bool WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, const LWSMatrix4f &Transform, const LWVector4f &Color, const LWSVector4f &Position, float Radius, uint32_t Flags = 0);

	bool WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, const LWSMatrix4f &Transform, const LWVector4f &Color, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds, uint32_t Flags = 0);

	uint32_t WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, LWERenderMaterial &GeometryMaterial, LWEGeometryModelData &GeometryData, const LWSVector4f &Position, float Radius, uint32_t Flags = 0);

	uint32_t WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, LWERenderMaterial &GeometryMaterial, LWEGeometryModelData &GeometryData, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds, uint32_t Flags = 0);

	bool PushMesh(const LWEMesh &Mesh, uint32_t PassBits, const LWSMatrix4f &Transform, uint32_t BoneID, const LWERenderMaterial *PrimitiveMaterialList, LWEGeometryModelData *PrimitiveDataList, uint32_t Flags = 0);

	bool PushMeshPrimitive(const LWEMesh &Mesh, uint32_t Primitive, uint32_t PassBits, const LWSMatrix4f &Transform, uint32_t BoneID, const LWERenderMaterial &PrimitiveMaterial, LWEGeometryModelData &PrimitiveData, uint32_t Flags = 0);

	uint32_t PushModel(const LWEGeometryModel &Model, const LWEGeometryModelData &Data, uint32_t PassBits, const LWSVector4f &Position, float Radius);

	uint32_t PushModel(const LWEGeometryModel &Model, const LWEGeometryModelData &Data, uint32_t PassBits, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds);

	bool PushLight(const LWEShaderLightData &Light, bool bIsShadowCaster = false);

	LWERenderFrame &Initialize(uint32_t FrameID);

	LWERenderFrame &Finalize(LWERenderer *Renderer, float Time, LWVideoBuffer *GlobalDataBuffer, LWVideoBuffer *ModelDataBuffer, LWVideoBuffer *BoneDataBuffer, LWVideoBuffer *LightDataBuffer, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWEGeometryRenderable RenderList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderCount[LWEMaxGeometryBuckets]);

	uint32_t NextBoneID(uint32_t BoneCount);

	uint32_t PushFrameData(LWEPassFrameData *Data);

	uint32_t PassCreateGeometryBuckets(uint32_t BucketCount);

	LWERenderFrame &SetPrimaryBucket(uint32_t PrimaryBucket);

	LWERenderFrame &SetDebugGeometry(LWERendererBlockGeometry *DebugGeometry);

	LWEGeometryBucket &GetGeometryBucket(uint32_t ID);

	LWEGeometryBucket &GetPrimaryBucket(void);

	LWEShaderGlobalData &GetGlobalData(void);

	uint32_t GetPassBucketCount(void) const;

	LWEPassFrameData *GetFrameData(uint32_t ID);

	LWSMatrix4f *GetBoneDataAt(uint32_t ID);

	LWEShadowBucketItem *GetShadowList(void);

	const LWEGeometryModel &GetModel(uint32_t Index) const;

	LWEGeometryModel &GetModel(uint32_t Index);

	const LWEShaderLightData &GetLight(uint32_t Index) const;

	LWEShaderLightData &GetLight(uint32_t Index);

	uint32_t GetLightCount(void) const;

	uint32_t GetShadowCount(void) const;

	uint32_t GetPrimaryBucketID(void) const;

	LWERendererBlockGeometry *GetDebugGeometry(void);

	template<class Type>
	Type *GetFrameData(uint32_t ID) {
		return (Type*)m_PassList[ID];
	}

	LWERenderFrame(LWVideoDriver *Driver, uint32_t PassCount, LWERendererBlockGeometry *DebugGeometry, LWAllocator &Allocator);

	LWERenderFrame() = default;
private:
	LWEShaderGlobalData m_GlobalData;
	LWEGeometryBucket m_GeometryBuckets[LWEMaxGeometryBuckets];
	LWEGeometryModel m_ModelList[LWEMaxBucketSize];
	LWEGeometryModelData m_ModelData[LWEMaxBucketSize];
	LWEShaderLightData m_LightData[LWEMaxLights];
	LWSMatrix4f m_BoneData[LWEMaxBoneCount];
	LWEShadowBucketItem m_ShadowLightList[LWEMaxLights];
	std::vector<LWEPassFrameData*> m_PassList;
	LWERendererBlockGeometry *m_DebugGeometry = nullptr;

	LWERenderFrameCounter m_ModelCount;
	LWERenderFrameCounter m_BoneCount;
	LWERenderFrameCounter m_LightCount;
	LWERenderFrameCounter m_ShadowCount;
	uint32_t m_PassCount = 0;
	uint32_t m_PassBucketCount = 0;
	uint32_t m_PrimaryBucket = 0;
	uint32_t m_FrameID = 0;
};

#endif