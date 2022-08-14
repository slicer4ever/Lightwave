#include "LWVideo/LWVideoDrivers/LWVideoDriver_DirectX11_1.h"
#include "LWVideo/LWTypes.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMath.h"
#include "LWCore/LWLogger.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>
#include <algorithm> 

//LWVideoDriver_DirectX11_1:
LWVideoDriver_DirectX11_1 *LWVideoDriver_DirectX11_1::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWDirectX11_1Context Context;
	LWWindowContext WinCon = Window->GetContext();
	IDXGIDevice2* DXGIDevice = nullptr; //Uh...technically we now require feature level 11.3...
	IDXGIAdapter* DXGIAdapter = nullptr;
	IDXGIFactory2* DXGIFactory = nullptr;
	DXGI_SWAP_CHAIN_DESC1 scd = { 0,0, DXGI_FORMAT_R8G8B8A8_UNORM, false, {1,0}, DXGI_USAGE_RENDER_TARGET_OUTPUT, 3,DXGI_SCALING_NONE, DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, DXGI_ALPHA_MODE_IGNORE, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT };
	D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_11_1;
	uint32_t Flag = 0;
	
	if((Type&DebugLayer)!=0) Flag |= D3D11_CREATE_DEVICE_DEBUG;

	auto EvaluateError = [&Context](const LWUTF8Iterator &FunctionName, HRESULT Res)->bool {
		if(LWLogCriticalIf<64>(Res==S_OK, "Error: '{}': {:#x}\n", FunctionName, Res)) return true;
		if (Context.m_DXSwapChain) Context.m_DXSwapChain->Release();
		if (Context.m_DXDeviceContext) Context.m_DXDeviceContext->Release();
		if (Context.m_DXDevice) Context.m_DXDevice->Release();
		return false;
	};
	
	if (!EvaluateError("D3D11CreateDevice", D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, Flag, &Level, 1, D3D11_SDK_VERSION, (ID3D11Device**)&Context.m_DXDevice, nullptr, (ID3D11DeviceContext**)&Context.m_DXDeviceContext))) return nullptr;
	else if (!EvaluateError("QueryInterface<IDXGIDevice2>", Context.m_DXDevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)&DXGIDevice))) return nullptr;
	else if (!EvaluateError("GetParent<IDXGIAdapter>", DXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&DXGIAdapter))) return nullptr;
	else if (!EvaluateError("GetParent<IDXGIFactory2>", DXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&DXGIFactory))) return nullptr;
	else if (!EvaluateError("CreateSwapChainForHwnd", DXGIFactory->CreateSwapChainForHwnd(Context.m_DXDevice, WinCon.m_WND, &scd, nullptr, nullptr, (IDXGISwapChain1**)&Context.m_DXSwapChain))) return nullptr;
	else {
		D3D11_TEXTURE2D_DESC Desc;// = { Size.x, Size.y, 1, 1, Formats[PackType],{ 1, 0 }, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | ((TextureState&LWTexture::RenderTarget) != 0 ? D3D11_BIND_RENDER_TARGET : 0) , 0, 0 }
		Context.m_BackBuffer = nullptr;
		Context.m_BackBufferDepthStencilView = nullptr;
		ID3D11Texture2D *TexBackBuffer = nullptr;
		ID3D11Texture2D *DepthStencilBuffer = nullptr;
		Context.m_DXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&TexBackBuffer);
		TexBackBuffer->GetDesc(&Desc);
		Desc = { Desc.Width, Desc.Height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT , { 1, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, 0 };
		D3D11_RENDER_TARGET_VIEW_DESC RTV = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D11_RTV_DIMENSION_TEXTURE2D, {0u} };
		if (!EvaluateError("CreateTexture2D<Depth>", Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, &DepthStencilBuffer))) return nullptr;
		Context.m_DXDevice->CreateDepthStencilView(DepthStencilBuffer, nullptr, &Context.m_BackBufferDepthStencilView);
		Context.m_DXDevice->CreateRenderTargetView(TexBackBuffer, &RTV, &Context.m_BackBuffer);
		Context.m_DXDeviceContext->OMSetRenderTargets(1, &Context.m_BackBuffer, Context.m_BackBufferDepthStencilView);
		TexBackBuffer->Release();
		DepthStencilBuffer->Release();
		return Window->GetAllocator()->Create<LWVideoDriver_DirectX11_1>(Window, Context, 256);
	}
	return nullptr;
}

bool LWVideoDriver_DirectX11_1::DestroyVideoContext(LWVideoDriver_DirectX11_1 *Driver) {
	LWDirectX11_1Context &Context = Driver->GetContext();
	if (Driver->GetWindow()->GetFlag()&LWWindow::Fullscreen) Context.m_DXSwapChain->SetFullscreenState(false, nullptr);
	for (auto &&Iter : Context.m_SamplerMap) {
		Iter.second->Release();
	}
	for (auto &&Iter : Context.m_RasterMap) {
		auto &RasterContext = Iter.second;
		RasterContext.m_RasterState->Release();
		RasterContext.m_DepthStencilState->Release();
		RasterContext.m_BlendState->Release();
	}
	Context.m_BackBufferDepthStencilView->Release();
	Context.m_BackBuffer->Release();
	Context.m_DXSwapChain->Release();
	Context.m_DXDevice->Release();
	Context.m_DXDeviceContext->Release();
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_DirectX11_1::Update(void) {
	uint32_t Flag = m_Window->GetFlag();
	if ((Flag&(LWWindow::FocusChanged | LWWindow::Focused | LWWindow::Fullscreen)) == (LWWindow::FocusChanged | LWWindow::Focused | LWWindow::Fullscreen)) {
		m_Context.m_DXSwapChain->SetFullscreenState(true, nullptr);
	}
	LWVector2i Size = m_Window->GetSize();
	if (Size.x == 0 || Size.y == 0) return false;
	if (Flag&LWWindow::SizeChanged) {
		ID3D11Texture2D *TexBackBuffer;
		ID3D11Texture2D *DepthStencilBuffer;
		D3D11_RENDER_TARGET_VIEW_DESC RTV = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D11_RTV_DIMENSION_TEXTURE2D, {0u} };
		if(!m_ActiveFrameBuffer) m_Context.m_DXDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		if(m_Context.m_BackBuffer) m_Context.m_BackBuffer->Release();
		if(m_Context.m_BackBufferDepthStencilView) m_Context.m_BackBufferDepthStencilView->Release();
		
		if(!LWLogCriticalIf(m_Context.m_DXSwapChain->ResizeBuffers(0, Size.x, Size.y, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT)==S_OK, "Error: Failed to resize backbuffers.")) return false;

		m_WaitObject = nullptr;
		m_Context.m_DXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&TexBackBuffer);
		m_Context.m_DXDevice->CreateRenderTargetView(TexBackBuffer, &RTV, &m_Context.m_BackBuffer);
		
		D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT ,{ 1, 0 }, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, 0 };
		HRESULT Res = m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, &DepthStencilBuffer);
		
		LWLogCriticalIf<128>(Res==S_OK, "Failed making new depthstencil texture: {:#x}", Res);

		Res = m_Context.m_DXDevice->CreateDepthStencilView(DepthStencilBuffer, nullptr, &m_Context.m_BackBufferDepthStencilView);
		LWLogCriticalIf<128>(Res == S_OK, "Failed making new depthstencil view: {:#x}", Res);

		if(!m_ActiveFrameBuffer) m_Context.m_DXDeviceContext->OMSetRenderTargets(1, &m_Context.m_BackBuffer, m_Context.m_BackBufferDepthStencilView);
		if(TexBackBuffer) TexBackBuffer->Release();
		if(DepthStencilBuffer) DepthStencilBuffer->Release();
	}
	if (!m_WaitObject) m_WaitObject = m_Context.m_DXSwapChain->GetFrameLatencyWaitableObject();
	WaitForSingleObjectEx(m_WaitObject, 1000, true);
	return true;
}
