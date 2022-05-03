#include <LWERenderPasses/LWEPPPass.h>
#include <LWERenderer.h>
#include <LWCore/LWLogger.h>

//LWEPassPostProcess
LWEPass *LWEPPPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEPPPass *PPPass = Pass ? (LWEPPPass*)Pass : Allocator.Create<LWEPPPass>();
	
	if(!LWLogCriticalIf<256>(LWEPass::ParseXML(Node, PPPass, Renderer, AssetManager, Allocator), "Could not create pass '{}'", Node->GetName())) {
		if (!Pass) LWAllocator::Destroy(PPPass);
		return nullptr;
	}
	uint32_t SubPassCount = 0;
	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t NodeID = C->GetName().CompareList("SubPass");
		if (NodeID == 0) {
			LWEPassPropertys Props;
			if (!LWEPass::ParseXMLPassPropertys(C, Props, Renderer)) continue;
			LWEXMLAttribute *PipelineAttr = C->FindAttribute("Pipeline");
			PPPass->PushSubPass(PipelineAttr ? PipelineAttr->GetValue().Hash() : LWUTF8I::EmptyHash, Props);
			SubPassCount++;
		} else ParseXMLChild(C, PPPass, Renderer, AssetManager, Allocator);
	}
	if (!SubPassCount) PPPass->PushSubPass(LWUTF8I::EmptyHash, PPPass->GetPropertys());
	return PPPass;
}

uint32_t LWEPPPass::RenderPass(LWERenderFrame & Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer * Renderer, LWVideoDriver * Driver, uint32_t PassIndex) {
	for (uint32_t i = 0; i < m_SubPassCount; i++) {
		PreparePass(Driver, Renderer, m_SubPassList[i]);
		LWPipeline *Pipeline = PreparePipeline(LWERenderMaterial(m_SubPassPipeline[i], nullptr, 0), LWUTF8I::EmptyHash, Driver, Renderer, PassIndex + i);
		Driver->DrawBuffer(Pipeline, LWVideoDriver::Triangle, m_ScreenPPVertices, nullptr, 6, sizeof(LWVertexTexture));
	}
	return m_SubPassCount;
}

LWEPass &LWEPPPass::WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator) {
	return *this;
}

uint32_t LWEPPPass::InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator) {
	m_ScreenPPVertices = Renderer->GetPPScreenVertices();
	for (uint32_t i = 0; i < m_SubPassCount; i++) PassData[i] = LWEShaderPassData(m_PassID, i);
	return m_SubPassCount;
}

void LWEPPPass::DestroyPass(LWVideoDriver *Driver, bool DestroySelf) {
	if (DestroySelf) LWAllocator::Destroy(this);
	return;
}

LWEPass &LWEPPPass::InitializeFrame(LWERenderFrame &Frame) {
	return *this;
}

LWEPass &LWEPPPass::PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) {
	LWEShaderGlobalData &GlobalData = Frame.GetGlobalData();
	for (uint32_t i = 0; i < m_SubPassCount; i++) FinalizePassGlobalData(GlobalData, m_SubPassList[i], Renderer, Driver, m_PassID, i);
	return *this;
}

LWEPass &LWEPPPass::PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver) {
	return *this;
}

LWEPass &LWEPPPass::CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	return *this;
}

LWEPass &LWEPPPass::ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver) {
	return *this;
}

bool LWEPPPass::PushSubPass(uint32_t PipelineName, const LWEPassPropertys &SubPass) {
	if(!LWLogCriticalIf(m_SubPassCount<MaxSubPasses, "PP Pass has exceeded max subpass's.")) return false;
	m_SubPassPipeline[m_SubPassCount] = PipelineName;
	m_SubPassList[m_SubPassCount] = SubPass;
	m_SubPassCount++;
	return true;
}