#include <LWCore/LWAllocator.h>
#include <LWCore/LWLogger.h>
#include <LWPlatform/LWWindow.h>
#include "LWERenderFrame.h"
#include "LWECamera.h"
#include "LWERenderer.h"
#include "LWEMesh.h"
#include <LWESGeometry3D.h>
#include <numeric>

const float LWEGeometryBucketItem::ForceFirstDistance = -1.0f;
const float LWEGeometryBucketItem::ForceLastDistance = std::numeric_limits<float>::max();

//LWEGeometryModelBlock:
uint32_t LWEGeometryModelBlock::Hash(void) const {
	return LWCrypto::HashFNV1A((uint8_t*)this,sizeof(this));
}

bool LWEGeometryModelBlock::isVideoBuffer(void) const {
	return m_BlockID == -1;
}

LWEGeometryModelBlock::LWEGeometryModelBlock(LWERendererBlockGeometry *Block, uint64_t BlockID, uint32_t Offset, uint32_t Count) : m_BlockID(BlockID), m_BufferName((uint64_t)Block->GetNameHash()), m_Offset(Offset), m_Count(Count) {}

LWEGeometryModelBlock::LWEGeometryModelBlock(uint32_t BufferName, uint64_t BlockID, uint32_t Offset, uint32_t Count) : m_BlockID(BlockID), m_BufferName((uint64_t)BufferName), m_Offset(Offset), m_Count(Count) {}

LWEGeometryModelBlock::LWEGeometryModelBlock(uint32_t VertexPositionBufferID, uint32_t VertexAttributeBufferID, uint32_t IndiceBufferID, uint32_t Offset, uint32_t Count) : m_BufferName(((uint64_t)VertexPositionBufferID << VertexPositionVBBitsOffset) | ((uint64_t)VertexAttributeBufferID << VertexAttributeVBBitsOffset) | ((uint64_t)IndiceBufferID << IndiceVBBitsOffset)), m_Offset(Offset), m_Count(Count) {}

LWEGeometryModelBlock::LWEGeometryModelBlock(const LWERenderVideoBuffer &VertexPositionBuffer, const LWERenderVideoBuffer &VertexAttributeBuffer, const LWERenderVideoBuffer &IndiceBuffer, uint32_t Offset, uint32_t Count) : m_BufferName(((uint64_t)VertexPositionBuffer.m_ID << VertexPositionVBBitsOffset) | ((uint64_t)VertexAttributeBuffer.m_ID << VertexAttributeVBBitsOffset) | ((uint64_t)IndiceBuffer.m_ID << IndiceVBBitsOffset)), m_Offset(Offset), m_Count(Count) {}

//LWEGeometryModel:
bool LWEGeometryModel::isTransparent(void) const {
	return (m_Flag & Transparent) != 0;
}

uint32_t LWEGeometryModel::GetDistanceMode(void) const {
	return LWBitFieldGet(DistanceModeBits, m_Flag);
}

LWEGeometryModel::LWEGeometryModel(const LWEGeometryModelBlock &Block, const LWERenderMaterial &Material, uint32_t Flag) : m_Material(Material), m_GeometryBlock(Block), m_Flag(Flag) {}


//LWEGeometryBucketItem:
bool LWEGeometryBucketItem::SortFrontToBack(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B) {
	return A.m_DistanceSq < B.m_DistanceSq;
}

bool LWEGeometryBucketItem::SortBackToFront(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B) {
	return A.m_DistanceSq > B.m_DistanceSq;
}

bool LWEGeometryBucketItem::SortState(const LWEGeometryBucketItem &A, const LWEGeometryBucketItem &B) {
	if (B.m_DistanceSq == ForceFirstDistance || A.m_DistanceSq==ForceLastDistance) return false;
	if (A.m_DistanceSq == ForceFirstDistance || B.m_DistanceSq==ForceLastDistance) return true;
	return (A.m_MaterialHash < B.m_MaterialHash || (A.m_MaterialHash == B.m_MaterialHash && A.m_BlockHash < B.m_BlockHash));// || (A.m_DistanceSq == ForceFirstDistance)) && A.m_DistanceSq != ForceLastDistance;
}

LWEGeometryBucketItem::LWEGeometryBucketItem(uint32_t ModelIndex, uint32_t MaterialHash, uint32_t BlockHash, float DistanceSq) : m_ModelIndex(ModelIndex), m_MaterialHash(MaterialHash), m_BlockHash(BlockHash), m_DistanceSq(DistanceSq) {}

//LWELightBucketItem:

bool LWEShadowBucketItem::SortFrontToBack(const LWEShadowBucketItem &A, const LWEShadowBucketItem &B) {
	return A.m_DistanceSq < B.m_DistanceSq;
}

bool LWEShadowBucketItem::SortBackToFront(const LWEShadowBucketItem &A, const LWEShadowBucketItem &B) {
	return A.m_DistanceSq > B.m_DistanceSq;
}

LWEShadowBucketItem::LWEShadowBucketItem(uint32_t LightIndex, float DistanceSq) : m_LightIndex(LightIndex), m_DistanceSq(DistanceSq) {}


//LWEGeometryBucket:
bool LWEGeometryBucket::PushModel(uint32_t ModelIndex, uint32_t ModelBlockHash, uint32_t MaterialHash, const LWEGeometryModel &Model, const LWSVector4f &Position) {
	float DisSq = (Position - m_ViewTransform[3]).LengthSquared3();
	uint32_t DistanceMode = Model.GetDistanceMode();
	if (DistanceMode == LWEGeometryModel::ForceDrawFirst) DisSq = LWEGeometryBucketItem::ForceFirstDistance;
	else if (DistanceMode == LWEGeometryModel::ForceDrawLast) DisSq = LWEGeometryBucketItem::ForceLastDistance;
	if (Model.isTransparent()) {
		uint32_t Index = m_TransparentCount++;
		if(!LWLogCriticalIf(Index<LWEMaxBucketSize, "Bucket transparent object's has been exhausted.")) return false;
		m_TransparentItems[Index] = LWEGeometryBucketItem(ModelIndex, MaterialHash, ModelBlockHash, DisSq);
	} else {
		uint32_t Index = m_OpaqueCount++;
		if(!LWLogCriticalIf(Index<LWEMaxBucketSize, "Bucket opaque object's has been exhausted.")) return false;
		m_OpaqueItems[Index] = LWEGeometryBucketItem(ModelIndex, MaterialHash, ModelBlockHash, DisSq);
	}
	return true;
}

bool LWEGeometryBucket::SphereInFrustum(const LWSVector4f &Position, float Radius) {
	if (!isInitialized()) return false;
	return LWESphereInFrustum(Position, Radius, m_ViewTransform[3], m_Frustum);
}

bool LWEGeometryBucket::AABBInFrustum(const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds) {
	if (!isInitialized()) return false;
	return LWEAABBInFrustum(MinBounds, MaxBounds, m_ViewTransform[3], m_Frustum);
}

bool LWEGeometryBucket::ConeInFrustum(const LWSVector4f &Position, const LWSVector4f &Direction, float Theta, float Length) {
	if (!isInitialized()) return false;
	return LWEConeInFrustum(Position, Direction, Theta, Length, m_ViewTransform[3], m_Frustum);
}

LWEGeometryBucket &LWEGeometryBucket::SetOpaqueSort(uint32_t SortMode) {
	m_Flag = LWBitFieldSet(OpaqueSortBits, m_Flag, SortMode);
	return *this;
}

LWEGeometryBucket &LWEGeometryBucket::SetTransparentSort(uint32_t SortMode) {
	m_Flag = LWBitFieldSet(TransparentSortBits, m_Flag, SortMode);
	return *this;
}

std::pair<uint32_t, uint32_t> LWEGeometryBucket::Finalize(LWERenderer *Renderer, LWERenderFrame &Frame, LWVideoBuffer *IndirectBuffer, LWVideoBuffer *IDBuffer, LWEGeometryRenderable RenderableList[LWEMaxBucketSize]) {
	std::function<bool(const LWEGeometryBucketItem &, const LWEGeometryBucketItem &)> SortFuncs[4] = { &LWEGeometryBucketItem::SortState, &LWEGeometryBucketItem::SortBackToFront, &LWEGeometryBucketItem::SortFrontToBack, nullptr };
	if (!isInitialized()) return { 0,0 };
	LWVideoDriver *Driver = Renderer->GetVideoDriver();
	uint32_t oSortMode = GetOpaqueSortMode();
	uint32_t tSortMode = GetTransparentSortMode();
	uint32_t OpaqueCount = m_OpaqueCount;
	uint32_t TransparentCount = m_TransparentCount;
	if(oSortMode!=SortNone) std::sort(m_OpaqueItems, m_OpaqueItems + OpaqueCount, SortFuncs[oSortMode]);
	if (tSortMode != SortNone) std::sort(m_TransparentItems, m_TransparentItems + TransparentCount, SortFuncs[tSortMode]);

	uint32_t *IDList = Driver->MapVideoBuffer<uint32_t>(IDBuffer);
	LWIndirectIndice *IndirectList = Driver->MapVideoBuffer<LWIndirectIndice>(IndirectBuffer);
	std::pair<uint32_t, uint32_t> RenderCountPair = { 0,0 };
	uint32_t IndirectCount = 0;
	uint32_t IDCount = 0;
	for (uint32_t n = 0; n < 2; n++) {
		const LWEGeometryModel *PrevMdl = nullptr;
		LWEGeometryBucketItem *PrevItm = nullptr;
		LWIndirectIndice *PrevIndirect = nullptr;
		uint32_t Cnt = n == 0 ? OpaqueCount : TransparentCount;
		uint32_t &RenderCount = n == 0 ? RenderCountPair.first : RenderCountPair.second;
		uint32_t RenderOffset = n == 0 ? 0 : RenderCountPair.first;
		for (uint32_t i = 0; i < Cnt; i++) {
			LWEGeometryBucketItem &Itm = n==0?m_OpaqueItems[i]:m_TransparentItems[i];
			const LWEGeometryModel &Mdl = Frame.GetModel(Itm.m_ModelIndex);
			const LWEGeometryModelBlock &Block = Mdl.m_GeometryBlock;
			IDList[IDCount] = Itm.m_ModelIndex;
			if (Block.isVideoBuffer()) {
				uint32_t Cnt = Block.m_Count;
				uint32_t Offset = Block.m_Offset;
				if (!Cnt) {
					uint32_t IndiceID = LWBitFieldGet(LWEGeometryModelBlock::IndiceVBBits, Block.m_BufferName);
					LWVideoBuffer *IndiceVB = Renderer->GetVideoBuffer(IndiceID);
					assert(IndiceVB != nullptr);
					Cnt = IndiceVB->GetLength();
					Offset = 0;
				}
				IndirectList[IndirectCount] = { Cnt, 1, Offset, 0, IDCount };
			} else {
				LWERendererBlockGeometry *BlockGeom = Renderer->FindNamedBlockGeometryMap((uint32_t)Block.m_BufferName);
				assert(BlockGeom != nullptr);
				IndirectList[IndirectCount] = BlockGeom->MakeDrawCall(Block.m_BlockID, IDCount, Block.m_Offset, Block.m_Count);
			}
			RenderableList[RenderOffset+RenderCount] = LWEGeometryRenderable(Block.m_BufferName, Mdl.m_Material, 1 | (Block.isVideoBuffer() ? LWEGeometryRenderable::BufferVideoBuffer : 0));
			IDCount++;
			if (RenderCount) {
				if (Block.m_BufferName == PrevMdl->m_GeometryBlock.m_BufferName && Itm.m_MaterialHash == PrevItm->m_MaterialHash) {
					if (IndirectList[IndirectCount].m_IndexCount == PrevIndirect->m_IndexCount && IndirectList[IndirectCount].m_IndexOffset == PrevIndirect->m_IndexOffset && IndirectList[IndirectCount].m_IndexVertexOffset == PrevIndirect->m_IndexVertexOffset) {
						PrevIndirect->m_InstanceCount++;
					} else {
						PrevIndirect = &IndirectList[IndirectCount++];
						RenderableList[RenderOffset+RenderCount - 1].m_DrawCount++;
					}
				} else {
					PrevIndirect = &IndirectList[IndirectCount++];
					RenderCount++;
				}
			} else {
				PrevIndirect = &IndirectList[IndirectCount++];
				RenderCount++;
			}
			PrevItm = &Itm;
			PrevMdl = &Mdl;
		}
	}
	Driver->UnmapVideoBuffer(IndirectBuffer);
	Driver->UnmapVideoBuffer(IDBuffer);
	return RenderCountPair;
}

void LWEGeometryBucket::Reset(void) {
	m_Flag &= ~Initialized;
	m_OpaqueCount = m_TransparentCount = 0;
	return;
}

bool LWEGeometryBucket::Initialize(const LWECamera &View) {
	std::copy(View.GetViewFrustrum(), View.GetViewFrustrum() + 6, m_Frustum);
	View.BuildFrustrumPoints(m_FrustumPoints);
	m_ViewTransform = View.GetDirectionMatrix();
	m_Flag |= Initialized;
	return true;
}

bool LWEGeometryBucket::isInitialized(void) const {
	return (m_Flag & Initialized) != 0;
}

uint32_t LWEGeometryBucket::GetTransparentSortMode(void) const {
	return LWBitFieldGet(TransparentSortBits, m_Flag);
}

uint32_t LWEGeometryBucket::GetOpaqueSortMode(void) const {
	return LWBitFieldGet(OpaqueSortBits, m_Flag);
}

//LWERenderFrame:
bool LWERenderFrame::InitializeBucket(const LWECamera &View, uint32_t PassBucketID, uint32_t BucketIdxID) {
	uint32_t PassBit = 1 << PassBucketID;
	for (uint32_t i = 0; i < m_PassBucketCount; i++) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		LWEShaderGeometryBucketData &sBucket = m_GlobalData.m_BucketList[i];
		if (Bucket.m_PassBit == PassBit) {
			if (!BucketIdxID) {
				if (!Bucket.Initialize(View)) return false;
				std::copy(Bucket.m_Frustum, Bucket.m_Frustum + 6, sBucket.m_Frustum);
				std::copy(Bucket.m_FrustumPoints, Bucket.m_FrustumPoints, sBucket.m_FrustumPoints);
				sBucket.m_ViewTransform = Bucket.m_ViewTransform;
				sBucket.m_ProjViewTransform = View.GetProjViewMatrix();
				return true;
			} else BucketIdxID--;
		}
	}
	return false;
}

uint32_t LWERenderFrame::SpherePassBitsInBuckets(uint32_t PassBits, const LWSVector4f &Position, float Radius) {
	uint32_t Result = 0;
	for (uint32_t i = 0; i < m_PassBucketCount; i++) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		if((Bucket.m_PassBit&PassBits)==0) continue;
		if (Bucket.SphereInFrustum(Position, Radius)) Result |= Bucket.m_PassBit;
	}
	return Result;
}

uint32_t LWERenderFrame::AABBPassBitsInBuckets(uint32_t PassBits, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds) {
	uint32_t Result = 0;
	for (uint32_t i = 0; i < m_PassBucketCount; i++) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		if ((Bucket.m_PassBit & PassBits) == 0) continue;
		if (Bucket.AABBInFrustum(MinBounds, MaxBounds)) Result |= Bucket.m_PassBit;
	}
	return Result;
}

bool LWERenderFrame::WriteDebugLine(uint32_t PassBits, const LWSVector4f &APnt, const LWSVector4f &BPnt, float Thickness, const LWVector4f &Color, uint32_t Flags) {
	assert(m_DebugGeometry != nullptr);
	LWSVector4f Dir = (BPnt - APnt);
	float Len = Dir.Length3();
	LWSVector4f nDir = Dir.Normalize3();
	LWSMatrix4f Rot;
	LWSVector4f Up;
	LWSVector4f Right;
	nDir.Orthogonal3(Right, Up);
	Rot = LWSMatrix4f(Right, Up, nDir, LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f));
	float hLen = Len * 0.5f;
	LWSVector4f MidPnt = APnt + nDir * hLen;
	LWSMatrix4f Transform = LWSMatrix4f(Thickness, Thickness, hLen, 1.0f) * Rot * LWSMatrix4f::Translation(MidPnt);
	return WriteDebugGeometry(PassBits, m_DebugGeometry->GetBoxPrimitive(), Transform, Color, APnt.Min(BPnt) - LWVector4f(Thickness, Thickness, Thickness, 0.0f), APnt.Max(BPnt) + LWVector4f(Thickness, Thickness, Thickness, 0.0f), Flags);
}

bool LWERenderFrame::WriteDebugPoint(uint32_t PassBits, const LWSVector4f &Ctr, float Radius, const LWVector4f &Color, uint32_t Flags) {
	assert(m_DebugGeometry != nullptr);
	LWSMatrix4f Transform = LWSMatrix4f(Radius, Radius, Radius, 1.0f) * LWSMatrix4f::Translation(Ctr);
	return WriteDebugGeometry(PassBits, m_DebugGeometry->GetSpherePrimitive(), Transform, Color, Ctr, Radius, Flags);
}

bool LWERenderFrame::WriteDebugCone(uint32_t PassBits, const LWSVector4f &Pos, const LWSVector4f &Dir, float Theta, float Length, const LWVector4f &Color, uint32_t Flags) {
	assert(m_DebugGeometry != nullptr);
	const LWSVector4f Forward = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f);
	const uint32_t ConeRadiCnt = 30;
	float T = tanf(Theta) * Length;
	LWSMatrix4f Transform;
	float d = Forward.Dot3(Dir);
	if (d < -1.0f + std::numeric_limits<float>::epsilon()) Transform = LWSMatrix4f(1.0f, 1.0f, -1.0f, 1.0f); //Flip 180Degrees.
	else if (d < 1.0f - std::numeric_limits<float>::epsilon()) {
		LWSVector4f Up = Forward.Cross3(Dir).Normalize3();
		LWSVector4f Rt = Up.Cross3(Dir).Normalize3();
		Up = Dir.Cross3(Rt);
		Transform = LWSMatrix4f(Rt, Up, Dir, LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f));
	}
	Transform = LWSMatrix4f(T, T, Length, 1.0f) * Transform * LWSMatrix4f::Translation(Pos);
	return WriteDebugGeometry(PassBits, m_DebugGeometry->GetConePrimtive(), Transform, Color, Pos, Length, Flags);
}

bool LWERenderFrame::WriteDebugCube(uint32_t PassBits, const LWSVector4f &Pos, const LWSVector4f &Size, const LWVector4f &Color, uint32_t Flags) {
	assert(m_DebugGeometry != nullptr);
	LWSVector4f hSize = (Size * 0.5f).AAAB(LWSVector4f());
	LWSMatrix4f Transform = LWSMatrix4f(hSize.x, hSize.y, hSize.z, 1.0f) * LWSMatrix4f::Translation(Pos);
	return WriteDebugGeometry(PassBits, m_DebugGeometry->GetBoxPrimitive(), Transform, Color, Pos - hSize, Pos + hSize, Flags);
}

bool LWERenderFrame::WriteDebugAABB(uint32_t PassBits, const LWSMatrix4f &Transform, const LWSVector4f &AAMin, const LWSVector4f &AAMax, float LineThickness, const LWVector4f &Color, uint32_t Flags){
	LWSVector4f Am = AAMin * Transform;
	LWSVector4f Bm = AAMin.BAAA(AAMax) * Transform;
	LWSVector4f Cm = AAMin.ABAA(AAMax) * Transform;
	LWSVector4f Dm = AAMin.BBAA(AAMax) * Transform;

	LWSVector4f Ax = AAMax * Transform;
	LWSVector4f Bx = AAMax.BAAA(AAMin) * Transform;
	LWSVector4f Cx = AAMax.ABAA(AAMin) * Transform;
	LWSVector4f Dx = AAMax.BBAA(AAMin) * Transform;
	bool Drawn = false;
	Drawn = WriteDebugLine(PassBits, Am, Bm, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Am, Cm, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Bm, Dm, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Cm, Dm, LineThickness, Color, Flags) || Drawn;

	Drawn = WriteDebugLine(PassBits, Ax, Bx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Ax, Cx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Bx, Dx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Cx, Dx, LineThickness, Color, Flags) || Drawn;

	Drawn = WriteDebugLine(PassBits, Am, Dx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Bm, Cx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Cm, Bx, LineThickness, Color, Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Dm, Ax, LineThickness, Color, Flags) || Drawn;
	return Drawn;
}

bool LWERenderFrame::WriteDebugAxis(uint32_t PassBits, const LWSMatrix4f &Transform, float LineLen, float LineThickness, bool IgnoreScale, uint32_t Flags) {
	LWSVector4f xAxis = LWSVector4f(1.0f, 0.0f, 0.0f, 0.0f) * Transform;
	LWSVector4f yAxis = LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f) * Transform;
	LWSVector4f zAxis = LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f) * Transform;
	if (IgnoreScale) {
		xAxis = xAxis.Normalize3();
		yAxis = yAxis.Normalize3();
		zAxis = zAxis.Normalize3();
	}
	xAxis *= LineLen;
	yAxis *= LineLen;
	zAxis *= LineLen;
	LWSVector4f Pos = Transform[3];
	bool Drawn = false;
	Drawn = WriteDebugLine(PassBits, Pos, Pos + xAxis, LineThickness, LWVector4f(1.0f, 0.0f, 0.0f, 1.0f), Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Pos, Pos - xAxis, LineThickness, LWVector4f(0.5f, 0.0f, 0.0f, 0.5f), Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Pos, Pos + yAxis, LineThickness, LWVector4f(0.0f, 1.0f, 0.0f, 1.0f), Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Pos, Pos - yAxis, LineThickness, LWVector4f(0.0f, 0.5f, 0.0f, 0.5f), Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Pos, Pos + zAxis, LineThickness, LWVector4f(0.0f, 0.0f, 1.0f, 1.0f), Flags) || Drawn;
	Drawn = WriteDebugLine(PassBits, Pos, Pos - zAxis, LineThickness, LWVector4f(0.0f, 0.0f, 0.5f, 0.5f), Flags) || Drawn;
	return Drawn;
}

bool LWERenderFrame::WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, const LWSMatrix4f &Transform, const LWVector4f &Color, const LWSVector4f &Position, float Radius, uint32_t Flags){
	LWEGeometryModelData ModelData = LWEGeometryModelData(Transform, 0);
	ModelData.m_MaterialData[0] = Color;
	return PushModel(LWEGeometryModel(LWEGeometryModelBlock(m_DebugGeometry, GeometryID), LWERenderMaterial("Debug", nullptr, 0), Flags | (Color.w < 1.0f ? LWEGeometryModel::Transparent : 0)), ModelData, PassBits, Position, Radius) != -1;
}

bool LWERenderFrame::WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, const LWSMatrix4f &Transform, const LWVector4f &Color, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds, uint32_t Flags) {
	LWEGeometryModelData ModelData = LWEGeometryModelData(Transform, 0);
	ModelData.m_MaterialData[0] = Color;
	return PushModel(LWEGeometryModel(LWEGeometryModelBlock(m_DebugGeometry, GeometryID), LWERenderMaterial("Debug", nullptr, 0), Flags | (Color.w < 1.0f ? LWEGeometryModel::Transparent : 0)), ModelData, PassBits, MinBounds, MaxBounds) != -1;
}

uint32_t LWERenderFrame::WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, LWERenderMaterial &GeometryMaterial, LWEGeometryModelData &GeometryData, const LWSVector4f &Position, float Radius, uint32_t Flags) {
	return PushModel(LWEGeometryModel(LWEGeometryModelBlock(m_DebugGeometry, GeometryID), GeometryMaterial, Flags), GeometryData, PassBits, Position, Radius);
}

uint32_t LWERenderFrame::WriteDebugGeometry(uint32_t PassBits, uint64_t GeometryID, LWERenderMaterial &GeometryMaterial, LWEGeometryModelData &GeometryData, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds, uint32_t Flags) {
	return PushModel(LWEGeometryModel(LWEGeometryModelBlock(m_DebugGeometry, GeometryID), GeometryMaterial, Flags), GeometryData, PassBits, MinBounds, MaxBounds);
}

bool LWERenderFrame::PushMesh(const LWEMesh &Mesh, uint32_t PassBits, const LWSMatrix4f &Transform, uint32_t BoneID, const LWERenderMaterial *PrimitiveMaterialList, LWEGeometryModelData *PrimitiveDataList, uint32_t Flags) {
	uint32_t PrimCount = Mesh.GetPrimitiveCount();
	uint32_t BlockName = Mesh.GetBlockGeometry();
	uint64_t BlockID = Mesh.GetID();
	LWSVector4f AAMin, AAMax;
	Mesh.TransformBounds(Transform, AAMin, AAMax);
	bool Drawn = false;
	for (uint32_t i = 0; i < PrimCount; i++) {
		const LWEPrimitive &P = Mesh.GetPrimitive(i);
		PrimitiveDataList[i].m_Transform = Transform;
		PrimitiveDataList[i].m_BoneID = BoneID;
		Drawn = PushModel(LWEGeometryModel(LWEGeometryModelBlock(BlockName, BlockID, P.m_Offset, P.m_Count), PrimitiveMaterialList[i], Flags), PrimitiveDataList[i], PassBits, AAMin, AAMax)!=-1 || Drawn;
	}
	return Drawn;
}

bool LWERenderFrame::PushMeshPrimitive(const LWEMesh &Mesh, uint32_t Primitive, uint32_t PassBits, const LWSMatrix4f &Transform, uint32_t BoneID, const LWERenderMaterial &PrimitiveMaterial, LWEGeometryModelData &PrimitiveData, uint32_t Flags) {
	assert(Primitive < Mesh.GetPrimitiveCount());
	LWSVector4f AAMin, AAMax;
	Mesh.TransformBounds(Transform, AAMin, AAMax);
	const LWEPrimitive &P = Mesh.GetPrimitive(Primitive);
	PrimitiveData.m_Transform = Transform;
	PrimitiveData.m_BoneID = BoneID;
	return PushModel(LWEGeometryModel(LWEGeometryModelBlock(Mesh.GetBlockGeometry(), Mesh.GetID(), P.m_Offset, P.m_Count), PrimitiveMaterial, Flags), PrimitiveData, PassBits, AAMin, AAMax) != -1;
}

uint32_t LWERenderFrame::PushModel(const LWEGeometryModel &Model, const LWEGeometryModelData &Data, uint32_t PassBits, const LWSVector4f &Position, float Radius) {
	if (!PassBits) return -1;
	uint32_t Index = -1;
	uint32_t BlockHash = LWUTF8I::EmptyHash;
	uint32_t MaterialHash = LWUTF8I::EmptyHash;
	for (uint32_t i = 0; i < m_PassBucketCount; ++i) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		if ((Bucket.m_PassBit & PassBits) == 0) continue;
		if (!Bucket.SphereInFrustum(Position, Radius)) continue;
		if (Index == -1) {
			Index = m_ModelCount++;
			if (Index >= LWEMaxBucketSize) {
				if (Index == LWEMaxBucketSize) LWLogCritical("Max model's reached for frame.");
				return -1;
			}
			m_ModelList[Index] = Model;
			m_ModelData[Index] = Data;
			MaterialHash = m_ModelList[Index].m_Material.Hash();
			BlockHash = m_ModelList[Index].m_GeometryBlock.Hash();
		}

		Bucket.PushModel(Index, BlockHash, MaterialHash, m_ModelList[Index], Position);
	}
	return Index;
}

uint32_t LWERenderFrame::PushModel(const LWEGeometryModel &Model, const LWEGeometryModelData &Data, uint32_t PassBits, const LWSVector4f &MinBounds, const LWSVector4f &MaxBounds) {
	if (!PassBits) return -1;
	uint32_t Index = -1;
	uint32_t BlockHash = LWUTF8I::EmptyHash;
	uint32_t MaterialHash = LWUTF8I::EmptyHash;
	LWSVector4f Ctr = MinBounds + (MaxBounds - MinBounds) * 0.5f;
	for (uint32_t i = 0; i < m_PassBucketCount; ++i) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		if ((Bucket.m_PassBit & PassBits) == 0) continue;
		if (!Bucket.AABBInFrustum(MinBounds, MaxBounds)) continue;
		if (Index == -1) {
			Index = m_ModelCount++;
			if (Index >= LWEMaxBucketSize) {
				if (Index == LWEMaxBucketSize) LWLogCritical("Max model's reached for frame.");
				return -1;
			}
			m_ModelList[Index] = Model;
			m_ModelData[Index] = Data;
			MaterialHash = m_ModelList[Index].m_Material.Hash();
			BlockHash = m_ModelList[Index].m_GeometryBlock.Hash();
		}
		Bucket.PushModel(Index, BlockHash, MaterialHash, m_ModelList[Index], Ctr);
	}
	return Index;
}

bool LWERenderFrame::PushLight(const LWEShaderLightData &Light, bool bIsShadowCaster) {
	LWEGeometryBucket &PrimaryBucket = m_GeometryBuckets[m_PrimaryBucket];
	float DistanceSq = 0.0f;
	if (Light.m_Position.w == 1.0f) {
		if (!PrimaryBucket.SphereInFrustum(Light.m_Position.w, Light.m_Direction.x + Light.m_Direction.y)) return false;
		DistanceSq = (PrimaryBucket.m_ViewTransform[3] - Light.m_Position).LengthSquared3();
	} else if (Light.m_Position.w > 1.0f) {
		if (!PrimaryBucket.ConeInFrustum(Light.m_Position.xyz1(), Light.m_Direction.xyz0(), Light.m_Position.w - 1.0f, Light.m_Direction.w)) return false;
		DistanceSq = (PrimaryBucket.m_ViewTransform[3] - Light.m_Position).LengthSquared3();
	} else if (Light.m_Position.w < 0.0f) bIsShadowCaster = false;
	uint32_t LightIdx = m_LightCount++;
	if (LightIdx >= LWEMaxLights) {
		if (LightIdx == LWEMaxLights) LWLogCritical("Max lights reached for frame.");
		return false;
	}
	m_LightData[LightIdx] = Light;
	if (bIsShadowCaster) {
		uint32_t ShadowIdx = m_ShadowCount++;
		m_ShadowLightList[ShadowIdx] = LWEShadowBucketItem(LightIdx, DistanceSq);
	}
	return true;
}

LWERenderFrame &LWERenderFrame::Initialize(uint32_t FrameID) {
	m_FrameID = FrameID;
	for (uint32_t i = 0; i < m_PassBucketCount; i++) m_GeometryBuckets[i].Reset();
	m_BoneCount = m_ModelCount = m_LightCount = m_ShadowCount = 0;
	return *this;
}

LWERenderFrame &LWERenderFrame::Finalize(LWERenderer *Renderer, float Time, LWVideoBuffer *GlobalDataBuffer, LWVideoBuffer *ModelDataBuffer, LWVideoBuffer *BoneDataBuffer, LWVideoBuffer *LightDataBuffer, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWEGeometryRenderable RenderList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderCount[LWEMaxGeometryBuckets]) {
	LWVideoDriver *Driver = Renderer->GetVideoDriver();
	m_GlobalData.m_WindowSize = Driver->GetWindow()->GetSizef();
#ifdef LWETHREADEDRENDERFRAME
	uint32_t ModelCount = std::min<uint32_t>(m_ModelCount.load(), LWEMaxBucketSize);
	uint32_t BoneCount = std::min<uint32_t>(m_BoneCount.load(), LWEMaxBoneCount);
	uint32_t LightCount = std::min<uint32_t>(m_LightCount.load(), LWEMaxLights);
#else
	uint32_t ModelCount = std::min<uint32_t>(m_ModelCount, LWEMaxBucketSize);
	uint32_t BoneCount = std::min<uint32_t>(m_BoneCount, LWEMaxBoneCount);
	uint32_t LightCount = std::min<uint32_t>(m_LightCount, LWEMaxLights);
#endif
	m_GlobalData.m_LightCount = LightCount;
	m_GlobalData.m_Time = Time;
	m_GlobalData.m_SinTime = sinf(Time);
	Driver->UpdateVideoBuffer<LWEShaderGlobalData>(GlobalDataBuffer, &m_GlobalData, 1);
	Driver->UpdateVideoBuffer<LWEGeometryModelData>(ModelDataBuffer, m_ModelData, ModelCount);
	Driver->UpdateVideoBuffer<LWEShaderLightData>(LightDataBuffer, m_LightData, LightCount);
	Driver->UpdateVideoBuffer<LWSMatrix4f>(BoneDataBuffer, m_BoneData, BoneCount);
	for (uint32_t i = 0; i < m_PassBucketCount; ++i) {
		LWEGeometryBucket &Bucket = m_GeometryBuckets[i];
		RenderCount[i] = Bucket.Finalize(Renderer, *this, IndirectBufferList[i], IDBufferList[i], RenderList[i]);
	}
	return *this;
}

uint32_t LWERenderFrame::PushFrameData(LWEPassFrameData *Data) {
	m_PassList.push_back(Data);
	return (uint32_t)m_PassList.size() - 1;
}

uint32_t LWERenderFrame::NextBoneID(uint32_t BoneCount) {
	if (!BoneCount) return 0;
#ifdef LWETHREADEDRENDERFRAME
	uint32_t ID = m_BoneCount.fetch_add(BoneCount);
#else
	uint32_t ID = m_BoneCount;
	m_BoneCount += BoneCount;
#endif
	if (ID >= LWEMaxBoneCount) {
		if (ID == LWEMaxBoneCount) LWLogCritical("Reached max bone's in frame.");
		return 0; //0 so the program won't crash right away, i guess.
	}
	return ID;
}

LWSMatrix4f *LWERenderFrame::GetBoneDataAt(uint32_t ID) {
	return m_BoneData + ID;
}

LWEPassFrameData *LWERenderFrame::GetFrameData(uint32_t ID) {
	return m_PassList[ID];
}

uint32_t LWERenderFrame::PassCreateGeometryBuckets(uint32_t BucketCount) {
	assert(m_PassBucketCount + BucketCount < LWEMaxGeometryBuckets);
	m_PassBucketCount += BucketCount;
	return m_PassBucketCount - BucketCount;
}

LWERenderFrame &LWERenderFrame::SetPrimaryBucket(uint32_t PrimaryBucket) {
	m_PrimaryBucket = PrimaryBucket;
	return *this;
}

LWERenderFrame &LWERenderFrame::SetDebugGeometry(LWERendererBlockGeometry *DebugGeometry) {
	m_DebugGeometry = DebugGeometry;
	return *this;
}

LWEGeometryBucket &LWERenderFrame::GetGeometryBucket(uint32_t ID) {
	return m_GeometryBuckets[ID];
}

LWEGeometryBucket &LWERenderFrame::GetPrimaryBucket(void) {
	return m_GeometryBuckets[m_PrimaryBucket];
}

LWEShaderGlobalData &LWERenderFrame::GetGlobalData(void) {
	return m_GlobalData;
}

const LWEGeometryModel &LWERenderFrame::GetModel(uint32_t Index) const {
	return m_ModelList[Index];
}

LWEGeometryModel &LWERenderFrame::GetModel(uint32_t Index) {
	return m_ModelList[Index];
}

LWEShadowBucketItem *LWERenderFrame::GetShadowList(void) {
	return m_ShadowLightList;
}

const LWEShaderLightData &LWERenderFrame::GetLight(uint32_t Index) const {
	return m_LightData[Index];
}

LWEShaderLightData &LWERenderFrame::GetLight(uint32_t Index) {
	return m_LightData[Index];
}

uint32_t LWERenderFrame::GetPassBucketCount(void) const {
	return m_PassBucketCount;
}

uint32_t LWERenderFrame::GetShadowCount(void) const {
	return m_ShadowCount;
}

uint32_t LWERenderFrame::GetLightCount(void) const {
	return m_LightCount;
}

uint32_t LWERenderFrame::GetPrimaryBucketID(void) const {
	return m_PrimaryBucket;
}

LWERendererBlockGeometry *LWERenderFrame::GetDebugGeometry(void) {
	return m_DebugGeometry;
}

LWERenderFrame::LWERenderFrame(LWVideoDriver *Driver, uint32_t PassCount, LWERendererBlockGeometry *DebugGeometry, LWAllocator &Allocator) : m_DebugGeometry(DebugGeometry) {
	m_PassList.reserve(PassCount);
}