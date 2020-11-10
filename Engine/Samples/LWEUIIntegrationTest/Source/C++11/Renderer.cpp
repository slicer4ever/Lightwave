#include "Renderer.h"
#include "LWVideo/LWVideoDriver.h"
#include "LWPlatform/LWWindow.h"
#include <LWCore/LWMatrix.h>
#include <LWCore/LWVector.h>
#include <LWVideo/LWMesh.h>
#include <LWEAsset.h>

//GFrame
void GFrame::Initalize() {
	m_UIFrame.m_TextureCount = 0;
	return;
}

void GFrame::Release(LWVideoDriver *Driver) {
	m_UIFrame.m_Mesh->Destroy(Driver);
}

GFrame::GFrame(LWVideoDriver *Driver, LWAllocator &Allocator) {
	LWMesh<LWVertexUI> *UIMesh = LWVertexUI::MakeMesh(Allocator, Driver->CreateVideoBuffer<LWVertexUI>(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable|LWVideoBuffer::LocalCopy, MaxUIVertices, Allocator, nullptr), 0);
	m_UIFrame = LWEUIFrame(UIMesh);
}

//Renderer
GFrame *Renderer::BeginFrame(void) {
	if (m_WriteFrame - m_ReadFrame >= MaxFrames - 1) return nullptr;
	GFrame &F = m_Frames[m_WriteFrame % MaxFrames];
	F.Initalize();
	return &F;
}

void Renderer::EndFrame(void) {
	m_WriteFrame++;
	return;
}

void Renderer::SizeUpdate(LWWindow *Window) {
	if (!m_SizeChanged) return;
	LWVector2f WndSize = Window->GetSizef();
	LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, WndSize.x, 0.0f, WndSize.y, 0.0f, 1.0f);
	m_Driver->UpdateVideoBuffer(m_UIUniform, (uint8_t*)&Ortho, sizeof(LWMatrix4f));
	m_Driver->ViewPort();
	m_SizeChanged = false;
	return;
}

void Renderer::ApplyFrame(GFrame &F) {
	F.m_UIFrame.m_Mesh->Finished();
	return;
}

void Renderer::RenderUIFrame(LWEUIFrame &F) {
	uint32_t o = 0;
	for (uint32_t i = 0; i < F.m_TextureCount; i++) {
		LWShader *S = F.m_FontTexture[i] ? m_UIFontShader : (F.m_Textures[i] ? m_UITextureShader : m_UIColorShader);
		m_UIPipeline->SetResource(0, F.m_Textures[i]);
		m_UIPipeline->SetPixelShader(S);
		m_Driver->DrawMesh(m_UIPipeline, LWVideoDriver::Triangle, F.m_Mesh, F.m_VertexCount[i], o);
		o += F.m_VertexCount[i];
	}
	return;
}


void Renderer::RenderFrame(GFrame &F) {

	RenderUIFrame(F.m_UIFrame);
	return;
}

void Renderer::Render(LWWindow *Window) {
	m_SizeChanged = Window->SizeUpdated() || m_SizeChanged;
	if (!m_Driver->Update()) return;
	SizeUpdate(Window);
	if (m_WriteFrame != m_ReadFrame) {
		ApplyFrame(m_Frames[m_ReadFrame % MaxFrames]);
		m_ReadFrame++;
	}
	if (!m_ReadFrame) return;
	GFrame &F = m_Frames[(m_ReadFrame - 1) % MaxFrames];
	m_Driver->ClearColor(0xFF);
	RenderFrame(F);
	m_Driver->Present(1);
	return;
}

void Renderer::LoadAssets(LWEAssetManager *AM, LWAllocator &Allocator) {

	m_UIColorShader = AM->GetAsset<LWShader>("UIColor");
	m_UITextureShader = AM->GetAsset<LWShader>("UITexture");
	m_UIFontShader = AM->GetAsset<LWShader>("UIFont");
	m_UIPipeline = AM->GetAsset<LWPipeline>("UIPipeline");
	
	m_UIPipeline->SetUniformBlock(0, m_UIUniform);
	return;
}

Renderer::Renderer(LWVideoDriver *Driver, LWAllocator &Allocator) : m_Driver(Driver) {
	m_UIUniform = m_Driver->CreateVideoBuffer<LWMatrix4f>(LWVideoBuffer::Uniform, LWVideoBuffer::WriteDiscardable, 1, Allocator, nullptr);
	for (uint32_t i = 0; i < MaxFrames; i++) m_Frames[i] = GFrame(Driver, Allocator);
}

Renderer::~Renderer() {
	m_Driver->DestroyVideoBuffer(m_UIUniform);
	for (uint32_t i = 0; i < MaxFrames; i++) m_Frames[i].Release(m_Driver);
}