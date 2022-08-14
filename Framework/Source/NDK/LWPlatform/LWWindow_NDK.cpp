#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include "LWCore/LWTimer.h"
#include <Android/log.h>
#include <Android/configuration.h>
#include <Android/sensor.h>
#include <Android/looper.h>
#include <thread>
#include <atomic>
#include <iostream>

const uint32_t LWWindow::Terminate; /*!< \brief the window has requested for a termination */
const uint32_t LWWindow::Visible; /*!< \brief the window is visible. */
const uint32_t LWWindow::Focused; /*!< \brief the window is focused or not. */
const uint32_t LWWindow::MouseVisible; /*!< \brief rather or not the mouse is to be considered visible or not. */
const uint32_t LWWindow::PosChanged; /*!< \brief the window position has been changed. */
const uint32_t LWWindow::SizeChanged; /*!< \brief the window size has been changed. */
const uint32_t LWWindow::FocusChanged; /*!< \brief the window focus has been changed. */
const uint32_t LWWindow::Error; /*!< \brief the window had an error during creation. */
const uint32_t LWWindow::Fullscreen; /*!< \brief the window is designed to be in fullscreen mode. note that for DX11 the display mode does not need to be changed if this flag is set. */
const uint32_t LWWindow::Borderless; /*!< \brief the window is designed to be a borderless window. */
const uint32_t LWWindow::OrientationChanged; /*!< \brief signals that the window orientation has changed. */
const uint32_t LWWindow::KeyboardPresent; /*!< \brief the windows softward keyboard is present, this flag is only viable for iOS and Android OS's. */
const uint32_t LWWindow::Rotation_0; /*!< \brief the current window is in default portrait mode, and is not rotated at all. */
const uint32_t LWWindow::Rotation_90; /*!< \brief the current window is orientated in landscape mode. */
const uint32_t LWWindow::Rotation_180; /*!< \brief the current window is orientated in upside down portrait mode. */
const uint32_t LWWindow::Rotation_270; /*!< \brief the current window is orientated in upside down landscape mode. */

const uint32_t LWWindow::MouseDevice; /*!< \brief flag to indicate window should create a mouse device if applicable to the platform(windows, mac, linux). */
const uint32_t LWWindow::KeyboardDevice; /*!< \brief flag to indicate window should create a keyboard device if applicable to the platform. (windows, mac, linux, android, iOS) */
const uint32_t LWWindow::GamepadDevice; /*!< \brief flag to indicate window should create the gamepad devices if applicable to the platform. (windows). */
const uint32_t LWWindow::TouchDevice; /*!< \brief flag to indicate window should create the touchscreen device if applicable to the platform. (android, iOS). */
const uint32_t LWWindow::GyroscopeDevice; /*!< \brief flag to indicate window should create and activate the gyroscope device if applicable to the platform. (android, iOS). */
const uint32_t LWWindow::AccelerometerDevice; /*!< \brief flag to indicate window should create and activate the accelerometer device if applicable to the platform. (android, IOS). */

const uint32_t LWWindow::DialogOK; /*!< \brief provides the dialog OK button when creating a dialog. */
const uint32_t LWWindow::DialogYES; /*!< \brief provides the dialog an yes and no button when creating a dialog. */
const uint32_t LWWindow::DialogNo; /*!< \brief return value only on what MakeDialog creates. */
const uint32_t LWWindow::DialogCancel; /*!< \brief provides the dialog an cancel button when creating a dialog. */


uint32_t LWWindow::MakeDialog(const LWUTF8Iterator &Text, const LWUTF8Iterator &Header, uint32_t DialogFlags){
	LWLogEvent<256>("Dialog: {}: {}\n", Header, Text);
	NDKFlushOutput();
	return 0;
}

bool LWWindow::MakeSaveFileDialog(const LWUTF8Iterator &Filter, char8_t *Buffer, uint32_t BufferLen) {
	return false;
}

bool LWWindow::MakeLoadFileDialog(const LWUTF8Iterator &Filter, char8_t *Buffer, uint32_t BufferLen) {
	return false;
}

uint32_t LWWindow::MakeLoadFileMultipleDialog(const LWUTF8Iterator &Filter, char8_t **Bufer, uint32_t BufferLen, uint32_t BufferCount) {
	return 0;
}

bool LWWindow::WriteClipboardText(const LWUTF8Iterator &Text) {
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass WindowManagerClass = 0;
	static jclass ContextClass = 0;
	static jclass ClipboardManager = 0;
	static jclass ClipData = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID ClipData_newPlainText = 0;
	static jmethodID ClipboardManager_setPrimaryClip = 0;
	if (EnvCtr != LWAppContext.m_AppEnvCounter) {
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		ContextClass = Env->FindClass("android/content/Context");
		ClipboardManager = Env->FindClass("android/content/ClipboardManager");
		ClipData = Env->FindClass("android/content/ClipData");

		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		ClipData_newPlainText = Env->GetStaticMethodID(ClipData, "newPlainText", "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;");
		ClipboardManager_setPrimaryClip = Env->GetMethodID(ClipboardManager, "setPrimaryClip", "(Landroid/content/ClipData;)V");
	}
	jstring Lbl = Env->NewStringUTF("Copied Text");
	jstring Val = Env->NewStringUTF(*Text.c_str<256>());
	jstring clips = Env->NewStringUTF("clipboard");
	jobject ClipManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, clips);
	jobject Clip = Env->CallStaticObjectMethod(ClipData, ClipData_newPlainText, Lbl, Val);
	Env->CallVoidMethod(ClipManager, ClipboardManager_setPrimaryClip, Clip);
	Env->DeleteLocalRef(Clip);
	Env->DeleteLocalRef(ClipManager);
	Env->DeleteLocalRef(clips);
	Env->DeleteLocalRef(Val);
	Env->DeleteLocalRef(Lbl);
	return true;
}

uint32_t LWWindow::ReadClipboardText(char8_t *Buffer, uint32_t BufferLen) {
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass WindowManagerClass = 0;
	static jclass ContextClass = 0;
	static jclass ClipboardManager = 0;
	static jclass ClipData = 0;
	static jclass ClipDataItem = 0;
	static jclass CharSequence = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID ClipboardManager_hasPrimaryClip = 0;
	static jmethodID ClipboardManager_getPrimaryClip = 0;
	static jmethodID ClipData_getItemAt = 0;
	static jmethodID ClipDataItem_coerceToText = 0;
	static jmethodID CharSequence_toString = 0;
	if (EnvCtr != LWAppContext.m_AppEnvCounter) {
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		ContextClass = Env->FindClass("android/content/Context");
		ClipboardManager = Env->FindClass("android/content/ClipboardManager");
		ClipData = Env->FindClass("android/content/ClipData");
		ClipDataItem = Env->FindClass("android/content/ClipData$Item");
		CharSequence = Env->FindClass("java/lang/CharSequence");

		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		ClipboardManager_hasPrimaryClip = Env->GetMethodID(ClipboardManager, "hasPrimaryClip", "()Z");
		ClipboardManager_getPrimaryClip = Env->GetMethodID(ClipboardManager, "getPrimaryClip", "()Landroid/content/ClipData;");
		ClipData_getItemAt = Env->GetMethodID(ClipData, "getItemAt", "(I)Landroid/content/ClipData$Item;");
		ClipDataItem_coerceToText = Env->GetMethodID(ClipDataItem, "coerceToText", "(Landroid/content/Context;)Ljava/lang/CharSequence;");
		CharSequence_toString = Env->GetMethodID(CharSequence, "toString", "()Ljava/lang/String;");
	}
	uint32_t Res = 0;
	jstring SystemService = Env->NewStringUTF("clipboard");
	jobject ClipManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, SystemService);
	jboolean HasClip = Env->CallBooleanMethod(ClipManager, ClipboardManager_hasPrimaryClip);
	if (HasClip) {
		jobject Clip = Env->CallObjectMethod(ClipManager, ClipboardManager_getPrimaryClip);
		jobject CItem = Env->CallObjectMethod(Clip, ClipData_getItemAt, 0);
		if (CItem != nullptr) {
			jobject TextObj = Env->CallObjectMethod(CItem, ClipDataItem_coerceToText, LWAppContext.m_App->clazz);
			jstring Text = (jstring)Env->CallObjectMethod(TextObj, CharSequence_toString);
			const char *ts = Env->GetStringUTFChars(Text, nullptr);
			if (ts) {
				strlcpy(Buffer, ts, BufferLen);
				Res = strlen(ts)+1;
			}
			Env->DeleteLocalRef(Text);
			Env->DeleteLocalRef(TextObj);
		}
		Env->DeleteLocalRef(CItem);
		Env->DeleteLocalRef(Clip);
	}
	Env->DeleteLocalRef(ClipManager);
	Env->DeleteLocalRef(SystemService);
	return Res;
}

LWWindow &LWWindow::SetTitle(const LWUTF8Iterator &Title){
	m_Title = Title;
	return *this;
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position){
	return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size){
	return *this;
}

LWWindow &LWWindow::SetVisible(bool isVisible){
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused){
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool isFullscreen) {
	return *this;
}

LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position){
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool isVisible){
	return *this;
}

LWWindow &LWWindow::SetActiveGamepad(LWGamePad *Gamepad) {
	m_ActiveGamepad = Gamepad;
	return *this;
}

LWInputDevice *LWWindow::AttachInputDevice(LWInputDevice *Device){
	Device->SetNext(m_FirstDevice);
	m_FirstDevice = Device;
	return Device;
}

LWWindow &LWWindow::OpenKeyboard(uint32_t KeyboardType){
	//ANativeActivity_showSoftInput(LWAppContext.m_App, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
	
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass InputMethodManagerClass = 0;
	static jclass ContextClass = 0;
	static jclass NativeActivityClass = 0;
	static jclass WindowClass = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID InputMethodManager_showSoftInput = 0;
	static jmethodID NativeActivity_getWindow = 0;
	static jmethodID Window_getDecorView = 0;
	if(EnvCtr!=LWAppContext.m_AppEnvCounter){
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		ContextClass = Env->FindClass("android/content/Context");
		InputMethodManagerClass = Env->FindClass("android/view/inputmethod/InputMethodManager");
		NativeActivityClass = Env->GetObjectClass(LWAppContext.m_App->clazz);
		WindowClass = Env->FindClass("android/view/Window");


		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		InputMethodManager_showSoftInput = Env->GetMethodID(InputMethodManagerClass, "showSoftInput", "(Landroid/view/View;I)Z");
		NativeActivity_getWindow = Env->GetMethodID(NativeActivityClass, "getWindow", "()Landroid/view/Window;");
		Window_getDecorView = Env->GetMethodID(WindowClass, "getDecorView", "()Landroid/view/View;");
	}
	jstring InputStr = Env->NewStringUTF("input_method");
	jobject InputMethodManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, InputStr);
	jobject Window = Env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
	jobject View = Env->CallObjectMethod(Window, Window_getDecorView);
	Env->CallBooleanMethod(InputMethodManager, InputMethodManager_showSoftInput, View, 0);
	Env->DeleteLocalRef(View);
	Env->DeleteLocalRef(Window);
	Env->DeleteLocalRef(InputMethodManager);
	Env->DeleteLocalRef(InputStr);
	m_Flag |= KeyboardPresent;
	return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t CursorPosition, uint32_t EditSize){
	return *this;
}

LWWindow &LWWindow::CloseKeyboard(void){
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass InputMethodManagerClass = 0;
	static jclass ContextClass = 0;
	static jclass NativeActivityClass = 0;
	static jclass WindowClass = 0;
	static jclass ViewClass = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID InputMethodManager_hideSoftInputFromWindow = 0;
	static jmethodID NativeActivity_getWindow = 0;
	static jmethodID Window_getDecorView = 0;
	static jmethodID View_getWindowToken = 0;
	if(EnvCtr!=LWAppContext.m_AppEnvCounter){
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		ContextClass = Env->FindClass("android/content/Context");
		InputMethodManagerClass = Env->FindClass("android/view/inputmethod/InputMethodManager");
		NativeActivityClass = Env->GetObjectClass(LWAppContext.m_App->clazz);
		WindowClass = Env->FindClass("android/view/Window");
		ViewClass = Env->FindClass("android/view/View");

		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		InputMethodManager_hideSoftInputFromWindow = Env->GetMethodID(InputMethodManagerClass, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
		NativeActivity_getWindow = Env->GetMethodID(NativeActivityClass, "getWindow", "()Landroid/view/Window;");
		Window_getDecorView = Env->GetMethodID(WindowClass, "getDecorView", "()Landroid/view/View;");
		View_getWindowToken = Env->GetMethodID(ViewClass, "getWindowToken", "()Landroid/os/IBinder;");

	}
	jstring InputStr = Env->NewStringUTF("input_method");
	jobject InputMethodManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, InputStr);
	jobject Window = Env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
	jobject View = Env->CallObjectMethod(Window, Window_getDecorView);
	jobject Binder = Env->CallObjectMethod(View, View_getWindowToken);
	Env->CallBooleanMethod(InputMethodManager, InputMethodManager_hideSoftInputFromWindow, Binder, 0);
	Env->DeleteLocalRef(Binder);
	Env->DeleteLocalRef(View);
	Env->DeleteLocalRef(Window);
	Env->DeleteLocalRef(InputMethodManager);
	Env->DeleteLocalRef(InputStr);
	m_Flag &= ~KeyboardPresent;
	return *this;
}

LWWindow &LWWindow::SetKeyboardText(const LWUTF8Iterator &Text) {
	return *this;
}

LWVector4f LWWindow::GetKeyboardLayout(void) {
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass NativeActivityClass = 0;
	static jclass WindowClass = 0;
	static jclass ViewClass = 0;
	static jclass RectClass = 0;

	static jmethodID NativeActivity_getWindow = 0;
	static jmethodID Window_getDecorView = 0;
	static jmethodID View_getWindowVisibleDisplayFrame = 0;
	static jfieldID Rect_top = 0;
	static jfieldID Rect_bottom = 0;
	static jmethodID Rect_Constructor = 0;

	static jobject Window = 0;
	static jobject View = 0;
	static jobject RectObj = 0;
	static LWVector4f KeyboardSize = LWVector4f();
	if(EnvCtr!=LWAppContext.m_AppEnvCounter){
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		NativeActivityClass = Env->GetObjectClass(LWAppContext.m_App->clazz);
		RectClass = Env->FindClass("android/graphics/Rect");
		WindowClass = Env->FindClass("android/view/Window");
		ViewClass = Env->FindClass("android/view/View");

		NativeActivity_getWindow = Env->GetMethodID(NativeActivityClass, "getWindow", "()Landroid/view/Window;");
		Window_getDecorView = Env->GetMethodID(WindowClass, "getDecorView", "()Landroid/view/View;");
		View_getWindowVisibleDisplayFrame = Env->GetMethodID(ViewClass, "getWindowVisibleDisplayFrame", "(Landroid/graphics/Rect;)V");
		Rect_Constructor = Env->GetMethodID(RectClass, "<init>", "()V");

		Rect_top = Env->GetFieldID(RectClass, "top", "I");
		Rect_bottom = Env->GetFieldID(RectClass, "bottom", "I");

	}
	Window = Env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
	View = Env->CallObjectMethod(Window, Window_getDecorView);
		
	RectObj = Env->NewObject(RectClass, Rect_Constructor);
	Env->CallVoidMethod(View, View_getWindowVisibleDisplayFrame, RectObj);
	jint RTop = Env->GetIntField(RectObj, Rect_top);
	jint RBtm = Env->GetIntField(RectObj, Rect_bottom);
	Env->DeleteLocalRef(RectObj);
	Env->DeleteLocalRef(Window);
	Env->DeleteLocalRef(View);
	KeyboardSize.w = m_Size.y - (RBtm-RTop);
	return KeyboardSize;
}

uint32_t LWWindow::GetKeyboardType(void) {
	return 0;
}

LWWindow &LWWindow::GetKeyboardEditRange(uint32_t &CursorPosition, uint32_t &EditSize){
	return *this;
}

uint32_t LWWindow::GetKeyboardText(char *Buffer, uint32_t BufferLen){
	if (Buffer) *Buffer = '\0';
	return 0;
}

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime){
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass NativeActivityClass = 0;
	static jclass DisplayClass = 0;
	static jclass WindowManagerClass = 0;
	static jclass WindowClass = 0;
	static jclass ContextClass = 0;

	static jmethodID NativeActivity_getWindow = 0;
	static jmethodID Context_GetSystemService = 0;
	static jmethodID WindowManager_GetDefaultDisplay = 0;
	static jmethodID Display_getRotation = 0;

	static bool FirstSize = false;
	static bool PortraitIsLandscape = false;
	static bool HasWindow = false;

	const uint32_t Rotate_0 = 0;
	const uint32_t Rotate_90 = 1;
	const uint32_t Rotate_180 = 2;
	const uint32_t Rotate_270 = 3;

	if(EnvCtr!=LWAppContext.m_AppEnvCounter){
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		NativeActivityClass = Env->GetObjectClass(LWAppContext.m_App->clazz);
		DisplayClass = Env->FindClass("android/view/Display");
		WindowManagerClass = Env->FindClass("android/view/WindowManager");
		ContextClass = Env->FindClass("android/content/Context");
		WindowClass = Env->FindClass("android/view/Window");
		
		NativeActivity_getWindow = Env->GetMethodID(NativeActivityClass, "getWindow", "()Landroid/view/Window;");
		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		WindowManager_GetDefaultDisplay = Env->GetMethodID(WindowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;");
		Display_getRotation = Env->GetMethodID(DisplayClass, "getRotation", "()I");
		
	}

	LWNDKEvent *Event = (LWNDKEvent*)MessageData;
	if(Message==(uint32_t)LWNDKEventCode::Destroy){
		LWLogEvent("Termination requested!\n");
		m_Flag |= Terminate; //Since the app terminating also causes the event state to get incremented, we don't have to specify our syncing here.
		return true;
	}else if(Message==(uint32_t)LWNDKEventCode::InputQueueCreated){
		m_Context.m_InputQueue = Event->m_InputQueue;
		return true;
	}else if(Message==(uint32_t)LWNDKEventCode::WindowDestroyed){
		m_Context.m_ActiveWindow = nullptr;
		m_Context.m_SyncState = true; //We need to sync the state for the application thread.
		HasWindow = false;
		m_Flag |= SizeChanged;
		return true;
	}else if(Message==(uint32_t)LWNDKEventCode::InputQueueDestroyed){
		m_Context.m_InputQueue = nullptr;
		m_Context.m_SyncState = true; //We need to sync the state for the application thread.
		return true;
	}else if(Message==(uint32_t)LWNDKEventCode::FocusLost || (Message==(uint32_t)LWNDKEventCode::Pause)){
		m_Flag = (m_Flag&(~Focused)) | FocusChanged;
		if (m_GyroscopeDevice && m_GyroscopeDevice->IsEnabled()) {
			const ASensor *Gyro = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_GYROSCOPE);
			if (Gyro) ASensorEventQueue_disableSensor(m_Context.m_SensorQueue, Gyro);
		}
		if (m_AccelerometerDevice && m_AccelerometerDevice->IsEnabled()) {
			const ASensor *Accel = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
			if (Accel) ASensorEventQueue_disableSensor(m_Context.m_SensorQueue, Accel);
		}
		return true;
	}else if (Message == (uint32_t)LWNDKEventCode::FocusGained || (Message == (uint32_t)LWNDKEventCode::Resume)){
		m_Flag |= Focused | FocusChanged;
		if (m_GyroscopeDevice && m_GyroscopeDevice->IsEnabled()) {
			const ASensor *Gyro = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_GYROSCOPE);
			if (Gyro) ASensorEventQueue_enableSensor(m_Context.m_SensorQueue, Gyro);
		}
		if (m_AccelerometerDevice && m_AccelerometerDevice->IsEnabled()) {
			const ASensor *Accel = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
			if (Accel) ASensorEventQueue_enableSensor(m_Context.m_SensorQueue, Accel);
		}
		return true;
	} else if (Message == (uint32_t)LWNDKEventCode::ConfigurationChanged || Message == (uint32_t)LWNDKEventCode::WindowResized || Message == (uint32_t)LWNDKEventCode::WindowCreated) {
		if (Message == LWNDKEventCode::WindowCreated) HasWindow = true;
		if (!HasWindow) return true;
		jstring WndString = Env->NewStringUTF("window");
		jobject WindowManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, WndString);
		jobject DefaultDisplay = Env->CallObjectMethod(WindowManager, WindowManager_GetDefaultDisplay);
		jobject Window = Env->CallObjectMethod(LWAppContext.m_App->clazz, NativeActivity_getWindow);
		jint Rotation = Env->CallIntMethod(DefaultDisplay, Display_getRotation);
		Env->DeleteLocalRef(Window);
		Env->DeleteLocalRef(DefaultDisplay);
		Env->DeleteLocalRef(WindowManager);
		Env->DeleteLocalRef(WndString);
		int32_t pWidth = m_Size.x; 
		int32_t pHeight = m_Size.y;
		int32_t Width = m_Size.x;
		int32_t Height = m_Size.y;
		if (Event && Event->m_Window) m_Context.m_ActiveWindow = Event->m_Window;
		if (m_Context.m_ActiveWindow) {
			Width = ANativeWindow_getWidth(m_Context.m_ActiveWindow);
			Height = ANativeWindow_getHeight(m_Context.m_ActiveWindow);
			if (!FirstSize) {
				FirstSize = true;
				if (Width > Height && (Rotation == Rotate_0 || Rotation == Rotate_180)) PortraitIsLandscape = true;
				if (Width < Height && (Rotation == Rotate_90 || Rotation == Rotate_270)) PortraitIsLandscape = true;
			}
		}
		int32_t Longest = std::max<int32_t>(Width, Height);
		int32_t Shortest = std::min<int32_t>(Width, Height);
		int32_t NewRotation = (((uint32_t)Rotation == Rotate_0) ? Rotation_0 : ((uint32_t)Rotation == Rotate_90) ? Rotation_90 : ((uint32_t)Rotation == Rotate_180) ? Rotation_180 : Rotation_270);
		m_Size = (Rotation == Rotate_0 || Rotation == Rotate_180) ? LWVector2i(Shortest, Longest) : LWVector2i(Longest, Shortest);
		if (m_Size.x != pWidth || m_Size.y != pHeight) m_Flag |= SizeChanged;
		if (NewRotation != (m_Flag&(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270))) m_Flag |= OrientationChanged;
		if (PortraitIsLandscape) std::swap(m_Size.x, m_Size.y);
		m_Flag = (m_Flag&~(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270)) | NewRotation;
		m_Context.m_LastConfigCheck = lCurrentTime;
		return true;
	}

	return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime) {
	const uint64_t ConfigChangeCheckFreq = LWTimer::GetResolution();
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);
	if(m_Context.m_SyncState){
		LWAppContext.m_EventState++; //Increment the state since the rest of the application should have been able to process the event change by now.
		m_Context.m_SyncState = false;
	}
	LWNDKEvent Event;
	for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);
	ANativeWindow *CurrWindow = m_Context.m_ActiveWindow;
	while ((m_Flag&Terminate) == 0 && LWAppContext.m_EventLoop.Pop(Event)) {
		ProcessWindowMessage(Event.m_EventCode, &Event, lCurrentTime);
	}
	if (lCurrentTime - m_Context.m_LastConfigCheck >= ConfigChangeCheckFreq) ProcessWindowMessage(LWNDKEventCode::ConfigurationChanged, nullptr, lCurrentTime);
	if (!CurrWindow && m_Context.m_ActiveWindow) LWAppContext.m_EventLoop.Push({ nullptr, nullptr, nullptr, LWNDKEventCode::WindowResized });
	
	if((m_Flag&Terminate)==0 && m_Context.m_InputQueue){
		AInputEvent *IEvent = nullptr;
		while (AInputQueue_getEvent(m_Context.m_InputQueue, &IEvent) >= 0) {
			if (AInputQueue_preDispatchEvent(m_Context.m_InputQueue, IEvent) != 0) {
				LWLogEvent("preDispatched!");
				continue;
			}
			bool Handled = false;
			for (LWInputDevice *Device = m_FirstDevice; Device && !Handled; Device = Device->GetNext()) Handled = Device->ProcessSystemMessage(0, IEvent, lCurrentTime, this);
			if (!Handled) LWLogEvent("Input event not handled.");
			AInputQueue_finishEvent(m_Context.m_InputQueue, IEvent, Handled);
		}
	}
	if ((m_Flag&Terminate) == 0 && m_Context.m_SensorQueue) {
		ASensorEvent SEvent;
		while (ASensorEventQueue_getEvents(m_Context.m_SensorQueue, &SEvent, 1) > 0) {
			bool Handled = false;
			for (LWInputDevice *Device = m_FirstDevice; Device && !Handled; Device = Device->GetNext()) Handled = Device->ProcessSystemMessage(1, &SEvent, lCurrentTime, this);
		}	

	}
	NDKFlushOutput();
	return *this;
}

LWInputDevice *LWWindow::GetInputDevices(void) {
	return m_FirstDevice;
}

LWMouse *LWWindow::GetMouseDevice(void) {
	return m_MouseDevice;
}

LWKeyboard *LWWindow::GetKeyboardDevice(void) {
	return m_KeyboardDevice;
}

LWTouch *LWWindow::GetTouchDevice(void) {
	return m_TouchDevice;
}

LWAccelerometer *LWWindow::GetAccelerometerDevice(void) {
	return m_AccelerometerDevice;
}

LWGyroscope *LWWindow::GetGyroscopeDevice(void) {
	return m_GyroscopeDevice;
}

LWGamePad *LWWindow::GetGamepadDevice(uint32_t i) {
	return m_GamepadDevice[i];
}

LWGamePad *LWWindow::GetActiveGamepadDevice(void) {
	return m_ActiveGamepad;
}

const LWUTF8 &LWWindow::GetTitle(void) const{
	return m_Title;
}

const LWUTF8 &LWWindow::GetName(void) const{
	return m_Name;
}

LWVector2i LWWindow::GetPosition(void) const {
	return m_Position;
}

LWVector2f LWWindow::GetPositionf(void) const {
	return LWVector2f((float)m_Position.x, (float)m_Position.y);
}

LWVector2i LWWindow::GetSize(void) const {
	return m_Size;
}

LWVector2f LWWindow::GetSizef(void) const {
	return LWVector2f((float)m_Size.x, (float)m_Size.y);
}

float LWWindow::GetAspect(void) const {
	return (float)m_Size.x / (float)m_Size.y;
}

uint32_t LWWindow::GetFlag(void) const{
	return m_Flag;
}

uint32_t LWWindow::GetOrientation(void) const{
	return m_Flag&(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270);
}

LWWindowContext &LWWindow::GetContext(void) {
	return m_Context;
}

LWAllocator *LWWindow::GetAllocator(void) const{
	return m_Allocator;
}

bool LWWindow::isFinished(void) const {
	return (m_Flag&Terminate) != 0;
}

bool LWWindow::isFocused(void) const {
	return (m_Flag&Focused) != 0;
}

bool LWWindow::SizeUpdated(void) const {
	return (m_Flag&SizeChanged) != 0;
}

bool LWWindow::PositionUpdated(void) const {
	return (m_Flag&PosChanged) != 0;
}

bool LWWindow::FocusUpdated(void) const {
	return (m_Flag&FocusChanged) != 0;
}

bool LWWindow::isVisible(void) const {
	return (m_Flag&Visible) != 0;
}

bool LWWindow::DidError(void) const {
	return (m_Flag & Error) != 0;
}

bool LWWindow::isVirtualKeyboardPresent(void) const {
	return (m_Flag&KeyboardPresent) != 0;
}

LWWindow::LWWindow(const LWUTF8Iterator &Title, const LWUTF8Iterator &Name, LWAllocator &Allocator, uint32_t Flag, const LWVector2i &Position, const LWVector2i &Size) : m_Allocator(&Allocator), m_FirstDevice(nullptr), m_Title(Title, Allocator), m_Name(Name, Allocator), m_Position(Position), m_Size(Size), m_Flag(Flag){
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	static jclass ContextClass = 0;
	static jclass SensorManagerClass = 0;
	static jclass SensorClass = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID SensorManager_getDefaultSensor = 0;
	static jmethodID Sensor_getMaximumRange = 0;

	if (EnvCtr != LWAppContext.m_AppEnvCounter) {
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;

		ContextClass = Env->FindClass("android/content/Context");
		SensorManagerClass = Env->FindClass("android/hardware/SensorManager");
		SensorClass = Env->FindClass("android/hardware/Sensor");

		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		SensorManager_getDefaultSensor = Env->GetMethodID(SensorManagerClass, "getDefaultSensor", "(I)Landroid/hardware/Sensor;");
		Sensor_getMaximumRange = Env->GetMethodID(SensorClass, "getMaximumRange", "()F");
	}
	
	m_MouseDevice = nullptr;
	m_KeyboardDevice = nullptr;
	m_TouchDevice = nullptr;
	m_AccelerometerDevice = nullptr;
	m_GyroscopeDevice = nullptr;
	m_ActiveGamepad = nullptr;
	memset(m_GamepadDevice, 0, sizeof(m_GamepadDevice));
	m_Context.m_ActiveWindow = nullptr;
	m_Context.m_InputQueue = nullptr;
	m_Context.m_SyncState = false;

	jstring SensorString = Env->NewStringUTF("sensor");
	jobject SensorManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, SensorString);


	m_Context.m_SensorManager = ASensorManager_getInstance();
	m_Context.m_SensorQueue = ASensorManager_createEventQueue(m_Context.m_SensorManager, ALooper_forThread(), 0, nullptr, nullptr);
	m_Context.m_LastConfigCheck = 0;

	if (m_Flag&KeyboardDevice) m_KeyboardDevice = AttachInputDevice(m_Allocator->Create<LWKeyboard>())->AsKeyboard();
	if (m_Flag&TouchDevice) m_TouchDevice = AttachInputDevice(m_Allocator->Create<LWTouch>())->AsTouch();
	if (m_Flag&AccelerometerDevice) {
		const ASensor *DefSensor = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
		if (DefSensor) {
			ASensorEventQueue_enableSensor(m_Context.m_SensorQueue, DefSensor);
			m_AccelerometerDevice = AttachInputDevice(m_Allocator->Create<LWAccelerometer>())->AsAccelerometer();
		} else LWLogWarn("No accelerometer sensor detected.");
	}
	if (m_Flag&GyroscopeDevice) {
		const ASensor *DefSensor = ASensorManager_getDefaultSensor(m_Context.m_SensorManager, ASENSOR_TYPE_GYROSCOPE);
		jobject jDefSensor = Env->CallObjectMethod(SensorManager, SensorManager_getDefaultSensor, ASENSOR_TYPE_GYROSCOPE);
		if (DefSensor) {
			float Resolution = ASensor_getResolution(DefSensor);
			jfloat Range = Env->CallFloatMethod(jDefSensor, Sensor_getMaximumRange);
			ASensorEventQueue_enableSensor(m_Context.m_SensorQueue, DefSensor);
			m_GyroscopeDevice = AttachInputDevice(m_Allocator->Create<LWGyroscope>(Resolution, (float)Range))->AsGyroscope();
			Env->DeleteLocalRef(jDefSensor);
		} else LWLogWarn("No Gyroscope sensor detected.");
	}
	Env->DeleteLocalRef(SensorManager);
	Env->DeleteLocalRef(SensorString);
}

LWWindow::~LWWindow(){

	ASensorManager_destroyEventQueue(m_Context.m_SensorManager, m_Context.m_SensorQueue);
	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
	if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
	if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);
}