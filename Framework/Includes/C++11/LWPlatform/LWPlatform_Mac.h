#ifndef LWPLATFORM_MAC_H
#define LWPLATFORM_MAC_H
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>


//Define some cross-platform network defines:
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (~0)

#define UDP_CONNRESET 0x9800000C
#define LWPLATFORM_ID LWPLATFORM_OSX

/*! \brief This context is provided here incase your application does need to access the underlying mac OS window context information.  in general this information should not ever be required by an application. */
struct LWWindowContext {
	NSApplication *App = nullptr; /*!< \brief this is a cached pointer to the main application. */
	NSWindow *Window = nullptr; /*!< \brief this is the associated window for the context. */
};
typedef std::thread LWThreadType;

#define LWVIDEO_IMPLEMENTED_OPENGL2_1
#define LWVIDEO_IMPLEMENTED_OPENGL3_2
#define LWVIDEO_IMPLEMENTED_OPENGL4_4
#endif

#ifdef LWVIDEODRIVER_OPENGL3_2_H
#ifndef LWVIDEODRIVER_OPENGL3_2_PLATFORM_H
#define LWVIDEODRIVER_OPENGL3_2_PLATFORM_H
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif
#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

struct LWOpenGL3_2Context {
	uint32_t m_VAOID; /*!< \brief the Vertex array object id for the entire context. */
};
#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL4_4_H
#ifndef LWVIDEODRIVER_OPENGL4_4_PLATFORM_H
#define LWVIDEODRIVER_OPENGL4_4_PLATFORM_H
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif
#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

struct LWOpenGL4_4Context {
	uint32_t m_VAOID; /*!< \brief the Vertex array object id for the entire context. */
};

#endif
#endif


#ifdef LWVIDEODRIVER_OPENGL2_1_H
#ifndef LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#define LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif
#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

#ifndef glVertexAttribIPointer
#define glVertexAttribIPointer glVertexAttribIPointerEXT
#endif



struct LWOpenGL2_1Context {};
#endif
#endif


#ifdef LW_USEOPENAL

#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H
#include <al.h>
#include <alc.h>

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
	char m_ReserveBuffer[LWSOUND_RESERVEBUFFERSIZE*LWSOUND_RESERVECNT];
	uint32_t m_ReserveIdx;
	uint32_t m_BufferIdx;
};

#endif
#endif

#else
#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>

struct LWAudioDriverContext {
};

#endif
#endif

#ifdef LWSOUND_H
#ifndef LWSOUND_PLATFORM_H
#define LWSOUND_PLATFORM_H
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>

#define LWSOUND_RESERVEBUFFERSIZE 32*1024 //128KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
    AudioQueueRef m_Queue;
    AudioQueueBufferRef m_QueueBuffers[LWSOUND_RESERVECNT]; //<--this should be the same value as LWSound::ReserveSlices, but there is no decent way to link to that value that i know of. :(
	uint32_t m_ReserveIdx;
};

#endif
#endif
#endif

