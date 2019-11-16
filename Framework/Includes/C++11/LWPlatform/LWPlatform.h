#ifndef LWPLATFORM_H
#define LWPLATFORM_H

#define LWARCH_UNKNOWN 0
#define LWARCH_X86 1
#define LWARCH_X64 2
#define LWARCH_ARM 3
#define LWARCH_ARM64 4
#define LWARCH_CNT 5

#define LWPLATFORM_WIN8_1 0
#define LWPLATFORM_WIN7 1
#define LWPLATFORM_LINUX 2
#define LWPLATFORM_OSX 3
#define LWPLATFORM_IOS 4
#define LWPLATFORM_NDK 5
#define LWPLATFORM_WEB 6
#define LWPLATFORM_CNT 7

#define LWARCH_NAMES {"UNKNOWN", "X86", "X64", "ARM", "ARM64"}
#define LWPLATFORM_NAMES {"WIN8_1", "WIN7", "LINUX", "OSX", "IOS", "NDK", "WEB" }
#define LWVIDEODRIVER_NAMES { "OpenGL3_3", "OpenGL2_1", "DirectX11_1", "OpenGLES2", "OpenGL4_5", "DirectX12", "DirectX9C", "OpenGLES3", "Metal", "Vulkan"} 

#ifdef __i386__
#define LWARCH_ID LWARCH_X86
#endif
#ifdef __x86_64__
#define LWARCH_ID LWARCH_X64
#endif
#ifdef __arm__
#define LWARCH_ID LWARCH_ARM
#endif
#ifdef __aarch64__
#define LWARCH_ID LWARCH_ARM64
#endif
#ifdef _M_X64
#define LWARCH_ID LWARCH_X64
#endif
#ifdef _M_IA64
#define LWARCH_ID LWARCH_X64
#endif
#ifdef _M_IX86
#define LWARCH_ID LWARCH_X86
#endif
#ifdef _M_X86
#define LWARCH_ID LWARCH_X86
#endif
#ifdef _M_ARM
#define LWARCH_ID LWARCH_ARM
#endif
#ifndef LWARCH_ID
#define LWARCH_ID LWARCH_UNKNOWN
#endif
#endif


//Multiple platform calls may be required if different modules need specific things.
#ifdef _WIN32
#include "LWPlatform/LWPlatform_Windows.h"
#elif __APPLE__
#ifdef __OBJC__
#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE //iOS devices
#include "LWPlatform/LWPlatform_iOS.h"
#else //mac
#include "LWPlatform/LWPlatform_Mac.h"
#endif
#endif
#elif __ANDROID__ //ndk
#include "LWPlatform/LWPlatform_NDK.h"
#elif __EMSCRIPTEN__ //web implementation
#include "LWPlatform/LWPlatform_Web.h"
#else //assume linux, but might be something else!
#include "LWPlatform/LWPlatform_Linux.h"
#endif