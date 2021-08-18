#include "LWEMesh.h"
#include <LWESGeometry3D.h>
#include "LWERenderer.h"
#include "LWELogger.h"

//LWEBone:
LWUTF8Iterator LWEBone::GetName(void) const {
	return m_Name;
}

LWEBone::LWEBone(const LWUTF8Iterator &Name, const LWSMatrix4f &Transform, const LWSMatrix4f &InvTransform) : m_NameHash(Name.Hash()), m_Transform(Transform), m_InvBindMatrix(InvTransform) {
	Name.Copy(m_Name, sizeof(m_Name));
}

//LWEMesh:
LWEMesh &LWEMesh::MakeGLTFMesh(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, bool BakeTransform) {
	const uint32_t VertexPositionOffset = 0;
	const uint32_t VertexWeightOffset = 16;
	const uint32_t VertexWeightIndicesOffset = 32;

	const uint32_t VertexTexCoordOffset = 0;
	const uint32_t VertexTangentOffset = 16;
	const uint32_t VertexNormalOffset = 32;

	m_VerticeCount = 0;
	m_IndiceCount = 0;
	m_VerticePositionTypeSize = sizeof(LWEStaticVertice);
	m_VerticeAttributeTypeSize = sizeof(LWEVerticePBR);
	LWSMatrix4f WorldTransform = P.GetNodeWorldTransform(Source->m_NodeID).AsSMat4();
	for (auto &&Prim : GMesh->m_Primitives) {
		if (Prim.FindAttributeAccessor(LWEGLTFAttribute::JOINTS_0) != -1) {
			m_VerticePositionTypeSize = sizeof(LWESkeletonVertice);
		}
		LWEGLTFAccessor *PosAccessor = P.GetAccessor(Prim.FindAttributeAccessor(LWEGLTFAttribute::POSITION));
		LWEGLTFAccessor *IdxAccessor = P.GetAccessor(Prim.m_IndiceID);
		if (PosAccessor) m_VerticeCount += PosAccessor->m_Count;
		if (IdxAccessor) m_IndiceCount += IdxAccessor->m_Count;
	}
	m_VerticePositions = LWAllocator::Destroy(m_VerticePositions);
	m_VerticeAttributes = LWAllocator::Destroy(m_VerticeAttributes);
	m_Indices = LWAllocator::Destroy(m_Indices);

	if (m_VerticeCount) {
		m_VerticePositions = Allocator.Allocate<uint8_t>(m_VerticeCount * m_VerticePositionTypeSize);
		m_VerticeAttributes = Allocator.Allocate<uint8_t>(m_VerticeCount * m_VerticeAttributeTypeSize);
		std::fill(m_VerticePositions, m_VerticePositions + m_VerticeCount * m_VerticePositionTypeSize, 0);
		std::fill(m_VerticeAttributes, m_VerticeAttributes + m_VerticeCount * m_VerticeAttributeTypeSize, 0);
		for (uint32_t i = 0; i < m_VerticeCount; i++) *(LWSVector4f*)(m_VerticePositions + i * m_VerticePositionTypeSize) = LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	}
	if (m_IndiceCount) m_Indices = Allocator.Allocate<uint32_t>(m_IndiceCount);

	uint32_t v = 0;
	uint32_t o = 0;
	for (auto &&Prim : GMesh->m_Primitives) {
		LWEPrimitive Pm = { m_Indices ? o : v, 0 };
		uint32_t VertCnt = 0;
		uint32_t IndiceCnt = 0;
		uint8_t *VP = m_VerticePositions + (m_VerticePositionTypeSize * (uintptr_t)v);
		uint8_t *VA = m_VerticeAttributes + (m_VerticeAttributeTypeSize * (uintptr_t)v);
		uint32_t *I = m_Indices + o;

		LWEGLTFAccessorView Position;
		LWEGLTFAccessorView TexCoord;
		LWEGLTFAccessorView Tangent;
		LWEGLTFAccessorView Normal;
		LWEGLTFAccessorView BoneWeight;
		LWEGLTFAccessorView BoneIndices;
		LWEGLTFAccessorView Indice;
		if (P.CreateAccessorView(Position, Prim.FindAttributeAccessor(LWEGLTFAttribute::POSITION))) {
			Position.ReadValues<float>((float*)(VP + VertexPositionOffset), m_VerticePositionTypeSize, Position.m_Count);
			VertCnt = Pm.m_Count = Position.m_Count;
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f*)(VP + i * m_VerticePositionTypeSize + VertexPositionOffset);
					*P = *P * WorldTransform;
				}
			}
		}
		if (P.CreateAccessorView(TexCoord, Prim.FindAttributeAccessor(LWEGLTFAttribute::TEXCOORD_0))) {
			TexCoord.ReadValues<float>((float*)(VA + VertexTexCoordOffset), m_VerticeAttributeTypeSize, TexCoord.m_Count);
		}
		if (P.CreateAccessorView(Normal, Prim.FindAttributeAccessor(LWEGLTFAttribute::NORMAL))) {
			Normal.ReadValues<float>((float*)(VA + VertexNormalOffset), m_VerticeAttributeTypeSize, Normal.m_Count);
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f*)(VA + i * m_VerticeAttributeTypeSize + VertexNormalOffset);
					*P = *P * WorldTransform;
				}
			}
		}
		if (P.CreateAccessorView(Tangent, Prim.FindAttributeAccessor(LWEGLTFAttribute::TANGENT))) {
			Tangent.ReadValues<float>((float*)(VA + VertexTangentOffset), m_VerticeAttributeTypeSize, Tangent.m_Count);
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f*)(VA + i * m_VerticeAttributeTypeSize + VertexTangentOffset);
					float t = P->w;
					*P = (*P).xyz0() * WorldTransform;
					P->w = t;
				}
			}
		} else {
			LWELogWarn("Model has no tangents, attempting to generate them.");
			for (uint32_t i = 0; i < VertCnt; i++) {
				LWVector4f *Normal = (LWVector4f*)(VA + m_VerticeAttributeTypeSize * i + VertexNormalOffset);
				LWVector4f *Tangent = (LWVector4f*)(VA + m_VerticeAttributeTypeSize * i + VertexTangentOffset);
				LWVector3f R;
				LWVector3f U;
				Normal->xyz().Othogonal(R, U);
				*Tangent = LWVector4f(R, 1.0f);
			}
		}
		if (P.CreateAccessorView(BoneWeight, Prim.FindAttributeAccessor(LWEGLTFAttribute::WEIGHTS_0))) {
			BoneWeight.ReadValues<float>((float*)(VP + VertexWeightOffset), m_VerticePositionTypeSize, BoneWeight.m_Count);
		}
		if (P.CreateAccessorView(BoneIndices, Prim.FindAttributeAccessor(LWEGLTFAttribute::JOINTS_0))) {
			BoneIndices.ReadValues<int32_t>((int32_t*)(VP + VertexWeightIndicesOffset), m_VerticePositionTypeSize, BoneIndices.m_Count);
		}
		if (P.CreateAccessorView(Indice, Prim.m_IndiceID)) {
			Indice.ReadValues<uint32_t>(I, sizeof(uint32_t), Indice.m_Count);
			for (uint32_t n = 0; n < Indice.m_Count; n++) I[n] += v;
			IndiceCnt = Pm.m_Count = Indice.m_Count;
		}
		v += VertCnt;
		o += IndiceCnt;
		m_PrimitiveList.push_back(Pm);
	}
	return *this;
}

LWEMesh &LWEMesh::BuildAABB(const LWSMatrix4f &Transform, const LWSMatrix4f *BoneMatrixs) {
	LWSMatrix4f BoneMats[LWEMaxBones];
	if (!m_VerticePositions) return *this;
	auto BlendMatrix = [this](const LWVector4f &BoneWeight, const LWVector4i &BoneIdxs, const LWSMatrix4f *BoneMatrixs) -> LWSMatrix4f {
		float WeightSum = std::min<float>(std::max<float>(1.0f - (BoneWeight.x + BoneWeight.y + BoneWeight.z + BoneWeight.w), 0.0f), 1.0f);
		LWSMatrix4f Mat =
			BoneMatrixs[BoneIdxs.x] * BoneWeight.x +
			BoneMatrixs[BoneIdxs.y] * BoneWeight.y +
			BoneMatrixs[BoneIdxs.z] * BoneWeight.z +
			BoneMatrixs[BoneIdxs.w] * BoneWeight.w +
			LWSMatrix4f() * WeightSum;
		return Mat;
	};
	
	if (!BoneMatrixs) {
		BuildBindTransforms(BoneMats);
		BuildRenderMatrixs(BoneMats, BoneMats);
	} else BuildRenderMatrixs(BoneMatrixs, BoneMats);

	if (m_VerticePositionTypeSize == sizeof(LWEStaticVertice)) {
		LWEStaticVertice *VP = (LWEStaticVertice*)m_VerticePositions;
		LWSVector4f P = VP[0].m_Position * Transform;
		m_MinBounds = m_MaxBounds = P;
		for (uint32_t i = 1; i < m_VerticeCount; i++) {
			P = VP[i].m_Position * Transform;
			m_MinBounds = m_MinBounds.Min(P);
			m_MaxBounds = m_MaxBounds.Max(P);
		}
	} else if (m_VerticePositionTypeSize == sizeof(LWESkeletonVertice)) {
		LWESkeletonVertice *VP = (LWESkeletonVertice*)m_VerticePositions;
		LWSVector4f P = VP[0].m_Position * BlendMatrix(VP[0].m_BoneWeights, VP[0].m_BoneIndices, BoneMats) * Transform;
		m_MinBounds = m_MaxBounds = P;
		for (uint32_t i = 1; i < m_VerticeCount; i++) {
			P = VP[i].m_Position * BlendMatrix(VP[i].m_BoneWeights, VP[i].m_BoneIndices, BoneMats) * Transform;
			m_MinBounds = m_MinBounds.Min(P);
			m_MaxBounds = m_MaxBounds.Max(P);
		}
	}
	return *this;
}

LWEMesh &LWEMesh::MakeGLTFSkin(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFSkin *Skin, LWAllocator &Allocator) {
	if (!Skin->m_JointList.size()) return *this;
	if (Skin->m_JointList.size() > LWEMaxBones) LWELogWarn<256>("importing model with more bones than supported: {} ({})", (uint32_t)Skin->m_JointList.size(), LWEMaxBones);
	LWMatrix4f InvBindMatrixs[LWEMaxBones];
	m_BoneCount = (uint32_t)Skin->m_JointList.size();
	LWEGLTFAccessorView InvBindView;
	if (P.CreateAccessorView(InvBindView, Skin->m_InverseBindMatrices)) {
		InvBindView.ReadValues<float>(&InvBindMatrixs[0].m_Rows[0].x, sizeof(LWMatrix4f), m_BoneCount);
	}

	auto MapIDToList = [](std::vector<uint32_t> &List, uint32_t ID)->uint32_t {
		for (uint32_t i = 0; i < (uint32_t)List.size(); i++) {
			if (List[i] == ID) return i;
		}
		return -1;
	};

	std::function<void(LWEGLTFNode *, LWEGLTFSkin*)> ParseJointNode = [this, &P, &InvBindMatrixs, &ParseJointNode, &MapIDToList](LWEGLTFNode *N, LWEGLTFSkin *Skin) {
		uint32_t ID = MapIDToList(Skin->m_JointList, N->m_NodeID);
		if (!*N->m_Name) N->SetName(LWUTF8I::Fmt<64>("Bone_{}", ID));
		m_BoneList[ID] = LWEBone(N->GetName(), LWSMatrix4f(N->m_TransformMatrix), LWSMatrix4f(InvBindMatrixs[ID]));
		return;
	};

	std::function<void(LWEGLTFNode*, LWEGLTFSkin*)> ParseJointChildren = [this, &P, &ParseJointChildren, &MapIDToList](LWEGLTFNode *N, LWEGLTFSkin *Skin) {
		uint32_t ID = MapIDToList(Skin->m_JointList, N->m_NodeID);
		uint32_t pID = -1;
		for (auto &&Iter : N->m_Children) {
			uint32_t cID = MapIDToList(Skin->m_JointList, Iter);

			if (pID != -1) {
				m_BoneList[pID].m_NextBoneID = cID;
			} else m_BoneList[ID].m_ChildBoneID = cID;
			pID = cID;
		}
	};
	for (auto &&Iter : Skin->m_JointList) ParseJointNode(P.GetNode(Iter), Skin);
	for (auto &&Iter : Skin->m_JointList) ParseJointChildren(P.GetNode(Iter), Skin);
	return *this;
}

const LWEMesh &LWEMesh::TransformBounds(const LWSMatrix4f &Transform, LWSVector4f &MinBounds, LWSVector4f &MaxBounds) const {
	LWETransformAABB(m_MinBounds, m_MaxBounds, Transform, MinBounds, MaxBounds);
	return *this;
}

bool LWEMesh::UploadData(LWERenderer *Renderer, uint32_t NamedBlockGeometry, LWAllocator &Allocator, bool KeepLocal) {
	LWERendererBlockGeometry *Geom = Renderer->FindNamedBlockGeometryMap(NamedBlockGeometry);
	if (!Geom) return false;
	m_BlockGeometry = NamedBlockGeometry;
	m_ID = Geom->Allocate(m_VerticePositions, m_VerticeAttributes, m_Indices, m_VerticeCount, m_IndiceCount, Renderer, KeepLocal ? LWERenderPendingResource::NoDiscard : 0);
	if (m_ID == LWERendererBlockGeometry::NullID) return false;
	if (!KeepLocal) {
		m_VerticePositions = m_VerticeAttributes = nullptr;
		m_Indices = nullptr;
	}
	return true;
}

bool LWEMesh::PushPrimitive(const LWEPrimitive &Prim) {
	m_PrimitiveList.push_back(Prim);
	return true;
}

LWEMesh &LWEMesh::ReleaseData(LWERenderer *Renderer) {
	LWERendererBlockGeometry *Geom = Renderer->FindNamedBlockGeometryMap(m_BlockGeometry);
	if (!Geom) return *this;
	Geom->Free(m_ID);
	m_VerticePositions = LWAllocator::Destroy(m_VerticePositions);
	m_VerticeAttributes = LWAllocator::Destroy(m_VerticeAttributes);
	m_Indices = LWAllocator::Destroy(m_Indices);
	return *this;
}

const LWEMesh &LWEMesh::ApplyRotationTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const {
	LWSVector4f Pos = TransformMatrixs[BoneID][3];
	LWSMatrix4f Matrix = LWSMatrix4f::Translation(-Pos) * Transform;
	LWSVector4f R0 = Matrix[0];
	LWSVector4f R1 = Matrix[1];
	LWSVector4f R2 = Matrix[2];
	LWSVector4f R3 = Matrix[3];
	R3 += Pos.AAAB(LWSVector4f());
	return ApplyTransformToBone(BoneID, LWSMatrix4f(R0, R1, R2, R3), TransformMatrixs);
}

const LWEMesh &LWEMesh::ApplyTransformToBone(uint32_t BoneID, const LWSMatrix4f &Transform, LWSMatrix4f *TransformMatrixs) const {
	std::function<void(uint32_t)> DoTransform = [this, &DoTransform, &TransformMatrixs, &Transform](uint32_t i) {
		if (i == -1) return;
		const LWEBone &B = m_BoneList[i];
		TransformMatrixs[i] *= Transform;
		DoTransform(B.m_ChildBoneID);
		DoTransform(B.m_NextBoneID);
	};
	const LWEBone &B = m_BoneList[BoneID];
	TransformMatrixs[BoneID] *= Transform;
	DoTransform(B.m_ChildBoneID);
	return *this;
}

uint32_t LWEMesh::BuildBindTransforms(LWSMatrix4f *TransformMatrixs) const {
	if (!m_BoneCount) return 0;
	std::function<void(const LWSMatrix4f &, uint32_t)> DoTransform = [this, &DoTransform, &TransformMatrixs](const LWSMatrix4f &PTransform, uint32_t i) {
		if (i == -1) return;
		const LWEBone &B = m_BoneList[i];
		TransformMatrixs[i] = B.m_Transform * PTransform;
		DoTransform(TransformMatrixs[i], B.m_ChildBoneID);
		DoTransform(PTransform, B.m_NextBoneID);
		return;
	};
	if (!m_BoneCount) return m_BoneCount;
	DoTransform(LWSMatrix4f(), 0);
	return m_BoneCount;
}

uint32_t LWEMesh::BuildAnimationTransform(const LWSMatrix4f *AnimTransforms, LWSMatrix4f *TransformMatrixs) const {
	if (!AnimTransforms) return BuildBindTransforms(TransformMatrixs);
	std::function<void(const LWSMatrix4f &, uint32_t)> DoTransform = [this, &DoTransform, &AnimTransforms, &TransformMatrixs](const LWSMatrix4f &PTransform, uint32_t i) {
		if (i == -1) return;
		const LWEBone &B = m_BoneList[i];
		TransformMatrixs[i] = AnimTransforms[i] * PTransform;
		DoTransform(TransformMatrixs[i], B.m_ChildBoneID);
		DoTransform(PTransform, B.m_NextBoneID);
		return;
	};
	if (!m_BoneCount) return m_BoneCount;
	DoTransform(LWSMatrix4f(), 0);
	return m_BoneCount;
}

uint32_t LWEMesh::BuildRenderMatrixs(const LWSMatrix4f *TransformMatrixs, LWSMatrix4f *RenderMatrixs) const {
	for (uint32_t i = 0; i < m_BoneCount; i++) RenderMatrixs[i] = m_BoneList[i].m_InvBindMatrix * TransformMatrixs[i];
	return m_BoneCount;
}

const uint8_t *LWEMesh::GetVerticePositions(void) const {
	return m_VerticePositions;
}

uint8_t *LWEMesh::GetVerticePositions(void) {
	return m_VerticePositions;
}

const uint8_t *LWEMesh::GetVerticeAttributes(void) const {
	return m_VerticeAttributes;
}

uint8_t *LWEMesh::GetVerticeAttributes(void) {
	return m_VerticeAttributes;
}

LWSVector4f LWEMesh::GetMinBounds(void) const {
	return m_MinBounds;
}

LWSVector4f LWEMesh::GetMaxBounds(void) const {
	return m_MaxBounds;
}

const uint32_t *LWEMesh::GetIndices(void) const {
	return m_Indices;
}

uint32_t *LWEMesh::GetIndices(void) {
	return m_Indices;
}

const LWEPrimitive &LWEMesh::GetPrimitive(uint32_t Idx) const {
	return m_PrimitiveList[Idx];
}

const LWEBone &LWEMesh::GetBone(uint32_t Idx) const {
	return m_BoneList[Idx];
}

LWEBone &LWEMesh::GetBone(uint32_t Idx) {
	return m_BoneList[Idx];
}

uint32_t LWEMesh::GetBoneCount(void) const {
	return m_BoneCount;
}

uint32_t LWEMesh::GetPrimitiveCount(void) const {
	return (uint32_t)m_PrimitiveList.size();
}

uint32_t LWEMesh::GetVerticeCount(void) const {
	return m_VerticeCount;
}

uint32_t LWEMesh::GetIndiceCount(void) const {
	return m_IndiceCount;
}

uint32_t LWEMesh::GetVerticePositionTypeSize(void) const {
	return m_VerticePositionTypeSize;
}

uint32_t LWEMesh::GetVerticeAttributeTypeSize(void) const {
	return m_VerticeAttributeTypeSize;
}

uint64_t LWEMesh::GetID(void) const {
	return m_ID;
}

uint32_t LWEMesh::GetBlockGeometry(void) const {
	return m_BlockGeometry;
}

LWEMesh::~LWEMesh() {
	LWAllocator::Destroy(m_VerticePositions);
	LWAllocator::Destroy(m_VerticeAttributes);
	LWAllocator::Destroy(m_Indices);
}