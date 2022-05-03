#ifndef LWPLATFORM_LINUX_H
#define LWPLATFORM_LINUX_H
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>

//Define some cross-platform network defines:
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (~0)

#define UDP_CONNRESET 0x9800000C
//Define All atoms to be used:

#define X11_WM_DELETE_WINDOW 0
#define X11_NET_WM_STATE 1
#define X11_NET_WM_STATE_FULLSCREEN 2
#define X11_NET_WM_STATE_HIDDEN 3
#define X11_NET_WM_ACTION_MOVE 4
#define X11_NET_WM_ACTION_RESIZE 5
#define X11_MOTIF_WM_HINTS 6
#define X11_XA_STRING 7
#define X11_CLIPBOARD 8
#define X11_NET_FRAME_EXTENTS 9
#define X11_ATOM_NAMES {"WM_DELETE_WINDOW","_NET_WM_STATE", "_NET_WM_STATE_FULLSCREEN", \
						"_NET_WM_STATE_HIDDEN", "_NET_WM_ACTION_MOVE", "_NET_WM_ACTION_RESIZE",\
						"_MOTIF_WM_HINTS", "XA_STRING", "CLIPBOARD", "_NET_FRAME_EXTENTS" }
#define X11_ATOM_COUNT 10

//Define undefined mouse buttons, these were captured in the input was pressed, they may not match all x server mouse implementations.
#define X11_MOUSE_X1 8
#define X11_MOUSE_X2 9

//Defined to act similar to Windows:
#define X11_MOUSE_SCROLL_INCREMENT 120
#define LWPLATFORM_ID LWPLATFORM_LINUX

/*! \brief This context is provided here incase your application does need to access the underlying x11 window context information.  in general this information should not ever be required by an application. */
struct LWWindowContext {
	//X11 context:
	Atom m_AtomList[X11_ATOM_COUNT]; /*!< \brief a list of atoms that the framework requires. */
	Display *m_Display; /*!< \brief the display object used for communicating with the x11 server. */
	Window m_Root; /*!< \brief the root window id. */
	Window m_Parent; /*!< \brief the parent window id. */
	Window m_Window; /*!< \brief the window id. */
	Colormap m_ColorMap; /*!< \brief the colormap for the window. */
};
typedef std::thread LWThreadType;
#undef None

#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif

#define LWVIDEO_IMPLEMENTED_OPENGL2_1
#define LWVIDEO_IMPLEMENTED_OPENGL3_3
#endif

#ifdef LWVIDEODRIVER_OPENGL3_3_H
#ifndef LWVIDEODRIVER_OPENGL3_3_PLATFORM_H
#define LWVIDEODRIVER_OPENGL3_3_PLATFORM_H
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/glext.h>

//These two defines were taken from the msdn gl headers.
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif

#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

/*! \brief This context is the underlying context used in the x11 openGL video driver.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL3_3Context {
	GLXContext m_GLContext;/*!< \brief the x11 openGL context. */
	uint32_t m_VAOID; /*!< \brief the Vertex array object id for the entire context. */
};

#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL4_5_H
#ifndef LWVIDEODRIVER_OPENGL4_5_PLATFORM_H
#define LWVIDEODRIVER_OPENGL4_5_PLATFORM_H
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/glext.h>

//These two defines were taken from the msdn gl headers.
#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif

#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

struct LWOpenGL4_5Context {
	GLXContext m_GLContext;
	uint32_t m_VAOID;
};

#endif
#endif


#ifdef LWVIDEODRIVER_OPENGL2_1_H
#ifndef LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#define LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#include <GL/glew.h>
#include <GL/glxew.h>
#include <GL/glext.h>

#ifndef GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT 0x8E8E
#endif

#ifndef GL_COMPRESSED_RGBA_BPTC_UNORM_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM_EXT 0x8E8C
#endif

/*! \brief This context is the underlying context used in the x11 openGL video driver.  the application should never require accessing it directly, but it is provided here incase the application is specifically targeting the openGL api. */
struct LWOpenGL2_1Context {
	GLXContext m_GLContext; /*!< \brief the x11 openGL context. */
	uint32_t m_ActiveAttribs = 0; /*!< \brief the number of currently active attribute arrays. */
	uint32_t m_ActiveAttributeIDs[32];
};
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
#include <pulse/pulseaudio.h>
#include <signal.h>

struct LWAudioDriverContext {
	pa_mainloop *m_MainLoop;
	pa_mainloop_api *m_MainLoopAPI;
	pa_context *m_Context;
};

#endif
#endif

#ifdef LWSOUND_H
#ifndef LWSOUND_PLATFORM_H
#define LWSOUND_PLATFORM_H
#include <pulse/pulseaudio.h>

#define LWSOUND_RESERVEBUFFERSIZE 32*1024 //128KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
	pa_stream *m_Source = nullptr;
	char m_ReserveBuffer[LWSOUND_RESERVEBUFFERSIZE*LWSOUND_RESERVECNT];
	uint32_t m_ReserveIdx = 0;
	uint32_t m_SeekSamples = 0;

	LWSoundContext() = default;
};

#endif
#endif
#endif
