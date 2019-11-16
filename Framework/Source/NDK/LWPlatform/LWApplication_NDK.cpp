#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWText.h"
#include <android/log.h>
#include <thread>
#include <atomic>
#include <cstring>
#include <iostream>
#include <sstream>

LWNDKAppContext LWAppContext;

void PushEvent(ANativeActivity *Activity, ANativeWindow *Window, AInputQueue *Queue, uint32_t EventCode){
	LWAppContext.m_EventLoop.Push({ Activity, Window, Queue, EventCode });
};

void PushSyncEvent(ANativeActivity *Activity, ANativeWindow *Window, AInputQueue *Queue, uint32_t EventCode, uint32_t Timeout){
	uint32_t Current = LWAppContext.m_EventState.load();
	LWAppContext.m_EventLoop.Push({ Activity, Window, Queue, EventCode });
	uint64_t Start = LWTimer::GetCurrent();
	while (LWAppContext.m_EventState.load() == Current && (LWTimer::GetCurrent() - Start) <= Timeout) std::this_thread::yield();
};

bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData) {
	uint64_t Prev = LWTimer::GetCurrent();
	while (true) {
		uint64_t Curr = LWTimer::GetCurrent();
		if (Curr - Prev >= Frequency) {
			if (!LoopFunc(UserData)) break;
			Prev += Frequency;
		} else std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return true;
}

void NDKFlushOutput(void) {
	const char *Str = LWAppContext.m_CoutStream.str().c_str();
	while (*Str) Str += (__android_log_write(ANDROID_LOG_VERBOSE, "LWApp", Str) - 8);
	LWAppContext.m_CoutStream.str("");
	LWAppContext.m_CoutStream.clear();
	return;
}

void NDKLog(const char *F) {
	while(*F) F+=(__android_log_write(ANDROID_LOG_VERBOSE, "LWApp", F)-8);
	return;
}

void NDKLogf(const char *Fmt, ...) {
	char Buffer[512];
	memset(Buffer, 0, sizeof(Buffer));
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	char *F = Buffer;
	while (*F) F += (__android_log_write(ANDROID_LOG_VERBOSE, "LWApp", F) - 8);
	return;
}


//Support the android entry point here, and then initiate the program's entry point!
void ANativeActivity_onCreate(ANativeActivity *Activity, void *SavedState, size_t SavedStateSize) {
	const uint32_t MaxEventItems = 50;
	const uint32_t MaxWaitSeconds = 2; /*!< \brief we wait a maximum of 2 seconds to verify a state changed has been noticed by the main app. */
	const uint32_t FLAG_FULLSCREEN = 0x400;

	const uint32_t SYSTEM_UI_FLAG_FULLSCREEN = 0x4;
	const uint32_t SYSTEM_UI_FLAG_HIDE_NAVIGATION = 0x2;
	const uint32_t SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN = 0x400;
	const uint32_t SYSTEM_UI_FLAG_IMMERSIVE = 0x800;
	const uint32_t SYSTEM_UI_FLAG_IMMERSIVE_STICKY = 0x1000;

	static bool IsInitiated = false; /*!< \brief boolean checking if the environment has already been constructed. */
	static bool IsRunning = false;

	static jclass ContextClass = 0;
	static jclass PackageManagerClass = 0;
	static jclass PackageInfoClass = 0;
	static jclass StringClass = 0;
	static jclass ApplicationInfoClass = 0;
	static jclass NativeActivityClass = 0;
	static jclass FileClass = 0;
	static jclass WindowClass = 0;
	static jclass ViewClass = 0;

	static jfieldID PackageInfo_applicationInfo = 0;
	static jfieldID ApplcationInfo_dataDir = 0;

	static jmethodID Context_getPackageName = 0;
	static jmethodID Context_getPackageManager = 0;
	static jmethodID PackageManager_getPackageInfo = 0;
	static jmethodID File_getPath = 0;
	static jmethodID NativeActivity_getExternalFilesDir = 0;
	static jmethodID NativeActivity_getWindow = 0;
	static jmethodID Window_addFlags = 0;
	static jmethodID Window_getDecorView = 0;
	static jmethodID View_setSystemUiVisibility = 0;

	
	auto MainApplication = [Activity, SavedState, SavedStateSize]() {
		NDKLog("Starting Main App.");
		if (LWAppContext.m_App->vm->AttachCurrentThread(&LWAppContext.m_AppEnv, nullptr) != JNI_OK) {
			LWAppContext.m_EventState++;
			ANativeActivity_finish(Activity);
			return;
		}
		LWAppContext.m_AppEnvCounter = (LWAppContext.m_AppEnvCounter | 1) + 1;
		static jclass LooperClass = 0;
		static jmethodID Looper_prepare = 0;

		LooperClass = LWAppContext.m_AppEnv->FindClass("android/os/Looper");
		Looper_prepare = LWAppContext.m_AppEnv->GetStaticMethodID(LooperClass, "prepare", "()V");
		LWAppContext.m_AppEnv->CallStaticVoidMethod(LooperClass, Looper_prepare);

		//Redirect STDOUT to logcat!
		std::streambuf *oCout = std::cout.rdbuf();

		std::cout.rdbuf(LWAppContext.m_CoutStream.rdbuf());
		LWAppContext.m_EventState++;
		LWMain(0, nullptr);


		LWAppContext.m_App->vm->DetachCurrentThread();
		ANativeActivity_finish(Activity);
		IsRunning = false;
		LWAppContext.m_EventState++;
		NDKFlushOutput();
		std::cout.rdbuf(oCout);
		NDKLog("Finished.");
		return;
	};

	LWAppContext.m_App = Activity;
	LWAppContext.m_SavedState = SavedState;
	LWAppContext.m_SaveSateSize = SavedStateSize;
	LWNDKEvent Ev;
	while (LWAppContext.m_EventLoop.Length()) LWAppContext.m_EventLoop.Pop(Ev);

	Activity->callbacks->onDestroy = [](ANativeActivity *Activity) {
		PushSyncEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::Destroy, MaxWaitSeconds*LWTimer::GetResolution());
	};
	Activity->callbacks->onConfigurationChanged = [](ANativeActivity *Activity) { 
		PushEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::ConfigurationChanged); 
	};
	Activity->callbacks->onInputQueueCreated = [](ANativeActivity *Activity, AInputQueue *Queue) { PushEvent(Activity, nullptr, Queue, (uint32_t)LWNDKEventCode::InputQueueCreated); };
	Activity->callbacks->onInputQueueDestroyed = [](ANativeActivity *Activity, AInputQueue *Queue) { PushSyncEvent(Activity, nullptr, Queue, (uint32_t)LWNDKEventCode::InputQueueDestroyed, MaxWaitSeconds*LWTimer::GetResolution()); };
	Activity->callbacks->onNativeWindowCreated = [](ANativeActivity *Activity, ANativeWindow *Window) { PushEvent(Activity, Window, nullptr, (uint32_t)LWNDKEventCode::WindowCreated); };
	Activity->callbacks->onNativeWindowDestroyed = [](ANativeActivity *Activity, ANativeWindow *Window) { PushSyncEvent(Activity, Window, nullptr, (uint32_t)LWNDKEventCode::WindowDestroyed, MaxWaitSeconds*LWTimer::GetResolution());  };
	Activity->callbacks->onNativeWindowResized = [](ANativeActivity *Activity, ANativeWindow *Window) { PushEvent(Activity, Window, nullptr, (uint32_t)LWNDKEventCode::WindowResized); };
	Activity->callbacks->onPause = [](ANativeActivity *Activity) { PushEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::Pause); };
	Activity->callbacks->onResume = [](ANativeActivity *Activity) { 
		PushEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::Resume); 

		jobject Wnd = Activity->env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
		jobject View = Activity->env->CallObjectMethod(Wnd, Window_getDecorView);

		Activity->env->CallVoidMethod(Wnd, Window_addFlags, FLAG_FULLSCREEN);
		Activity->env->CallVoidMethod(View, View_setSystemUiVisibility, SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		Activity->env->DeleteLocalRef(View);
		Activity->env->DeleteLocalRef(Wnd);
	
	};
	Activity->callbacks->onStart = [](ANativeActivity *Activity) { 
		PushEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::Start); 

		jobject Wnd = Activity->env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
		jobject View = Activity->env->CallObjectMethod(Wnd, Window_getDecorView);

		Activity->env->CallVoidMethod(Wnd, Window_addFlags, FLAG_FULLSCREEN);
		Activity->env->CallVoidMethod(View, View_setSystemUiVisibility, SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		Activity->env->DeleteLocalRef(View);
		Activity->env->DeleteLocalRef(Wnd);
	};
	Activity->callbacks->onStop = [](ANativeActivity *Activity) { PushEvent(Activity, nullptr, nullptr, (uint32_t)LWNDKEventCode::Stop); };
	Activity->callbacks->onWindowFocusChanged = [](ANativeActivity *Activity, int Focused) {
		PushEvent(Activity, nullptr, nullptr, (uint32_t)(Focused ? LWNDKEventCode::FocusGained : LWNDKEventCode::FocusLost));
	};
	if (!IsInitiated) {
		IsInitiated = true;
		LWAppContext.m_EventState.store(0);

		//Find our working directory.
		ContextClass = Activity->env->FindClass("android/content/Context");
		PackageManagerClass = Activity->env->FindClass("android/content/pm/PackageManager");
		PackageInfoClass = Activity->env->FindClass("android/content/pm/PackageInfo");
		StringClass = Activity->env->FindClass("java/lang/String");
		ApplicationInfoClass = Activity->env->FindClass("android/content/pm/ApplicationInfo");
		NativeActivityClass = Activity->env->FindClass("android/app/NativeActivity");
		FileClass = Activity->env->FindClass("java/io/File");
		WindowClass = Activity->env->FindClass("android/view/Window");
		ViewClass = Activity->env->FindClass("android/view/View");

		PackageInfo_applicationInfo = Activity->env->GetFieldID(PackageInfoClass, "applicationInfo", "Landroid/content/pm/ApplicationInfo;");
		ApplcationInfo_dataDir = Activity->env->GetFieldID(ApplicationInfoClass, "dataDir", "Ljava/lang/String;");

		Context_getPackageName = Activity->env->GetMethodID(ContextClass, "getPackageName", "()Ljava/lang/String;");
		Context_getPackageManager = Activity->env->GetMethodID(ContextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
		PackageManager_getPackageInfo = Activity->env->GetMethodID(PackageManagerClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
		File_getPath = Activity->env->GetMethodID(FileClass, "getPath", "()Ljava/lang/String;");
		NativeActivity_getExternalFilesDir = Activity->env->GetMethodID(NativeActivityClass, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
		NativeActivity_getWindow = Activity->env->GetMethodID(NativeActivityClass, "getWindow", "()Landroid/view/Window;");
		Window_addFlags = Activity->env->GetMethodID(WindowClass, "addFlags", "(I)V");
		Window_getDecorView = Activity->env->GetMethodID(WindowClass, "getDecorView", "()Landroid/view/View;");
		View_setSystemUiVisibility = Activity->env->GetMethodID(ViewClass, "setSystemUiVisibility", "(I)V");

		jobject PackName = Activity->env->CallObjectMethod(Activity->clazz, Context_getPackageName);
		jobject PackMan = Activity->env->CallObjectMethod(Activity->clazz, Context_getPackageManager);
		jobject PackInfo = Activity->env->CallObjectMethod(PackMan, PackageManager_getPackageInfo, PackName, 0);
		jobject AppInfo = Activity->env->GetObjectField(PackInfo, PackageInfo_applicationInfo);
		jobject DataDir = Activity->env->GetObjectField(AppInfo, ApplcationInfo_dataDir);
		//jobject DDir = Activity->env->CallObjectMethod(Activity->clazz, NativeActivity_getExternalFilesDir, nullptr);
		//jobject DataDir = Activity->env->CallObjectMethod(DDir, File_getPath);
		
		const char *DataDirBuffer = Activity->env->GetStringUTFChars((jstring)DataDir, nullptr);
		uint32_t Length = ((uint32_t)std::strlen(DataDirBuffer)) + 1;
		std::memcpy(LWAppContext.m_AppDirectory, DataDirBuffer, sizeof(char)*std::min<uint32_t>(sizeof(LWAppContext.m_AppDirectory), Length));
		Activity->env->ReleaseStringUTFChars((jstring)DataDir, DataDirBuffer);
	}
	jobject Wnd = Activity->env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
	jobject View = Activity->env->CallObjectMethod(Wnd, Window_getDecorView);

	Activity->env->CallVoidMethod(Wnd, Window_addFlags, FLAG_FULLSCREEN);
	Activity->env->CallVoidMethod(View, View_setSystemUiVisibility, SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_HIDE_NAVIGATION | SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
	Activity->env->DeleteLocalRef(View);
	Activity->env->DeleteLocalRef(Wnd);
	if (!IsRunning) {
		NDKLog("Creating app"); 
		
		//Start main application!
		std::thread Thread(MainApplication);
		Thread.detach();

		while (LWAppContext.m_EventState.load() == 0) std::this_thread::yield();
		IsRunning = true;
	}
	return;
}

bool LWExecute(const char *BinaryPath, const char *Parameters) {
	return false;
}

bool LWEmail(const char *SrcEmail, const char *TargetEmail, const char *Subject, const char *Body, const char *SMTPServer, const char *SMTPUsername, const char *SMTPPassword){
	return false;
}

bool LWBrowser(const char *URL) {
	return false;
}

float LWSystemScale(void) {
	return 1.0f;
}