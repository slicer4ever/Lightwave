#ifndef LWPLATFORM_WINDOWS_H
#define LWPLATFORM_WINDOWS_H
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <ShlObj.h>
#include <Windows.h>
#include <WinDNS.h>
#include <MMSystem.h>
#include <stdio.h>
#include <thread>

//uncomment to use OpenAL instead of Xaudio2 for audio backend.
//#define LW_USEOPENAL
#ifndef LW_OPENALPATH
#define LW_OPENALPATH C:/Program Files (x86)/OpenAL 1.1 SDK
#endif
#ifndef LW_VULKANSDKPATH
#define LW_VULKANSDKPATH C:/VulkanSDK/1.2.131.2
#endif

#if (WIN32_WINNT >= 0x0602 /* WIN32_WINNT_WIN8*/)
#define LWPLATFORM_ID LWPLATFORM_WIN8_1
#else
#define LWPLATFORM_ID LWPLATFORM_WIN7
#endif

#define DIRECTINPUT_VERSION DIRECTINPUT_HEADER_VERSION
//Undef names used by Lightwave that windows headers also use(because fuck commonly used names, and let's macro them apparently)
#undef DrawText
#undef GetUserName
#undef LoadImage
#undef FindResource
#undef max
#undef min

#ifndef snprintf
#define snprintf sprintf_s
#endif

#ifndef stat
#define stat _stat
#endif

#ifndef fstat
#define fstat _fstat
#endif

#ifndef fileno
#define fileno _fileno
#endif

#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

#define LWVIDEO_IMPLEMENTED_DIRECTX11
#define LWVIDEO_IMPLEMENTED_OPENGL2_1
#define LWVIDEO_IMPLEMENTED_OPENGL3_3
#define LWVIDEO_IMPLEMENTED_OPENGL4_5
//#define LWVIDEO_IMPLEMENTED_VULKAN

/*! \brief This context is provided here incase your application does need to access the underlying win32 window context information.  in general this information should not ever be required by an application. */
struct LWWindowContext {
	HINSTANCE m_Instance; /*!< \brief the application instance id.*/
	HWND m_WND; /*!< \brief the window's id. */

	STICKYKEYS m_StickyKeyState;
	TOGGLEKEYS m_ToggleKeyState;
	FILTERKEYS m_FilterKeyState;
};

typedef std::thread LWThreadType;
#endif

//Add driver contexts when necessary, please note that we want to include these only at the appropriate time, as such we do a bit of preprocessor mangling to make this work.
#ifdef LWVIDEODRIVER_DIRECTX11_H
#ifdef LWVIDEO_IMPLEMENTED_DIRECTX11
#ifndef LWVIDEODRIVER_DIRECTX11_PLATFORM_H
#define LWVIDEODRIVER_DIRECTX11_PLATFORM_H
#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11shader.h>
#include <d3d10.h>
#include <d3dcompiler.h>
#include <unordered_map>

struct LWDirectX11_1RasterContext {
	ID3D11BlendState *m_BlendState;
	ID3D11RasterizerState1 *m_RasterState;
	ID3D11DepthStencilState *m_DepthStencilState;
};

/*! \brief This context is the underlying context used in a directX video driver.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the directX api. */
struct LWDirectX11_1Context {
	IDXGISwapChain *m_DXSwapChain; /*!< \brief the direct X swap chain.*/
	ID3D11Device1 *m_DXDevice; /*!< \brief the direct x device. */
	ID3D11DeviceContext1 *m_DXDeviceContext; /*!< \brief the direct x device context. */
	ID3D11DepthStencilView *m_BackBufferDepthStencilView;
	ID3D11RenderTargetView *m_BackBuffer; /*!< \brief the default back buffer for the application. */
	uint32_t m_BoundComputeUnorderedCount = 0;/*!< \brief compute unordered access views that are currently bounded(these need to be cleared if necessary). */
	std::unordered_map<uint32_t, ID3D11SamplerState*> m_SamplerMap; /*!< \brief the map of samplers for various textures. */
	std::unordered_map<uint32_t, LWDirectX11_1RasterContext> m_RasterMap; /*!< \brief map of different raster states that have been made throughout the life of the application. */
};


#endif
#endif
#endif

#ifdef LWVIDEODRIVER_VULKAN_H
#ifdef LWVIDEO_IMPLEMENTED_VULKAN
#ifndef LWVIDEODRIVER_VULKAN_PLATFORM_H
#define LWVIDEODRIVER_VULKAN_PLATFORM_H
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <vector>
//#include LWSTR(LW_VULKANSDKPATH/include/vulkan/vulkan.h)
//#include LWSTR(LW_VULKANSDKPATH/include/vulkan/vulkan_win32.h)
#if LWARCH_ID==LWARCH_X86
#pragma comment(lib, LWSTR(LW_VULKANSDKPATH/Lib32/vulkan-1.lib))
#else
#pragma comment(lib, LWSTR(LW_VULKANSDKPATH/Lib/vulkan-1.lib))
#endif

struct LWVulkan_Context{
	VkInstance m_Instance = nullptr;
	VkDevice m_Device = nullptr;
	VkQueue m_GraphicsQueue = nullptr;
	VkSurfaceKHR m_Surface = nullptr;
	VkSwapchainKHR m_Swapchain = nullptr;
	std::vector<VkImage> m_SwapImageList;
	std::vector<VkImageView> m_SwapImageViewList;
};

#endif
#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL4_5_H
#ifdef LWVIDEO_IMPLEMENTED_OPENGL4_5
#ifndef LWVIDEODRIVER_OPENGL4_5_PLATFORM_H
#define LWVIDEODRIVER_OPENGL4_5_PLATFORM_H
#include <gl/glew.h>
#include <gl/wglew.h>
/*! \brief This context is the underlying context used in the windows openGL video driver.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL4_5Context {
	HGLRC m_GLRC; /*!< \brief the openGL rendering context. */
	HDC m_DC; /*!< \brief the openGL drawing context. */
};

#endif
#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL3_3_H
#ifdef LWVIDEO_IMPLEMENTED_OPENGL3_3
#ifndef LWVIDEODRIVER_OPENGL3_3_PLATFORM_H
#define LWVIDEODRIVER_OPENGL3_3_PLATFORM_H
#include <gl/glew.h>
#include <gl/wglew.h>
/*! \brief This context is the underlying context used in the windows openGL video driver.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL3_3Context {
	HGLRC m_GLRC; /*!< \brief the openGL rendering context. */
	HDC m_DC; /*!< \brief the openGL drawing context. */
};

#endif
#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL2_1_H
#ifdef LWVIDEO_IMPLEMENTED_OPENGL2_1
#ifndef LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#define LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#include <gl/glew.h>
#include <gl/wglew.h>
struct LWOpenGL2_1Context {
	HGLRC m_GLRC; /*!< \brief the openGL rendering context. */
	HDC m_DC; /*!< \brief the openGL drawing context. */
	uint32_t m_ActiveAttribs = 0; /*!< \brief the number of currently active attribute arrays. */
	uint32_t m_ActiveAttributeIDs[32];
};

#endif
#endif
#endif

#ifdef LW_USEOPENAL

#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H

#include LWSTR(LW_OPENALPATH/include/al.h)
#include LWSTR(LW_OPENALPATH/include/alc.h)
#if LWARCH_ID == LWARCH_X64
#pragma comment(lib, LWSTR(LW_OPENALPATH/libs/Win64/OpenAL32.lib))
#else
#pragma comment(lib, LWSTR(LW_OPENALPATH/libs/Win32/OpenAL32.lib))
#endif

struct LWAudioDriverContext {
	ALCdevice *m_Device;
	ALCcontext *m_Context;
};

#endif
#endif

#ifdef LWSOUND_H
#ifndef LWSOUND_PLATFORM_H
#define LWSOUND_PLATFORM_H
#define LWSOUND_RESERVEBUFFERSIZE 32*1024 //128KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
	uint32_t m_Source;
	uint32_t m_Buffers[LWSOUND_RESERVECNT];
	char m_ReserveBuffer[LWSOUND_RESERVECNT][LWSOUND_RESERVEBUFFERSIZE];
	uint32_t m_ReserveIdx = 0;
	uint32_t m_BufferIdx = 0;
	uint32_t m_SeekSamples = 0;
};

#endif
#endif

#else
#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H
#if (WIN32_WINNT >= 0x0602 /* WIN32_WINNT_WIN8*/)
#if defined(_MSC_VER) && (_MSC_VER < 1700)
#error DirectX Tool Kit for Audio does not support VS 2010 without the DirectX SDK 
#endif
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>
#include <xapofx.h>
#pragma comment(lib,"xaudio2.lib")
#else
// Using XAudio 2.7 requires the DirectX SDK
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\comdecl.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\xaudio2.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\xaudio2fx.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\xapofx.h>
#pragma warning(push)
#pragma warning( disable : 4005 )
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\x3daudio.h>
#pragma warning(pop)
#pragma comment(lib,"x3daudio.lib")
#pragma comment(lib,"xapofx.lib")
#endif
#include <functional>

struct LWAudioDriverContext {
	IXAudio2 *m_Device;
	IXAudio2MasteringVoice *m_MasterVoice;
	X3DAUDIO_HANDLE m_3DHandle;
	X3DAUDIO_LISTENER m_Listener;
	uint32_t m_OpSet = 1;
	bool m_OpChanged = false;

	LWAudioDriverContext() = default;
};

#endif
#endif

#ifdef LWSOUND_H
#ifndef LWSOUND_PLATFORM_H
#define LWSOUND_PLATFORM_H
#include <xaudio2.h>
#include <x3daudio.h>

#define LWSOUND_RESERVEBUFFERSIZE 32*1024 //32KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
	IXAudio2SourceVoice *m_Source = nullptr;
	X3DAUDIO_EMITTER m_Emitter;
	char m_ReserveBuffer[LWSOUND_RESERVECNT][LWSOUND_RESERVEBUFFERSIZE]; //128KB
	uint32_t m_ReserveIdx = 0;
	uint32_t m_SeekSamples = 0;

	LWSoundContext() = default;
};

#endif
#endif
#endif