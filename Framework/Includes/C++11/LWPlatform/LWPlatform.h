#ifndef LWPLATFORM_H
#define LWPLATFORM_H

//Simple string concation tools.
#define LWXSTR(x) #x
#define LWSTR(x) LWXSTR(x)

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

#define LWARCH_NAMES {u8"UNKNOWN", u8"X86", u8"X64", u8"ARM", u8"ARM64"}
#define LWPLATFORM_NAMES {u8"WIN8_1", u8"WIN7", u8"LINUX", u8"OSX", u8"IOS", u8"NDK", u8"WEB" }
#define LWVIDEODRIVER_NAMES { u8"OpenGL3_3", u8"OpenGL2_1", u8"DirectX11_1", u8"OpenGLES2", u8"OpenGL4_5", u8"DirectX12", u8"DirectX9C", u8"OpenGLES3", u8"Metal", u8"Vulkan"} 

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

