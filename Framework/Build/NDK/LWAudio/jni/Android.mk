LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../Includes/C++11/
Includes +=$(LOCAL_PATH)/../../../../../Dependency/libvorbis/include/
Includes +=$(LOCAL_PATH)/../../../../../Dependency/libogg/include/
Src = $(LOCAL_PATH)/../../../../../Source/

include $(CLEAR_VARS)
LOCAL_MODULE    := libLWAudio
LOCAL_SRC_FILES := $(Src)C++11/LWAudio/LWAudioDriver.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWAudio/LWAudioDriver_NDK.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWAudio/LWSound.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWAudio/LWAudioStream.cpp
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
