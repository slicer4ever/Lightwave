#ifndef LWEMESH_H
#define LWEMESH_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWSMatrix.h>
#include <LWCore/LWUnicodeIterator.h>
#include "LWEGLTFParser.h"
#include "LWERenderTypes.h"

static const uint32_t LWEMaxBones = 64;
static const uint32_t LWEBoneMaxName = 64;

struct LWEPrimitive {
	uint32_t m_Offset = 0;
	uint32_t m_Count = 0;

	static bool Deserialize(LWEPrimitive &Prim, LWByteBuffer &BB);

	uint32_t Serialize(LWByteBuffer &BB) const;

	LWEPrimitive() = default;
};

struct LWEBone {
	char8_t m_Name[LWEBoneMaxName] = {};
	LWSMatrix4f m_InvBindMatrix;
	LWSMatrix4f m_Transform;
	uint32_t m_NextBoneID = -1;
	uint32_t m_ChildBoneID = -1;
	uint32_t m_NameHash = LWUTF8I::EmptyHash;

	static bool Deserialize(LWEBone &Bone, LWByteBuffer &BB);

	uint32_t Serialize(LWByteBuffer &BB) const;

	LWUTF8Iterator GetName(void) const;

	LWEBone(const LWUTF8Iterator &Name, const LWSMatrix4f &Transform, const LWSMatrix4f &InvTransform);

	LWEBone() = default;
};

class LWEMesh {
public:

	static bool Deserialize(LWEMesh &Mesh, LWByteBuffer &BB, LWAllocator &Allocator);

	//Convience function if using built-in vertex types, otherwise should use the custom loader to specify offsets for the data, BakeTransform bake's the world transform for the mesh into the vertice's.
	LWEMesh &MakeGLTFMesh(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, bool BakeTransform = false);

	//loads gltf data into specified offsets in bytes of structure(Position, BoneWeights+BoneIndices are expected to be in the positional portion of data, all other attributes are expected to be in the attr porition of data), -1 indicates that there is no corrisponding vertice in the structure.
	//Note: All types(except texcoord) are expected to be vec4's, and texcoord are expected to be vec2's.
	LWEMesh &MakeGLTFMeshCustom(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, uint32_t VerticePosTypeSize, uint32_t VerticeAttrTypeSize, uint32_t PositionOffset = 0, uint32_t WeightOffset = -1, uint32_t JointOffset = -1, uint32_t NormalOffset = -1, uint32_t TangentOffset = -1, uint32_t TexCoord0Offset = -1, uint32_t TexCoord1Offset = -1, uint32_t ColorOffset = -1, bool BakeTransform = false);

	LWEMesh &MakeGLTFSkin(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFSkin *Skin, LWAllocator &Allocator);

	//Pass Null to BoneMatrix's will build a bind posed AABB, build's aabb for LWEStaticVertex+LWESkeletonVertex, if using a custom type, then use the overloaded method.
	LWEMesh &BuildAABB(const LWSMatrix4f &Transform, const LWSMatrix4f *BoneMatrixs);

	//Does same as other BuildAABB but allows for custom vertex types, Offsets are in byte size and the 3 attributes are expected to be in Position data, BoneWeightOffset and BoneIndexOffset are not equal to do bone transforms, otherwise only builds from position data alone.
	LWEMesh &BuildVertexAABB(const LWSMatrix4f &Transform, const LWSMatrix4f *BoneMatrixs, uint32_t PositionOffset = 0, uint32_t BoneWeightOffset = 0, uint32_t BoneIndexOffset = 0);

	const LWEMesh &TransformBounds(const LWSMatrix4f &Transform, LWSVector4f &MinBounds, LWSVector4f &MaxBounds) const;

	bool UploadData(LWERenderer *Renderer, uint32_t NamedBlockGeometry, LWAllocator &Allocator, bool KeepLocal = false);

	bool UploadData(LWERenderer *Renderer, const LWUTF8Iterator &NamedBlockGeometry, LWAllocator &Allocator, bool KeepLocal = false);

	bool PushPrimitive(const LWEPrimitive &Prim);

	LWEMesh &ReleaseData(LWERenderer *Renderer);

	const LWEMesh &ApplyRotationTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const;

	const LWEMesh &ApplyTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildBindTransforms(LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildAnimationTransform(const LWSMatrix4f *AnimTransforms, LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildRenderMatrixs(const LWSMatrix4f *TransformMatrixs, LWSMatrix4f *RenderMatrixs) const;

	void SetBounds(const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds);

	void SetBoneCount(uint32_t Count);

	uint32_t Serialize(LWByteBuffer &Buffer) const;

	LWEMesh &operator = (LWEMesh &&Other);

	LWEMesh &operator = (const LWEMesh &Other);

	const uint8_t *GetVerticePositions(void) const;

	uint8_t *GetVerticePositions(void);

	template<class Type>
	const Type *GetVerticePositions(void) const {
		return (Type*)GetVerticePositions();
	}

	template<class Type>
	Type *GetVerticePositions(void) {
		return (Type*)GetVerticePositions();
	}

	const uint8_t *GetVerticeAttributes(void) const;

	uint8_t *GetVerticeAttributes(void);

	template<class Type>
	const Type *GetVerticeAttributes(void) const {
		return (Type*)GetVerticeAttributes();
	}

	template<class Type>
	Type *GetVerticeAttributes(void) {
		return (Type*)GetVerticeAttributes();
	}

	const uint32_t *GetIndices(void) const;

	uint32_t *GetIndices(void);

	const LWEPrimitive &GetPrimitive(uint32_t Idx) const;

	const LWEBone &GetBone(uint32_t Idx) const;

	LWEBone &GetBone(uint32_t Idx);

	uint32_t GetBoneCount(void) const;

	uint32_t GetPrimitiveCount(void) const;

	uint32_t GetVerticeCount(void) const;

	uint32_t GetIndiceCount(void) const;

	uint32_t GetVerticePositionTypeSize(void) const;

	uint32_t GetVerticeAttributeTypeSize(void) const;

	LWSVector4f GetMinBounds(void) const;

	LWSVector4f GetMaxBounds(void) const;

	uint64_t GetID(void) const;

	uint32_t GetBlockGeometry(void) const;

	template<class VertexPosType, class VertexAttrType>
	LWEMesh(LWAllocator &Allocator, uint32_t VerticeCount, uint32_t IndiceCount, LWEBone *BoneList, uint32_t BoneCount) : LWEMesh(Allocator, sizeof(VertexPosType), sizeof(VertexAttrType), VerticeCount, IndiceCount, BoneList, BoneCount) {}

	template<class VertexPosType, class VertexAttrType>
	LWEMesh(LWAllocator &Allocator, VertexPosType *VerticePos, VertexAttrType *VerticeAttr, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount, LWEBone *BoneList, uint32_t BoneCount) : LWEMesh(Allocator, sizeof(VertexPosType), sizeof(VertexAttrType), VerticeCount, IndiceCount, BoneList, BoneCount) {
		std::copy(VerticePos, VerticePos+m_VerticeCount, (VertexPosType*)m_VerticePositions);
		std::copy(VerticeAttr, VerticeAttr+m_VerticeCount, (VertexAttrType*)m_VerticeAttributes);
		std::copy(Indices, Indices+m_IndiceCount, m_Indices);
	}

	LWEMesh(LWAllocator &Allocator, uint32_t VerticePosTypeSize, uint32_t VerticeAttrTypeSize, uint32_t VerticeCount, uint32_t IndiceCount, LWEBone *BoneList, uint32_t BoneCount);

	//Move constructor
	LWEMesh(LWEMesh &&Other);

	//Copy constructor.
	LWEMesh(const LWEMesh &Other);

	LWEMesh() = default;

	~LWEMesh();
protected:
	std::vector<LWEPrimitive> m_PrimitiveList;
	LWEBone m_BoneList[LWEMaxBones];
	LWSVector4f m_MinBounds;
	LWSVector4f m_MaxBounds;
	uint8_t *m_VerticePositions = nullptr;
	uint8_t *m_VerticeAttributes = nullptr;
	uint32_t *m_Indices = nullptr;
	uint64_t m_ID = -1;
	uint32_t m_BlockGeometry = LWUTF8I::EmptyHash;
	uint32_t m_VerticePositionTypeSize = 0;
	uint32_t m_VerticeAttributeTypeSize = 0;
	uint32_t m_VerticeCount = 0;
	uint32_t m_IndiceCount = 0;
	uint32_t m_BoneCount = 0;
};

#endif