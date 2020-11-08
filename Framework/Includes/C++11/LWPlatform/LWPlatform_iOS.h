#ifndef LWPLATFORM_IOS_H
#define LWPLATFORM_IOS_H
#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#include "LWCore/LWTypes.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include "LWCore/LWVector.h"
#include <atomic>
#include <sys/poll.h>
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

#define LWPLATFORM_ID LWPLATFORM_IOS

#define LWMAXTOUCHPOINTS 8 /*!< \brief the maximum number of multi-touch points supported at any time. */
#define LWMAXKEYBOARDTEXTBUFFER 256 /*!< \brief the max length of the internal keyboard text object. */

@interface MainAppDelegate : NSObject <UIApplicationDelegate>{}
@end


@interface LWUITextField : UITextField<UITextFieldDelegate>
@end

@interface LWIOSWindow : UIWindow{}
@end

/*! \brief the LWIOSEventCode is a list of event codes to be passed to our application. */
enum LWIOSEventCode {
	Destroy = 0, /*!< \brief a destroy app event has been triggered. */
	FocusLost, /*!< \brief the focus has been lost for the app. */
	FocusGained, /*!< \brief the focus has been gained for the app. */
	OrientationChanged, /*!< \brief the orientation of the app has changed. */
    TouchDown, /*!< \brief either a new touch point has been added to the screen. */
	TouchMoved, /*!< \brief one of the touch points has moved. */
	TouchEnded, /*!< \brief one of the touch points has been removed from the screen. */
	KeyboardOpen, /*!< \brief requests the underlying application thread open the keyboard. */
	KeyboardClose, /*!< \brief requests the underlying application thread close the keyboard. */
	KeyboardEditRange, /*!< \brief requests the underlying application thread set the edit range for the internal textbox. */
    CharEvent,
    KeyEvent
};

#define LWMAXEVENTQUEUESIZE 50 /*!< \brief the max number of events that can build up, before they are dropped. */
/*!< \brief the iOS app event to be passed to the window class for processing. */
struct LWIOSEvent {
	uint32_t m_EventCode; /*!< \brief the event code for the iOS event being handled. */
	uint32_t m_TouchCount; /*!< \brief the number of points being touched. */
	LWVector2i m_TouchPoints[LWMAXTOUCHPOINTS]; /*!< \brief the actual points being touched. a maximum of 8 points are supported. */
    uint32_t m_TouchState[LWMAXTOUCHPOINTS]; /*!< \brief the current state of each touch point. */
    float m_TouchSize[LWMAXTOUCHPOINTS]; /*!< \breif the current size of each touch point. */
    
};

/*!< \brief the iOS app context which is a singleton class for managing messages between the application and the iOS environment. */
struct LWIOSAppContext {
	char m_TextBuffer[LWMAXKEYBOARDTEXTBUFFER]; /*!< \brief the text buffer used when opening the keyboard. */
	LWConcurrentFIFO<LWIOSEvent, LWMAXEVENTQUEUESIZE> m_EventLoop; /*!< \brief the main event loop which passes events to the application from the underlying application thread. */
	LWConcurrentFIFO<LWIOSEvent, LWMAXEVENTQUEUESIZE> m_AppLoop; /*!< \briefthe app event loop which passes events from the application to the underlying application thread. */
	CMMotionManager *m_MotionManager; /*!< \brief the motion manager for app sensors. */
	LWUITextField *m_KeyboardTextField; /*!< \brief the keyboard text field used for the internal keyboard. */
	UIApplication *m_Applicaiton; /*!< \brief this is a cached pointer to the main application object. */
	UIWindow *m_Window; /*!< \brief the default window object created when the application is started. */
	std::atomic<uint32_t> m_EventState; /*!< \brief the event state used to synchronize events between the application thread and the iOS environment thread. */
    CGRect m_KeyboardLayout;
	bool m_SyncState; /*!< \brief the sync state for the application, rather to sychronize or not. */
};

/*!< \brief no window context needs to be maintained. */
struct LWWindowContext {};

/*!< \brief the iOS app context. */
extern LWIOSAppContext LWAppContext;

typedef std::thread LWThreadType;

#define LWVIDEO_IMPLEMENTED_OPENGLES2
#endif

#ifdef LWVIDEODRIVER_OPENGLES2_H
#ifndef LWVIDEODRIVER_OPENGLES2_PLATFORM_H
#define LWVIDEODRIVER_OPENGLES2_PLATFORM_H
#import <GLKit/GLKit.h>

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

struct LWOpenGLES2Context {
	GLKView *m_View;
    bool m_ViewChanged;
};
#endif
#endif

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

#define LWSOUND_RESERVEBUFFERSIZE 128*1024 //128KB
#define LWSOUND_RESERVECNT 4

struct LWSoundContext {
	AudioQueueRef m_Queue;
	AudioQueueBufferRef m_QueueBuffers[LWSOUND_RESERVECNT]; //<--this should be the same value as LWSound::ReserveSlices, but there is no decent way to link to that value that i know of. :(
	uint32_t m_ReserveIdx;
};

#endif
#endif
