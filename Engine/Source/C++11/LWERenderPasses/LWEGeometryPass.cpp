#include "LWERenderPasses/LWEGeometryPass.h"
#include "LWERenderer.h"
#include <LWCore/LWLogger.h>

//LWEBucketPropertys:
LWBitField32Define(LWEBucketPropertys::PassBitIDBits);
const uint32_t LWEBucketPropertys::PrimarySource;

bool LWEBucketPropertys::isPrimarySource(void) const {
	return (m_Flags & PrimarySource) != 0;
}

uint32_t LWEBucketPropertys::GetPassBitID(void) const {
	return LWBitFieldGet(PassBitIDBits, m_Flags);
}

LWEBucketPropertys::LWEBucketPropertys(const LWEPassPropertys &PassProps, uint32_t BucketFlags, uint32_t PassBitID) : m_PassPropertys(PassProps), m_BucketFlags(BucketFlags), m_Flags((PassBitID << PassBitIDBitsOffset)) {}

//LWEGeometryPass
bool LWEGeometryPass::ParseXMLBucketPropertys(LWEXMLNode *Node, LWEBucketPropertys &BucketProps, LWERenderer *Renderer) {
	if (!ParseXMLPassPropertys(Node, BucketProps.m_PassPropertys, Renderer)) return false;
	LWEXMLAttribute *OpaqueSortAttr = Node->FindAttribute("OpaqueSort");
	LWEXMLAttribute *TransparentSortAttr = Node->FindAttribute("TransparentSort");
	LWEXMLAttribute *PrimaryAttr = Node->FindAttribute("Primary");
	LWEXMLAttribute *PassBitIDAttr = Node->FindAttribute("PassBitID");
	if (OpaqueSortAttr) {
		uint32_t Sort = OpaqueSortAttr->GetValue().CompareList("ByStates", "FrontToBack", "BackToFront", "None");
		if(LWLogCriticalIf<128>(Sort!=-1, "Unknown sort for opaque: '{}'", OpaqueSortAttr->GetValue()))	{
			BucketProps.m_BucketFlags = LWBitFieldSet(LWEGeometryBucket::OpaqueSortBits, BucketProps.m_BucketFlags, Sort);
		}
	}
	if (TransparentSortAttr) {
		uint32_t Sort = TransparentSortAttr->GetValue().CompareList("ByStates", "FrontToBack", "BackToFront", "None");
		if(LWLogCriticalIf<128>(Sort!=-1, "Unknown sort for Transparent: '{}'", TransparentSortAttr->GetValue())) {
			BucketProps.m_BucketFlags = LWBitFieldSet(LWEGeometryBucket::TransparentSortBits, BucketProps.m_BucketFlags, Sort);
		}
	}
	if (PrimaryAttr) BucketProps.m_Flags |= LWEBucketPropertys::PrimarySource;
	if (PassBitIDAttr) BucketProps.m_Flags = LWBitFieldSet(LWEBucketPropertys::PassBitIDBits, BucketProps.m_Flags, atoi(PassBitIDAttr->GetValue().c_str()));
	return true;
}

LWEPass *LWEGeometryPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEGeometryPass *GPass = Pass ? (LWEGeometryPass*)Pass : Allocator.Create<LWEGeometryPass>();
	if(!LWLogCriticalIf<256>(LWEPass::ParseXML(Node, GPass, Renderer, AssetManager, Allocator), "Could not create pass '{}'", Node->GetName())) {
		if (!Pass) LWAllocator::Destroy(GPass);
		return nullptr;
	}
	LWEXMLAttribute *PassBitIDAttr = Node->FindAttribute("PassBitID");
	uint32_t DefaultBitID = PassBitIDAttr ? atoi(PassBitIDAttr->GetValue().c_str()) : 0;
	uint32_t BucketCount = 0;
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t cID = C->GetName().CompareList("Bucket");
		LWEBucketPropertys BucketProps = LWEBucketPropertys(GPass->GetPropertys(), (LWEGeometryBucket::SortByStates << LWEGeometryBucket::OpaqueSortBitsOffset) | (LWEGeometryBucket::SortBackToFront << LWEGeometryBucket::TransparentSortBitsOffset), DefaultBitID);
		if (cID == 0) {
			if (ParseXMLBucketPropertys(C, BucketProps, Renderer)) GPass->SetGeometryBucketPropertys(BucketCount++, BucketProps);
		} else ParseXMLChild(C, GPass, Renderer, AssetManager, Allocator);
	}

	GPass->SetGeometryBucketCount(BucketCount);
	return GPass;
}

LWPipeline *LWEGeometryPass::PrepareRendablePipeline(const LWEGeometryRenderable &Rendable, const LWERenderMaterial &Material, LWVideoBuffer *&IndiceBuffer, uint32_t &RenderCount, LWPipelineInputStream InputStream[LWShader::MaxInputs], LWVideoDriver *Driver, LWERenderer *Renderer, uint32_t SubPassIdx, uint32_t SubPassOffset) {
	LWPipeline *Pipeline = PreparePipeline(Material, (uint32_t)Rendable.m_BlockBufferNameHash, Driver, Renderer, SubPassIdx+SubPassOffset);;
	assert(Pipeline != nullptr);
	if (Rendable.m_DrawCount & LWEGeometryRenderable::BufferVideoBuffer) {
		LWVideoBuffer *VPBuffer = Renderer->GetVideoBuffer((uint32_t)LWBitFieldGet(LWEGeometryModelBlock::VertexPositionVBBits, Rendable.m_BlockBufferNameHash));
		LWVideoBuffer *VABuffer = Renderer->GetVideoBuffer((uint32_t)LWBitFieldGet(LWEGeometryModelBlock::VertexAttributeVBBits, Rendable.m_BlockBufferNameHash));
		IndiceBuffer = Renderer->GetVideoBuffer((uint32_t)LWBitFieldGet(LWEGeometryModelBlock::IndiceVBBits, Rendable.m_BlockBufferNameHash));
		if (!VPBuffer || !VABuffer || !IndiceBuffer) return Pipeline;
		uint32_t InputCnt = Pipeline->GetInputCount();
		uint32_t VPSize = VPBuffer->GetTypeSize();
		uint32_t VASize = VABuffer->GetTypeSize();
		for (uint32_t i = 0; i < InputCnt; i++) {
			LWShaderInput &In = Pipeline->GetInput(i);
			if (In.GetInstanceFrequency() == 0) {
				uint32_t Offset = In.GetOffset();
				if (Offset >= VPSize) InputStream[i] = LWPipelineInputStream(VABuffer, Offset - VPSize, VASize);
				else InputStream[i] = LWPipelineInputStream(VPBuffer, Offset, VPSize);
			} else InputStream[i] = LWPipelineInputStream(m_IDBuffer[SubPassIdx], 0, sizeof(uint32_t));
		}
	} else {
		LWERendererBlockGeometry *Geometry = Renderer->FindNamedBlockGeometryMap((uint32_t)Rendable.m_BlockBufferNameHash);
		assert(Geometry != nullptr);
		Geometry->BuildInputStreams(Pipeline, InputStream, m_IDBuffer[SubPassIdx]);
		IndiceBuffer = Geometry->GetIndiceBuffer();
	}
	RenderCount = Rendable.m_DrawCount & ~LWEGeometryRenderable::BufferVideoBuffer;

	return Pipeline;
}

uint32_t LWEGeometryPass::RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex) {
	LWPipelineInputStream InputStreams[LWShader::MaxInputs];
	uint32_t p = 0;
	for (uint32_t i = m_GeometryPassIdx; i < m_GeometryPassIdx + m_GeometryBucketCount; ++i, ++p) {
		if(!Frame.GetGeometryBucket(i).isInitialized()) continue;
		PreparePass(Driver, Renderer, m_GeometryPropertys[p].m_PassPropertys);
		uint32_t RenderableOffset = 0;
		//Draw opaque:
		for (uint32_t o = 0; o < RenderableCount[i].first; o++) {
			LWEGeometryRenderable &Renderable = RenderableList[i][o];
			LWVideoBuffer *IndiceBuffer = nullptr;
			uint32_t Count = 0;
			LWPipeline *Pipeline = PrepareRendablePipeline(Renderable, Renderable.m_Material, IndiceBuffer, Count, InputStreams, Driver, Renderer, p, SubPassIndex);
			if(!Count) continue;
			Driver->DrawIndirectBuffer(Pipeline, LWVideoDriver::Triangle, InputStreams, IndiceBuffer, m_IndirectBuffer[p], Count, RenderableOffset);
			RenderableOffset += Count;
		}
		//Draw Transparent:
		for (uint32_t t = RenderableCount[i].first; t < RenderableCount[i].first + RenderableCount[i].second; t++) {
			LWEGeometryRenderable &Renderable = RenderableList[i][t];
			LWVideoBuffer *IndiceBuffer = nullptr;
			uint32_t Count = 0;
			LWPipeline *Pipeline = PrepareRendablePipeline(Renderable, Renderable.m_Material, IndiceBuffer, Count, InputStreams, Driver, Renderer, p, SubPassIndex);
			if (!Count) continue;
			Driver->DrawIndirectBuffer(Pipeline, LWVideoDriver::Triangle, InputStreams, IndiceBuffer, m_IndirectBuffer[p], Count, RenderableOffset);
			RenderableOffset += Count;

		}
	}
	return m_GeometryBucketCount;
}

uint32_t LWEGeometryPass::InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator) {
	for (uint32_t i = 0; i < m_GeometryBucketCount; i++) {
		m_IndirectBuffer[i] = Driver->CreateVideoBuffer<LWIndirectIndice>(LWVideoBuffer::Indirect, LWVideoBuffer::WriteDiscardable, LWEMaxBucketSize, Allocator, nullptr);
		m_IDBuffer[i] = Driver->CreateVideoBuffer<uint32_t>(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable, LWEMaxBucketSize, Allocator, nullptr);
		PassData[i] = LWEShaderPassData(m_PassID, i);
	}
	return m_GeometryBucketCount;
}

LWEPass &LWEGeometryPass::WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator) {
	return *this;
}

void LWEGeometryPass::DestroyPass(LWVideoDriver *Driver, bool DestroySelf) {
	for (uint32_t i = 0; i < m_GeometryBucketCount; i++) {
		Driver->DestroyVideoBuffer(m_IndirectBuffer[i]);
		Driver->DestroyVideoBuffer(m_IDBuffer[i]);
	}
	if (DestroySelf) LWAllocator::Destroy(this);
	return;
}

LWEPass &LWEGeometryPass::InitializeFrame(LWERenderFrame &Frame) {
	return *this;
}

LWEPass &LWEGeometryPass::PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) {
	LWEShaderGlobalData &GlobalData = Frame.GetGlobalData();
	GlobalData.m_PassData[m_PassID].m_GeometryBucketOffset = m_GeometryPassIdx;
	for (uint32_t i = 0; i < m_GeometryBucketCount; i++) {
		FinalizePassGlobalData(GlobalData, m_GeometryPropertys[i].m_PassPropertys, Renderer, Driver, m_PassID, i);
		IndirectBufferList[m_GeometryPassIdx + i] = m_IndirectBuffer[i];
		IDBufferList[m_GeometryPassIdx + i] = m_IDBuffer[i];
	}
	return *this;
}

LWEPass &LWEGeometryPass::PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver) {
	return *this;
}

LWEPass &LWEGeometryPass::CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	m_GeometryPassIdx = Frame.PassCreateGeometryBuckets(m_GeometryBucketCount);
	for (uint32_t i = 0; i < m_GeometryBucketCount; i++) {
		LWEGeometryBucket &Bucket = Frame.GetGeometryBucket(m_GeometryPassIdx + i);
		Bucket.m_Flag = m_GeometryPropertys[i].m_BucketFlags;
		Bucket.m_PassBit = (1 << m_GeometryPropertys[i].GetPassBitID());
		if (m_GeometryPropertys[i].isPrimarySource()) Frame.SetPrimaryBucket(m_GeometryPassIdx + i);
	}
	return *this;
}

LWEPass &LWEGeometryPass::ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver) {
	return *this;
}

LWEGeometryPass &LWEGeometryPass::SetGeometryBucketCount(uint32_t BucketCount) {
	m_GeometryBucketCount = BucketCount;
	return *this;
}

LWEGeometryPass &LWEGeometryPass::SetGeometryBucketPropertys(uint32_t BucketIdx, const LWEBucketPropertys &Props) {
	m_GeometryPropertys[BucketIdx] = Props;
	return *this;
}

uint32_t LWEGeometryPass::GetGeometryBucketCount(void) const {
	return m_GeometryBucketCount;
}

LWEGeometryPass::LWEGeometryPass(uint32_t BucketCount) : m_GeometryBucketCount(BucketCount) {
	assert(m_GeometryBucketCount < LWEMaxGeometryBuckets);
}