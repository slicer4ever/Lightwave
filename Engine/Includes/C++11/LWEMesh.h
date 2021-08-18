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

	LWEPrimitive() = default;
};

struct LWEBone {
	char8_t m_Name[LWEBoneMaxName] = {};
	LWSMatrix4f m_InvBindMatrix;
	LWSMatrix4f m_Transform;
	uint32_t m_NextBoneID = -1;
	uint32_t m_ChildBoneID = -1;
	uint32_t m_NameHash = LWUTF8I::EmptyHash;

	LWUTF8Iterator GetName(void) const;

	LWEBone(const LWUTF8Iterator &Name, const LWSMatrix4f &Transform, const LWSMatrix4f &InvTransform);

	LWEBone() = default;
};

class LWEMesh {
public:

	LWEMesh &MakeGLTFMesh(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, bool BakeTransform = false);

	LWEMesh &MakeGLTFSkin(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFSkin *Skin, LWAllocator &Allocator);

	//Pass Null to BoneMatrix's will build a bind posed AABB.
	LWEMesh &BuildAABB(const LWSMatrix4f &Transform, const LWSMatrix4f *BoneMatrixs);

	const LWEMesh &TransformBounds(const LWSMatrix4f &Transform, LWSVector4f &MinBounds, LWSVector4f &MaxBounds) const;

	bool UploadData(LWERenderer *Renderer, uint32_t NamedBlockGeometry, LWAllocator &Allocator, bool KeepLocal = false);

	bool PushPrimitive(const LWEPrimitive &Prim);

	LWEMesh &ReleaseData(LWERenderer *Renderer);

	const LWEMesh &ApplyRotationTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const;

	const LWEMesh &ApplyTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildBindTransforms(LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildAnimationTransform(const LWSMatrix4f *AnimTransforms, LWSMatrix4f *TransformMatrixs) const;

	uint32_t BuildRenderMatrixs(const LWSMatrix4f *TransformMatrixs, LWSMatrix4f *RenderMatrixs) const;

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