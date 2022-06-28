#include "LWEMesh.h"
#include <LWESGeometry3D.h>
#include "LWERenderer.h"

//LWEPrimitive:
bool LWEPrimitive::Deserialize(LWEPrimitive &Prim, LWByteBuffer &BB) {
	Prim.m_Offset = BB.Read<uint32_t>();
	Prim.m_Count = BB.Read<uint32_t>();
	return true;
}

uint32_t LWEPrimitive::Serialize(LWByteBuffer &BB) const {
	uint32_t o = 0;
	o += BB.Write(m_Offset);
	o += BB.Write(m_Count);
	return o;
}

//LWEBone:

bool LWEBone::Deserialize(LWEBone &Bone, LWByteBuffer &BB) {
	BB.ReadUTF(Bone.m_Name, sizeof(Bone.m_Name));
	Bone.m_InvBindMatrix = BB.ReadSMat4<float>();
	Bone.m_Transform = BB.ReadSMat4<float>();
	Bone.m_NextBoneID = BB.Read<uint32_t>();
	Bone.m_ChildBoneID = BB.Read<uint32_t>();
	Bone.m_NameHash = Bone.GetName().Hash();
	return true;
}

uint32_t LWEBone::Serialize(LWByteBuffer &BB) const {
	uint32_t o = 0;
	o += BB.WriteUTF(GetName());
	o += BB.Write(m_InvBindMatrix);
	o += BB.Write(m_Transform);
	o += BB.Write(m_NextBoneID);
	o += BB.Write(m_ChildBoneID);
	return o;
}

LWUTF8Iterator LWEBone::GetName(void) const {
	return m_Name;
}

LWEBone::LWEBone(const LWUTF8Iterator &Name, const LWSMatrix4f &Transform, const LWSMatrix4f &InvTransform) : m_NameHash(Name.Hash()), m_Transform(Transform), m_InvBindMatrix(InvTransform) {
	Name.Copy(m_Name, sizeof(m_Name));
}

//LWEMesh:
bool LWEMesh::Deserialize(LWEMesh &Mesh, LWByteBuffer &BB, LWAllocator &Allocator) {
	LWSVector4f MinBounds, MaxBounds;

	MinBounds = BB.ReadSVec4<float>();
	MaxBounds = BB.ReadSVec4<float>();
	uint32_t VerticeTypeSize = BB.Read<uint32_t>();
	uint32_t AttributeTypeSize = BB.Read<uint32_t>();
	uint32_t VerticeCount = BB.Read<uint32_t>();
	uint32_t IndiceCount = BB.Read<uint32_t>();
	uint32_t PrimCount = BB.Read<uint32_t>();
	uint32_t BoneCount = BB.Read<uint32_t>();

	Mesh = LWEMesh(Allocator, VerticeTypeSize, AttributeTypeSize, VerticeCount, IndiceCount, nullptr, 0);
	BB.Read<uint8_t>(Mesh.GetVerticePositions(), VerticeTypeSize * VerticeCount);
	BB.Read<uint8_t>(Mesh.GetVerticeAttributes(), AttributeTypeSize * VerticeCount);
	BB.Read<uint32_t>(Mesh.GetIndices(), IndiceCount);
	for (uint32_t i = 0; i < PrimCount; i++) {
		LWEPrimitive P;
		if (!LWEPrimitive::Deserialize(P, BB)) return false;
		Mesh.PushPrimitive(P);
	}
	for (uint32_t i = 0; i < BoneCount; i++) {
		if (!LWEBone::Deserialize(Mesh.GetBone(i), BB)) return false;
	}
	Mesh.SetBoneCount(BoneCount);
	Mesh.SetBounds(MinBounds, MaxBounds);
	return true;
}

LWEMesh &LWEMesh::MakeGLTFMesh(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, bool BakeTransform) {
	const uint32_t VertexPositionOffset = 0;
	const uint32_t VertexWeightOffset = 16;
	const uint32_t VertexWeightIndicesOffset = 32;

	const uint32_t VertexTexCoordOffset = 0;
	const uint32_t VertexTangentOffset = 16;
	const uint32_t VertexNormalOffset = 32;

	m_VerticePositionTypeSize = sizeof(LWEStaticVertice);
	m_VerticeAttributeTypeSize = sizeof(LWEVerticePBR);
	//Find if we are loading a static vertice, or skeleton vertice:
	for (auto &&Prim : GMesh->m_Primitives) {
		if (Prim.FindAttributeAccessor(LWEGLTFAttribute::JOINTS_0) != -1) {
			m_VerticePositionTypeSize = sizeof(LWESkeletonVertice);
		}
	}
	return MakeGLTFMeshCustom(P, Source, GMesh, Allocator, m_VerticePositionTypeSize, m_VerticeAttributeTypeSize, VertexPositionOffset, VertexWeightOffset, VertexWeightIndicesOffset, VertexNormalOffset, VertexTangentOffset, VertexTexCoordOffset, -1, -1, BakeTransform);
}

LWEMesh &LWEMesh::MakeGLTFMeshCustom(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFMesh *GMesh, LWAllocator &Allocator, uint32_t VerticePosTypeSize, uint32_t VerticeAttrTypeSize, uint32_t PositionOffset, uint32_t WeightOffset, uint32_t JointOffset, uint32_t NormalOffset, uint32_t TangentOffset, uint32_t TexCoord0Offset, uint32_t TexCoord1Offset, uint32_t ColorOffset, bool BakeTransform) {
	m_VerticeCount = 0;
	m_IndiceCount = 0;
	m_VerticePositionTypeSize = VerticePosTypeSize;
	m_VerticeAttributeTypeSize = VerticeAttrTypeSize;
	LWSMatrix4f WorldTransform = P.GetNodeWorldTransform(Source->m_NodeID).AsSMat4();
	for (auto &&Prim : GMesh->m_Primitives) {
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
		for (uint32_t i = 0; i < m_VerticeCount; i++) *(LWSVector4f *)(m_VerticePositions + PositionOffset + i * m_VerticePositionTypeSize) = LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f);
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
		LWEGLTFAccessorView TexCoord0;
		LWEGLTFAccessorView TexCoord1;
		LWEGLTFAccessorView Tangent;
		LWEGLTFAccessorView Normal;
		LWEGLTFAccessorView Color;
		LWEGLTFAccessorView BoneWeight;
		LWEGLTFAccessorView BoneIndices;
		LWEGLTFAccessorView Indice;
		if (PositionOffset!=-1 && P.CreateAccessorView(Position, Prim.FindAttributeAccessor(LWEGLTFAttribute::POSITION))) {
			Position.ReadValues<float>((float *)(VP + PositionOffset), m_VerticePositionTypeSize, Position.m_Count);
			VertCnt = Pm.m_Count = Position.m_Count;
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f *)(VP + i * m_VerticePositionTypeSize + PositionOffset);
					*P = *P * WorldTransform;
				}
			}
		}
		if (TexCoord0Offset!=-1 && P.CreateAccessorView(TexCoord0, Prim.FindAttributeAccessor(LWEGLTFAttribute::TEXCOORD_0))) {
			TexCoord0.ReadValues<float>((float *)(VA + TexCoord0Offset), m_VerticeAttributeTypeSize, TexCoord0.m_Count);
		}
		if (TexCoord1Offset=-1 && P.CreateAccessorView(TexCoord1, Prim.FindAttributeAccessor(LWEGLTFAttribute::TEXCOORD_1))) {
			TexCoord1.ReadValues<float>((float*)(VA+TexCoord1Offset), m_VerticeAttributeTypeSize, TexCoord1.m_Count);
		}
		if(ColorOffset!=-1 && P.CreateAccessorView(Color, Prim.FindAttributeAccessor(LWEGLTFAttribute::COLOR_0))) {
			Color.ReadValues<float>((float*)(VA+ColorOffset), m_VerticeAttributeTypeSize, Color.m_Count);
		}
		if (NormalOffset!=-1 && P.CreateAccessorView(Normal, Prim.FindAttributeAccessor(LWEGLTFAttribute::NORMAL))) {
			Normal.ReadValues<float>((float *)(VA + NormalOffset), m_VerticeAttributeTypeSize, Normal.m_Count);
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f *)(VA + i * m_VerticeAttributeTypeSize + NormalOffset);
					*P = *P * WorldTransform;
				}
			}
		}
		if (TangentOffset!=-1 && P.CreateAccessorView(Tangent, Prim.FindAttributeAccessor(LWEGLTFAttribute::TANGENT))) {
			Tangent.ReadValues<float>((float *)(VA + TangentOffset), m_VerticeAttributeTypeSize, Tangent.m_Count);
			if (BakeTransform) {
				for (uint32_t i = 0; i < VertCnt; i++) {
					LWSVector4f *P = (LWSVector4f *)(VA + i * m_VerticeAttributeTypeSize + TangentOffset);
					float t = P->w;
					*P = (*P).xyz0() * WorldTransform;
					P->w = t;
				}
			}
		} else if(TangentOffset!=-1) {
			LWLogWarn("Model has no tangents, attempting to generate them.");
			for (uint32_t i = 0; i < VertCnt; i++) {
				LWVector4f *Normal = (LWVector4f *)(VA + m_VerticeAttributeTypeSize * i + NormalOffset);
				LWVector4f *Tangent = (LWVector4f *)(VA + m_VerticeAttributeTypeSize * i + TangentOffset);
				LWVector3f R;
				LWVector3f U;
				Normal->xyz().Othogonal(R, U);
				*Tangent = LWVector4f(R, 1.0f);
			}
		}
		if (WeightOffset!=-1 && P.CreateAccessorView(BoneWeight, Prim.FindAttributeAccessor(LWEGLTFAttribute::WEIGHTS_0))) {
			BoneWeight.ReadValues<float>((float *)(VP + WeightOffset), m_VerticePositionTypeSize, BoneWeight.m_Count);
		}
		if (JointOffset!=-1 && P.CreateAccessorView(BoneIndices, Prim.FindAttributeAccessor(LWEGLTFAttribute::JOINTS_0))) {
			BoneIndices.ReadValues<int32_t>((int32_t *)(VP + JointOffset), m_VerticePositionTypeSize, BoneIndices.m_Count);
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
	if(m_VerticePositionTypeSize==sizeof(LWEStaticVertice)) return BuildVertexAABB(Transform, BoneMatrixs, offsetof(LWEStaticVertice, m_Position));
	else if(m_VerticePositionTypeSize==sizeof(LWESkeletonVertice)) return BuildVertexAABB(Transform, BoneMatrixs, offsetof(LWESkeletonVertice, m_Position), offsetof(LWESkeletonVertice, m_BoneWeights), offsetof(LWESkeletonVertice, m_BoneIndices));
	return *this;
}

LWEMesh &LWEMesh::BuildVertexAABB(const LWSMatrix4f &Transform, const LWSMatrix4f *BoneMatrixs, uint32_t PositionOffset, uint32_t BoneWeightOffset, uint32_t BoneIndexOffset) {
	LWSMatrix4f BoneMats[LWEMaxBones];
	if (!m_VerticePositions || !m_VerticeCount) return *this;
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
	}
	else BuildRenderMatrixs(BoneMatrixs, BoneMats);

	if (BoneWeightOffset == BoneIndexOffset) {
		LWSVector4f *VP = (LWSVector4f *)(m_VerticePositions + PositionOffset);
		m_MinBounds = m_MaxBounds = VP[0] * Transform;
		for (uint32_t i = 1; i < m_VerticeCount; ++i) {
			VP = (LWSVector4f *)(m_VerticePositions + PositionOffset + m_VerticePositionTypeSize * i);
			LWSVector4f P = VP[0] * Transform;
			m_MinBounds = m_MinBounds.Min(P);
			m_MaxBounds = m_MaxBounds.Max(P);
		}
	}
	else {
		LWSVector4f *VP = (LWSVector4f *)(m_VerticePositions + PositionOffset);
		LWVector4f *VW = (LWVector4f *)(m_VerticePositions + BoneWeightOffset);
		LWVector4i *VI = (LWVector4i *)(m_VerticePositions + BoneIndexOffset);

		LWSVector4f P = VP[0] * BlendMatrix(VW[0], VI[0], BoneMats) * Transform;
		m_MinBounds = m_MaxBounds = P;
		for (uint32_t i = 1; i < m_VerticeCount; ++i) {
			VP = (LWSVector4f *)(m_VerticePositions + PositionOffset + m_VerticePositionTypeSize * i);
			VW = (LWVector4f *)(m_VerticePositions + BoneWeightOffset + m_VerticePositionTypeSize * i);
			VI = (LWVector4i *)(m_VerticePositions + BoneIndexOffset + m_VerticePositionTypeSize * i);
			P = VP[0] * BlendMatrix(VW[0], VI[0], BoneMats) * Transform;
			m_MinBounds = m_MinBounds.Min(P);
			m_MaxBounds = m_MaxBounds.Max(P);
		}
	}
	return *this;
}

LWEMesh &LWEMesh::MakeGLTFSkin(LWEGLTFParser &P, LWEGLTFNode *Source, LWEGLTFSkin *Skin, LWAllocator &Allocator) {
	if (!Skin->m_JointList.size()) return *this;
	if (Skin->m_JointList.size() > LWEMaxBones) LWLogWarn<256>("importing model with more bones than supported: {} ({})", (uint32_t)Skin->m_JointList.size(), LWEMaxBones);
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

bool LWEMesh::UploadData(LWERenderer *Renderer, const LWUTF8Iterator &NamedBlockGeometry, LWAllocator &Allocator, bool KeepLocal) {
	return UploadData(Renderer, NamedBlockGeometry.Hash(), Allocator, KeepLocal);
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
	for (uint32_t i = 0; i < m_BoneCount; i++) {
		RenderMatrixs[i] = m_BoneList[i].m_InvBindMatrix * TransformMatrixs[i];
	}
	return m_BoneCount;
}

void LWEMesh::SetBounds(const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds) {
	m_MinBounds = MinBounds;
	m_MaxBounds = MaxBounds;
	return;
}

void LWEMesh::SetBoneCount(uint32_t Count) {
	m_BoneCount = Count;
	return;
}

uint32_t LWEMesh::Serialize(LWByteBuffer &Buffer) const {
	if (!LWLogWarnIf<128>(m_VerticePositions && m_VerticeAttributes && m_Indices, "Cannot serialize mesh, missing required data.")) return 0;
	uint32_t o = 0;
	o += Buffer.Write(m_MinBounds);
	o += Buffer.Write(m_MaxBounds);
	o += Buffer.Write(m_VerticePositionTypeSize);
	o += Buffer.Write(m_VerticeAttributeTypeSize);
	o += Buffer.Write(m_VerticeCount);
	o += Buffer.Write(m_IndiceCount);
	o += Buffer.Write((uint32_t)m_PrimitiveList.size());
	o += Buffer.Write(m_BoneCount);
	o += Buffer.Write(m_VerticePositionTypeSize * m_VerticeCount, m_VerticePositions);
	o += Buffer.Write(m_VerticeAttributeTypeSize * m_VerticeCount, m_VerticeAttributes);
	o += Buffer.Write(m_IndiceCount, m_Indices);
	for (auto &P : m_PrimitiveList) o += P.Serialize(Buffer);
	for (uint32_t i = 0; i < m_BoneCount; i++) o += m_BoneList[i].Serialize(Buffer);
	return o;
}

LWEMesh &LWEMesh::operator = (LWEMesh &&Other) {
	m_VerticePositions = LWAllocator::Destroy(m_VerticePositions);
	m_VerticeAttributes = LWAllocator::Destroy(m_VerticeAttributes);
	m_Indices = LWAllocator::Destroy(m_Indices);

	m_VerticePositions = std::exchange(Other.m_VerticePositions, nullptr);
	m_VerticeAttributes = std::exchange(Other.m_VerticeAttributes, nullptr);
	m_Indices = std::exchange(Other.m_Indices, nullptr);
	m_VerticePositionTypeSize = Other.m_VerticePositionTypeSize;
	m_VerticeAttributeTypeSize = Other.m_VerticeAttributeTypeSize;
	m_VerticeCount = Other.m_VerticeCount;
	m_IndiceCount = Other.m_IndiceCount;
	m_MinBounds = Other.m_MinBounds;
	m_MaxBounds = Other.m_MaxBounds;
	m_PrimitiveList = Other.m_PrimitiveList;
	m_BoneCount = Other.m_BoneCount;
	m_ID = std::exchange(Other.m_ID, -1);
	m_BlockGeometry = std::exchange(Other.m_BlockGeometry, LWUTF8I::EmptyHash);
	std::copy(Other.m_BoneList, Other.m_BoneList + Other.m_BoneCount, m_BoneList);
	return *this;
}

LWEMesh &LWEMesh::operator = (const LWEMesh &Other) {
	LWAllocator *Alloc = LWAllocator::GetAllocator(m_VerticePositions);
	m_VerticePositions = LWAllocator::Destroy(m_VerticePositions);
	m_VerticeAttributes = LWAllocator::Destroy(m_VerticeAttributes);
	m_Indices = LWAllocator::Destroy(m_Indices);

	m_VerticePositionTypeSize = Other.m_VerticePositionTypeSize;
	m_VerticeAttributeTypeSize = Other.m_VerticeAttributeTypeSize;
	m_VerticeCount = Other.m_VerticeCount;
	m_IndiceCount = Other.m_IndiceCount;
	m_MinBounds = Other.m_MinBounds;
	m_MaxBounds = Other.m_MaxBounds;
	m_PrimitiveList = Other.m_PrimitiveList;
	m_BoneCount = Other.m_BoneCount;
	m_ID = -1;
	m_BlockGeometry = LWUTF8I::EmptyHash;
	std::copy(Other.m_BoneList, Other.m_BoneList + Other.m_BoneCount, m_BoneList);

	if (Alloc) {
		uint32_t PLen = m_VerticePositionTypeSize * m_VerticeCount;
		uint32_t ALen = m_VerticeAttributeTypeSize * m_VerticeCount;
		m_VerticePositions = Alloc->Allocate<uint8_t>(PLen);
		m_VerticeAttributes = Alloc->Allocate<uint8_t>(ALen);
		m_Indices = Alloc->Allocate<uint32_t>(m_IndiceCount);
		std::copy(Other.m_VerticePositions, Other.m_VerticePositions + PLen, m_VerticePositions);
		std::copy(Other.m_VerticeAttributes, Other.m_VerticeAttributes + ALen, m_VerticeAttributes);
		std::copy(Other.m_Indices, Other.m_Indices + m_IndiceCount, m_Indices);
	}
	return *this;
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

LWEMesh::LWEMesh(LWAllocator &Allocator, uint32_t VerticePosTypeSize, uint32_t VerticeAttrTypeSize, uint32_t VerticeCount, uint32_t IndiceCount, LWEBone *BoneList, uint32_t BoneCount) : m_VerticePositions(Allocator.Allocate<uint8_t>(VerticeCount * VerticePosTypeSize)), m_VerticeAttributes(Allocator.Allocate<uint8_t>(VerticeCount * VerticeAttrTypeSize)), m_Indices(Allocator.Allocate<uint32_t>(IndiceCount)), m_VerticePositionTypeSize(VerticePosTypeSize), m_VerticeAttributeTypeSize(VerticeAttrTypeSize), m_VerticeCount(VerticeCount), m_IndiceCount(IndiceCount), m_BoneCount(BoneCount) {
	assert(BoneCount <= LWEMaxBones);
	std::copy(BoneList, BoneList + BoneCount, m_BoneList);
}

LWEMesh::LWEMesh(LWEMesh &&Other) {
	*this = std::move(Other);
}

LWEMesh::LWEMesh(const LWEMesh &Other) {
	*this = Other;
}

LWEMesh::~LWEMesh() {
	LWAllocator::Destroy(m_VerticePositions);
	LWAllocator::Destroy(m_VerticeAttributes);
	LWAllocator::Destroy(m_Indices);
}