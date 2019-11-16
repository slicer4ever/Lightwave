LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../Includes/C++11/
Src = $(LOCAL_PATH)/../../../../../Source/

include $(CLEAR_VARS)
LOCAL_MODULE    := libLWPlatform
LOCAL_SRC_FILES := $(Src)C++11/LWPlatform/LWDirectory.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWPlatform/LWFileStream.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWPlatform/LWInputDevice.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWPlatform/LWVideoMode.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWPlatform/LWThread.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWDirectory_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWFileStream_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWInputDevice_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWVideoMode_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWWindow_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWPlatform/LWApplication_NDK.cpp
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14 -frtti
include $(BUILD_STATIC_LIBRARY)
