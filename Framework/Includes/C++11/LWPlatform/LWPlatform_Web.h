#ifndef LWPLATFORM_WEB_H
#define LWPLATFORM_WEB_H
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <unistd.h>
#include <LWCore/LWVector.h>
#include <emscripten.h>
#include <emscripten/html5.h>

//Define some cross-platform network defines:
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (~0)

#define UDP_CONNRESET 0x9800000C
//Define All atoms to be used:

//Defined to act similar to Windows:
#define X11_MOUSE_SCROLL_INCREMENT 120
#define LWPLATFORM_ID LWPLATFORM_WEB

#define LWWEBEVENT_FOCUS 0
#define LWWEBEVENT_RESIZE 1
#define LWWEBEVENT_MOUSE 2
#define LWWEBEVENT_SCROLL 3
#define LWWEBEVENT_KEY 4

struct LWWebGenericEvent {
	uint32_t m_Type;
	uint32_t m_SubType;
};

struct LWWebMouseEvent {
	uint32_t m_Type;
	uint32_t m_SubType;
	uint32_t m_Mousex;
	uint32_t m_Mousey;
	uint32_t m_ButtonState;
};

struct LWWebKeyEvent {
	uint32_t m_Type;
	uint32_t m_SubType;
	uint32_t m_KeyCode;
	uint32_t m_CharCode;
};

struct LWWebSizeEvent {
	uint32_t m_Type;
	uint32_t m_SubType;
	uint32_t m_Width;
	uint32_t m_Height;
};

struct LWWebEvent {
	union {
		uint32_t m_Type;
		LWWebGenericEvent m_FocusEvent;
		LWWebMouseEvent m_MouseEvent;
		LWWebKeyEvent m_KeyEvent;
		LWWebSizeEvent m_SizeEvent;
		uint8_t Padding[32];
	};
};

/*! \brief This context is provided here incase your application does need to access the underlying web window context information.  in general this information should not ever be required by an application. */
struct LWWindowContext {
	//Web context:
	static const uint32_t MaxEvents = 64;
	LWWebEvent m_Events[MaxEvents];
	uint32_t m_EventCount;

	bool PushEvent(LWWebEvent &Event) {
		if (m_EventCount >= MaxEvents) return false;
		m_Events[m_EventCount++] = Event;
		return true;
	}

	bool PopEvent(LWWebEvent &Res) {
		if (!m_EventCount) return false;
		Res = m_Events[m_EventCount - 1];
		m_EventCount--;
		return true;
	}

	LWWindowContext() : m_EventCount(0) {}


};
#undef None

typedef uint32_t LWThreadType;

#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif

#define LWVIDEO_IMPLEMENTED_OPENGLES2
#endif

#ifdef LWVIDEODRIVER_OPENGLES2_H
#ifndef LWVIDEODRIVER_OPENGLES2_PLATFORM_H
#define LWVIDEODRIVER_OPENGLES2_PLATFORM_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

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

/*!< \brief the ndk openGL ES Context. */
struct LWOpenGLES2Context {
	EGLDisplay m_Display; /*!< \brief the display object for the android app. */
	EGLContext m_Context; /*!< \brief the context object for the android app. */
	EGLSurface m_Surface; /*!< \brief the surface object for the android app. */
	EGLConfig m_DisplayConfig; /*!< \brief the config object for the android app. */
};
#endif
#endif


#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H
#include <AL/al.h>
#include <AL/alc.h>

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
