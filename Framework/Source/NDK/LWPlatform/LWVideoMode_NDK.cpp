#include "LWPlatform/LWVideoMode.h"
#include "LWPlatform/LWPlatform.h"
#include <cstring>
#include <algorithm>

uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize){
	
	if (Buffer) *Buffer = GetActiveMode();
	return 1;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements){
	
	LWVideoMode Default = GetActiveMode();
	if (Default.GetSize().x < SizeRequirements.x) return 0;
	if (Default.GetSize().y < SizeRequirements.y) return 0;
	if (SizeRequirements.z>0 && Default.GetSize().x>SizeRequirements.z) return 0;
	if (SizeRequirements.w > 0 && Default.GetSize().y > SizeRequirements.w) return 0;
	if (Default.GetFrequency() < (uint32_t)FrequencyRequirements.x) return 0;
	if (FrequencyRequirements.y >0 && Default.GetFrequency() > (uint32_t)FrequencyRequirements.y) return 0;
	if (FlagRequirements != 0 && (Default.GetFlag()&FlagRequirements) == 0) return 0;
	if (Buffer) *Buffer = Default;
	return 1;
}

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode){
	return false;
}

LWVideoMode LWVideoMode::GetActiveMode(void){
	//Find all of the classes/methods to call.
	static jclass DisplayClass = 0;
	static jclass WindowManagerClass = 0;
	static jclass ContextClass = 0;
	static jclass PointClass = 0;
	static jclass ResourcesClass = 0;
	static jclass DisplayMetricsClass = 0;

	static jmethodID Context_GetSystemService = 0;
	static jmethodID Context_getResources = 0;
	static jmethodID Resources_getDisplayMetrics = 0;
	static jmethodID WindowManager_GetDefaultDisplay = 0;
	static jmethodID Display_getDisplayMetrics = 0;
	static jmethodID Display_getSize = 0;
	static jmethodID Display_getRefreshRate = 0;
	static jmethodID Display_getRotation = 0;
	static jmethodID Point_Constructor = 0;
	static jmethodID DisplayMetrics_Constructor = 0;
	static jfieldID Point_x = 0;
	static jfieldID Point_y = 0;
	static jfieldID DisplayMetrics_WidthPixels = 0;
	static jfieldID DisplayMetrics_HeightPixels = 0;
	static jfieldID DisplayMetrics_densityDPI = 0;
	
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCtr = 0;
	const uint32_t Rotate_0 = 0;
	const uint32_t Rotate_90 = 1;
	const uint32_t Rotate_180 = 2;
	const uint32_t Rotate_270 = 3;
	if (EnvCtr != LWAppContext.m_AppEnvCounter) {
		EnvCtr = LWAppContext.m_AppEnvCounter;
		Env = LWAppContext.m_AppEnv;
		DisplayClass = Env->FindClass("android/view/Display");
		WindowManagerClass = Env->FindClass("android/view/WindowManager");
		ContextClass = Env->FindClass("android/content/Context");
		PointClass = Env->FindClass("android/graphics/Point");
		ResourcesClass = Env->FindClass("android/content/res/Resources");
		DisplayMetricsClass = Env->FindClass("android/util/DisplayMetrics");
		Point_x = Env->GetFieldID(PointClass, "x", "I");
		Point_y = Env->GetFieldID(PointClass, "y", "I");
		DisplayMetrics_WidthPixels = Env->GetFieldID(DisplayMetricsClass, "widthPixels", "I");
		DisplayMetrics_HeightPixels = Env->GetFieldID(DisplayMetricsClass, "heightPixels", "I");
		DisplayMetrics_densityDPI = Env->GetFieldID(DisplayMetricsClass, "densityDpi", "I");
		Context_GetSystemService = Env->GetMethodID(ContextClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		WindowManager_GetDefaultDisplay = Env->GetMethodID(WindowManagerClass, "getDefaultDisplay", "()Landroid/view/Display;");
		Display_getRefreshRate = Env->GetMethodID(DisplayClass, "getRefreshRate", "()F");
		Display_getRotation = Env->GetMethodID(DisplayClass, "getRotation", "()I");
		Point_Constructor = Env->GetMethodID(PointClass, "<init>", "()V");
		DisplayMetrics_Constructor = Env->GetMethodID(DisplayMetricsClass, "<init>", "()V");
#if __ANDROID_API__ > 11
		Display_getSize = Env->GetMethodID(DisplayClass, "getSize", "(Landroid/graphics/Point;)V");
		Display_getDisplayMetrics = Env->GetMethodID(DisplayClass, "getMetrics", "(Landroid/util/DisplayMetrics;)V");
#else
		Context_getResources = Env->GetMethodID(ContextClass, "getResources", "()Landroid/content/res/Resources;");
		Resources_getDisplayMetrics = Env->GetMethodID(ResourcesClass, "getDisplayMetrics", "()Landroid/util/DisplayMetrics;");	
#endif
	}
	jstring WindowStr = Env->NewStringUTF("window");
	jobject WindowManager = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_GetSystemService, WindowStr);
	jobject DefaultDisplay = Env->CallObjectMethod(WindowManager, WindowManager_GetDefaultDisplay);
	jobject ScreenPoint = Env->NewObject(PointClass, Point_Constructor);
	jint Rotation = Env->CallIntMethod(DefaultDisplay, Display_getRotation);
	jfloat RefreshRate = Env->CallFloatMethod(DefaultDisplay, Display_getRefreshRate);
#if __ANDROID_API__ > 13
	Env->CallVoidMethod(DefaultDisplay, Display_getSize, ScreenPoint);
	jobject Metrics = Env->NewObject(DisplayMetricsClass, DisplayMetrics_Constructor);
	Env->CallVoidMethod(DefaultDisplay, Display_getDisplayMetrics, Metrics);
	jint Width = Env->GetIntField(ScreenPoint, Point_x);
	jint Height = Env->GetIntField(ScreenPoint, Point_y);
	jint DPI = Env->GetIntField(Metrics, DisplayMetrics_densityDPI);
	Env->DeleteLocalRef(Metrics);
#else
	jobject Resources = Env->CallObjectMethod(LWAppContext.m_App->clazz, Context_getResources);
	jobject Metrics = Env->CallObjectMethod(Resources, Resources_getDisplayMetrics);
	jint Width = Env->GetIntField(Metrics, DisplayMetrics_WidthPixels);
	jint Height = Env->GetIntField(Metrics, DisplayMetrics_HeightPixels);
	jint DPI = Env->GetIntField(Metrics, DisplayMetrics_densityDPI);
	Env->DeleteLocalRef(Metrics);
	Env->DeleteLocalRef(Resources);
#endif
	Env->DeleteLocalRef(ScreenPoint);
	Env->DeleteLocalRef(DefaultDisplay);
	Env->DeleteLocalRef(WindowManager);
	Env->DeleteLocalRef(WindowStr);
	return LWVideoMode(LWVector2i((uint32_t)Width, (uint32_t)Height), LWVector2i(DPI, DPI), (uint32_t)RefreshRate, Colored32Bit | (((uint32_t)Rotation == Rotate_0) ? Rotation_0 : ((uint32_t)Rotation == Rotate_90) ? Rotation_90 : ((uint32_t)Rotation == Rotate_180) ? Rotation_180 : Rotation_270));
	
}