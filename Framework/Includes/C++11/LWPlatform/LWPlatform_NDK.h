#ifndef LWPLATFORM_NDK_H
#define LWPLATFORM_NDK_H
#include <jni.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <atomic>
#include "LWCore/LWTypes.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include <sstream>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <thread>

//Network defines:
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (~0)

#define UDP_CONNRESET 0x9800000C

#define LWMAXEVENTQUEUESIZE 50 /*!< \brief the max number of events that can build up, before they are dropped. */
#define LWMAXKEYBOARDTEXTBUFFER 256 /*!< \brief the max length of the internal keyboard text object. */
#define LWMAXTEXTBUFFER 256 /*!< \brief the max length of the app and storage directory paths. */

#define LWPLATFORM_ID LWPLATFORM_NDK /*!< \brief platform id. */
/*! \brief the LWNDKEventCode is a list of event codes to be passed to our application. */
enum LWNDKEventCode {
	Destroy = 0, /*!< \brief a destroy app event has been triggered. */
	FocusLost, /*!< \brief the focus has been lost for the app. */
	FocusGained, /*!< \brief the focus has been gained for the app. */
	SaveRequested, /*!< \brief the os has requested the app save it's current state. */
	Pause, /*!< \brief the application is going to be paused. */
	Stop, /*!< \brief the application is going to be stopped. */
	Resume, /*!< \brief the application has resumed from a paused state. */
	Start, /*!< \brief the application has been started. */
	WindowCreated, /*!< \brief the native window for drawing has been created. */
	WindowDestroyed, /*!< \brief the native window for drawing has been destroyed. */
	WindowResized, /*!< \brief the native window for drawing has changed size. */
	InputQueueCreated, /*!< \brief the input queue has been created. */
	InputQueueDestroyed, /*!< \brief the input queue has been destroyed. */
	ConfigurationChanged /*!< \brief the device configuration has changed. */
};

/*!< \brief the LWNDKEvent is the structure to contain all the information an event can contain. */
struct LWNDKEvent {
	ANativeActivity *m_Activity; /*!< \brief the native activity that was passed to the event. */
	ANativeWindow *m_Window; /*!< \brief the window that was passed to the event. */
	AInputQueue *m_InputQueue; /*!< \brief the input queue that is going to be changed. */
	uint32_t m_EventCode; /*!< \brief the event code that represents the event. */
};

/*! \brief This context is the contains the parameters passed to the entry point for the android application. */
struct LWNDKAppContext {
	char m_AppDirectory[LWMAXTEXTBUFFER]; /*!< \brief the actual app's working directory. */
	char m_StorageDirectory[LWMAXTEXTBUFFER]; /*!< \brief the actual app's user and app directorys. */
	LWConcurrentFIFO<LWNDKEvent, LWMAXEVENTQUEUESIZE> m_EventLoop; /*!< \brief the event loop that is processed in LWWindow. */
	ANativeActivity *m_App; /*!< \brief the native activity. */
	JNIEnv *m_AppEnv; /*!< \brief the applications jni environment, do not access the environment nativeActivity contains, as that environment sits on a diffrent thread. */
	uint32_t m_AppEnvCounter; /*!< \brief the applications jni app environment counter for detecting jni enviroment changes(as m_AppEnv may result in the same pointer being returned, we need to use a seperate counter to track changes.) */
	void *m_SavedState; /*!< \brief the saved state passed to the onCreate method. */
	size_t m_SaveSateSize; /*!< \brief the save state size that was passed to the onCreate method. */
	std::atomic<uint32_t> m_EventState; /*!< \brief this variable is used for synchronizing events between the main app thread, and the ndk app thread. */
	std::stringstream m_CoutStream; /*!< \brief the redirected cout stream, which is redirected to LogCat with the tag LWApp. */
};

/*!< \brief the ndk window context used by LWWindow. */
struct LWWindowContext {
	ANativeWindow *m_ActiveWindow; /*!< \brief the active window for the android app. */
	AInputQueue *m_InputQueue; /*!< \brief the active input queue for the android app. */
	ASensorEventQueue *m_SensorQueue; /*!< \brief the active sensor queue for the android app. */
	ASensorManager *m_SensorManager; /*!< \brief the sensor manager instance for the android app. */
	uint64_t m_LastConfigCheck; /*!< \brief the last time config was checked for changed. */
	bool m_SyncState; /*!< \brief tells the window if it needs to increment the android state on the next frame, this is done on the next frame to allow the application to process the requested state change before it is lost. */
};

/*! \brief This variable is declared in the LWWindow_NDK.cpp and is a global scope variable, it is provided for applications that require processing the android saved state. */
extern LWNDKAppContext LWAppContext;

void NDKLog(const char *F);

void NDKLogf(const char *Fmt, ...);

/*!< \brief flushs the ndk cout output for debugging purposes. */
void NDKFlushOutput(void);

typedef std::thread LWThreadType;

#define LWVIDEO_IMPLEMENTED_OPENGLES2
#endif

//Add driver contexts when necessary, please note that we want to include these only at the appropriate time, as such we do a bit of preprocessor mangling to make this work.
#ifdef LWVIDEODRIVER_OPENGLES2_H
#ifndef LWVIDEODRIVER_OPENGLES2_PLATFORM_H
#define LWVIDEODRIVER_OPENGLES2_PLATFORM_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
/*!< \brief the ndk openGL ES Context. */
struct LWOpenGLES2Context {
	EGLDisplay m_Display; /*!< \brief the display object for the android app. */
	EGLContext m_Context; /*!< \brief the context object for the android app. */
	EGLSurface m_Surface; /*!< \brief the surface object for the android app. */
	EGLConfig m_DisplayConfig; /*!< \brief the config object for the android app. */
	ANativeWindow *m_ActiveWindow; /*!< \brief the active window for the android app, we check this against the window to see if we need to destroy or create the context. */
};
#endif
#endif

#ifdef LWVIDEODRIVER_DIRECTX11_H
#ifndef LWVIDEODRIVER_DIRECTX11_PLATFORM_H
#define LWVIDEODRIVER_DIRECTX11_PLATFORM_H

/*!< \brief empty context for non-existant DX11 on this platform. */
struct LWDirectX11_1Context {};

#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL4_4_H
#ifndef LWVIDEODRIVER_OPENGL4_4_PLATFORM_H
#define LWVIDEODRIVER_OPENGL4_4_PLATFORM_H
struct LWOpenGL4_4Context {};

#endif
#endif


#ifdef LWVIDEODRIVER_OPENGL3_2_H
#ifndef LWVIDEODRIVER_OPENGL3_2_PLATFORM_H
#define LWVIDEODRIVER_OPENGL3_2_PLATFORM_H

/*!< \brief empty context for non-existant OpenGL3.2 on this platform. */
struct LWOpenGL3_2Context {};

#endif
#endif

#ifdef LWVIDEODRIVER_OPENGL2_1_H
#ifndef LWVIDEODRIVER_OPENGL2_1_PLATFORM_H
#define LWVIDEODRIVER_OPENGL2_1_PLATFORM_H

struct LWOpenGL2_1Context {};

#endif
#endif


#ifdef LWAUDIODRIVER_H
#ifndef LWAUDIODRIVER_PLATFORM_H
#define LWAUDIODRIVER_PLATFORM_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

struct LWAudioDriverContext {
	SLObjectItf m_EngineObjectItf; //Object Interface.
	SLEngineItf m_EngineItf; //Engine interface
	SLObjectItf m_OutputMixItf; //Output mix interface.
};

#endif
#endif

#ifdef LWSOUND_H
#ifndef LWSOUND_PLATFORM_H
#define LWSOUND_PLATFORM_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#define LWSOUND_RESERVEBUFFERSIZE 32*1024 //128KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
	char m_ReserveBuffer[LWSOUND_RESERVEBUFFERSIZE*LWSOUND_RESERVECNT];
	SLObjectItf m_PlayerItf;
	SLPlayItf m_PlayItf;
	SLBufferQueueItf m_BufferQueueItf;
	SLVolumeItf m_VolumeItf;
	uint32_t m_ReserveIdx;
};

#endif
#endif