#include <LWERenderPasses/LWEUIPass.h>
#include <LWERenderer.h>
#include <LWCore/LWLogger.h>
#include <LWPlatform/LWWindow.h>
#include <limits>
#include <cmath>

//LWEUIPass:
LWEPass *LWEUIPass::ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	LWEUIPass *UIPass = Pass ? (LWEUIPass*)Pass : Allocator.Create<LWEUIPass>();
	if(!LWLogCriticalIf<256>(LWEPass::ParseXML(Node, UIPass, Renderer, AssetManager, Allocator), "Could not create pass: '{}'", Node->GetName())) {
		if (!Pass) LWAllocator::Destroy(UIPass);
		return nullptr;
	}

	auto ParseUIShaderNode = [&UIPass, &AssetManager](LWEXMLNode *CNode) {
		LWEXMLAttribute *ColorAttr = CNode->FindAttribute("Color");
		LWEXMLAttribute *TextureAttr = CNode->FindAttribute("Texture");
		LWEXMLAttribute *FontAttr = CNode->FindAttribute("Font");
		if (ColorAttr) UIPass->SetColorShader(AssetManager->GetAsset<LWShader>(ColorAttr->GetValue()));
		if (TextureAttr) UIPass->SetTextureShader(AssetManager->GetAsset<LWShader>(TextureAttr->GetValue()));
		if (FontAttr) UIPass->SetFontShader(AssetManager->GetAsset<LWShader>(FontAttr->GetValue()));
	};

	for (LWEXMLNode *C = Node->m_FirstChild; C; C = C->m_Next) {
		uint32_t NodeID = C->GetName().CompareList("UIShaders");
		if (NodeID == 0) ParseUIShaderNode(C);
		else ParseXMLChild(C, UIPass, Renderer, AssetManager, Allocator);
	}
	return UIPass;
}

uint32_t LWEUIPass::RenderPass(LWERenderFrame & Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t PassIndex) {
	PreparePass(Driver, Renderer, m_Propertys);
	LWPipeline *Pipeline = PreparePipeline(LWERenderMaterial(), LWUTF8I::EmptyHash, Driver, Renderer, PassIndex);
	auto UIFrame = Frame.GetFrameData<LWEUIPassFrameData>(m_FrameID);
	LWEUIFrame &UIF = UIFrame->m_UIFrame;
	uint32_t o = 0;
	for (uint32_t i = 0; i < UIF.m_TextureCount; i++) {
		Pipeline->SetResource(0, UIF.m_Textures[i]);
		Pipeline->SetPixelShader(UIF.m_FontTexture[i] ? m_FontShader : (UIF.m_Textures[i] ? m_TextureShader : m_ColorShader));
		Driver->DrawMesh(Pipeline, LWVideoDriver::Triangle, UIF.m_Mesh, UIF.m_VertexCount[i], o);
		o += UIF.m_VertexCount[i];
	}
	return 1;
}

LWEPass &LWEUIPass::WindowSizeChanged(LWVideoDriver *Driver, LWERenderer *Renderer, LWWindow *Window, LWAllocator &Allocator) {
	LWVector2f WndSize = Window->GetSizef();
	LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, WndSize.x, 0.0f, WndSize.y, 0.0f, 1.0f);
	Driver->UpdateVideoBuffer(m_UIBuffer, (const uint8_t*)&Ortho, sizeof(LWMatrix4f));
	return *this;
}

uint32_t LWEUIPass::InitializePass(LWVideoDriver *Driver, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWEShaderPassData *PassData, LWAllocator &Allocator) {
	LWEPassPipelinePropertys *Props = FindPipeline(LWUTF8I::EmptyHash);
	LWEPassResource &UIUniformBlock = Props->m_BlockList[0];
	m_UIBuffer = (LWVideoBuffer *)UIUniformBlock.m_Resource;
	if (!m_UIBuffer) m_UIBuffer = Renderer->GetVideoBuffer(UIUniformBlock.m_ResourceID);
	PassData[0] = LWEShaderPassData(m_PassID);
	return 1;
}

void LWEUIPass::DestroyPass(LWVideoDriver *Driver, bool DestroySelf) {
	if (DestroySelf) LWAllocator::Destroy(this);
	return;
}

LWEPass &LWEUIPass::InitializeFrame(LWERenderFrame &Frame) {
	auto UIFrame = Frame.GetFrameData<LWEUIPassFrameData>(m_FrameID);
	Frame.GetGlobalData().m_PassData[m_PassID].m_PassData[0] = LWVector4f(std::numeric_limits<float>::quiet_NaN());
	UIFrame->m_UIFrame.m_TextureCount = 0;
	return *this;
}

LWEPass &LWEUIPass::PreFinalizeFrame(LWERenderFrame &Frame, LWVideoBuffer *IndirectBufferList[LWEMaxGeometryBuckets], LWVideoBuffer *IDBufferList[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver) {
	auto &Global = Frame.GetGlobalData();
	FinalizePassGlobalData(Global, m_Propertys, Renderer, Driver, m_PassID);
	auto UIFrame = Frame.GetFrameData<LWEUIPassFrameData>(m_FrameID);
	UIFrame->m_UIFrame.m_Mesh->Finished();
	LWVector4f Viewport = Global.m_PassData[m_PassID].m_PassData[0];
	if (std::isnan(Viewport.x)) return *this;
	LWMatrix4f Ortho = LWMatrix4f::Ortho(Viewport.x, Viewport.x + Viewport.z, Viewport.y, Viewport.y + Viewport.w, 0.0f, 1.0f);
	Driver->UpdateVideoBuffer(m_UIBuffer, (const uint8_t*)&Ortho, sizeof(LWMatrix4f));

	return *this;
}

LWEPass &LWEUIPass::PostFinalizeFrame(LWERenderFrame &Frame, LWERenderer *Renderer, LWVideoDriver *Driver) {
	return *this;
}

LWEPass &LWEUIPass::CreateFrame(LWERenderFrame &Frame, LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator) {
	auto UIFrame = Allocator.Create<LWEUIPassFrameData>();
	LWVideoBuffer *UIBuffer = Driver->CreateVideoBuffer<LWVertexUI>(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, MaxUIItems * 6, Allocator, nullptr);
	UIFrame->m_UIFrame.m_Mesh = LWVertexUI::MakeMesh(Allocator, UIBuffer, 0);
	m_FrameID = Frame.PushFrameData(UIFrame);
	return *this;
}

LWEPass &LWEUIPass::ReleaseFrame(LWERenderFrame &Frame, LWVideoDriver *Driver) {
	auto UIFrame = Frame.GetFrameData<LWEUIPassFrameData>(m_FrameID);
	UIFrame->m_UIFrame.m_Mesh->Destroy(Driver);
	LWAllocator::Destroy(UIFrame);
	return *this;
}

LWEUIPass &LWEUIPass::SetColorShader(LWShader *Shader) {
	m_ColorShader = Shader;
	return *this;
}

LWEUIPass &LWEUIPass::SetTextureShader(LWShader *Shader) {
	m_TextureShader = Shader;
	return *this;
}

LWEUIPass &LWEUIPass::SetFontShader(LWShader *Shader) {
	m_FontShader = Shader;
	return *this;
}

LWEUIFrame &LWEUIPass::GetUIFrame(LWERenderFrame &Frame) {
	auto UIFrame = Frame.GetFrameData<LWEUIPassFrameData>(m_FrameID);
	return UIFrame->m_UIFrame;
}

const LWShader *LWEUIPass::GetColorShader(void) const {
	return m_ColorShader;
}

const LWShader *LWEUIPass::GetTextureShader(void) const {
	return m_TextureShader;
}

const LWShader *LWEUIPass::GetFontShader(void) const {
	return m_FontShader;
}