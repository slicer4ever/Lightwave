#include "LWVideo/LWVideoDrivers/LWVideoDriver_DirectX11_1.h"
#include "LWVideo/LWTypes.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWMath.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>
#include <algorithm> 


LWVideoDriver_DirectX11_1 *LWVideoDriver_DirectX11_1::MakeVideoDriver(LWWindow *Window) {
	LWDirectX11_1Context Context;
	LWWindowContext WinCon = Window->GetContext();
	DXGI_SWAP_CHAIN_DESC scd = { { 0, 0,{ 0, 0 }, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED },{ 4, 0 }, DXGI_USAGE_RENDER_TARGET_OUTPUT, 1, WinCon.m_WND, (Window->GetFlag()&LWWindow::Fullscreen) == 0, DXGI_SWAP_EFFECT_DISCARD, 0 };
	D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_11_1;
	uint32_t Flag = 0;
#ifdef _DEBUG
	Flag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT Res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, Flag, &Level, 1, D3D11_SDK_VERSION, &scd, &Context.m_DXSwapChain, (ID3D11Device**)&Context.m_DXDevice, nullptr, (ID3D11DeviceContext**)&Context.m_DXDeviceContext);
	if (Res != S_OK) {
		std::cout << "Error: 'D3D11CreateDeviceAndSwapChain: " << std::hex << Res << std::dec << "'" << std::endl;
		if (Res == DXGI_ERROR_UNSUPPORTED) return nullptr;
	} else {
		D3D11_TEXTURE2D_DESC Desc;// = { Size.x, Size.y, 1, 1, Formats[PackType],{ 1, 0 }, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | ((TextureState&LWTexture::RenderTarget) != 0 ? D3D11_BIND_RENDER_TARGET : 0) , 0, 0 };

		Context.m_BackBuffer = nullptr;
		Context.m_BackBufferDepthStencilView = nullptr;
		ID3D11Texture2D *TexBackBuffer = nullptr;
		ID3D11Texture2D *DepthStencilBuffer = nullptr;
		Context.m_DXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&TexBackBuffer);
		TexBackBuffer->GetDesc(&Desc);
		Desc = { Desc.Width, Desc.Height, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT , { 4, 0}, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, 0 };
		HRESULT Res = Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, &DepthStencilBuffer);
		if (FAILED(Res)) {
			std::cout << "Error creating depth stencil backbuffer." << std::endl;
			Context.m_DXSwapChain->Release();
			Context.m_DXDevice->Release();
			Context.m_DXDeviceContext->Release();
			return nullptr;
		}
		Context.m_DXDevice->CreateDepthStencilView(DepthStencilBuffer, nullptr, &Context.m_BackBufferDepthStencilView);
		Context.m_DXDevice->CreateRenderTargetView(TexBackBuffer, nullptr, &Context.m_BackBuffer);
		Context.m_DXDeviceContext->OMSetRenderTargets(1, &Context.m_BackBuffer, Context.m_BackBufferDepthStencilView);
		TexBackBuffer->Release();
		DepthStencilBuffer->Release();
		return Window->GetAllocator()->Allocate<LWVideoDriver_DirectX11_1>(Window, Context, 256);
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
		if(!m_ActiveFrameBuffer) m_Context.m_DXDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		if(m_Context.m_BackBuffer) m_Context.m_BackBuffer->Release();
		if(m_Context.m_BackBufferDepthStencilView) m_Context.m_BackBufferDepthStencilView->Release();
		m_Context.m_DXSwapChain->ResizeBuffers(1, Size.x, Size.y, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_EFFECT_DISCARD);

		m_Context.m_DXSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&TexBackBuffer);
		m_Context.m_DXDevice->CreateRenderTargetView(TexBackBuffer, nullptr, &m_Context.m_BackBuffer);
		
		D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y, 1, 1, DXGI_FORMAT_D24_UNORM_S8_UINT ,{ 4, 0 }, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL, 0, 0 };
		HRESULT Res = m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, &DepthStencilBuffer);
		if(FAILED(Res)){
			std::cout << "Failed making new depthstencil texture." << std::endl;
		}
		Res = m_Context.m_DXDevice->CreateDepthStencilView(DepthStencilBuffer, nullptr, &m_Context.m_BackBufferDepthStencilView);
		if(FAILED(Res)){
			std::cout << "Failed making new depthstencil view." << std::endl;
		}
		if(!m_ActiveFrameBuffer) m_Context.m_DXDeviceContext->OMSetRenderTargets(1, &m_Context.m_BackBuffer, m_Context.m_BackBufferDepthStencilView);
		if(TexBackBuffer) TexBackBuffer->Release();
		if(DepthStencilBuffer) DepthStencilBuffer->Release();
	}
	return true;
}
