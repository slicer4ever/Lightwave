# Generated by VisualGDB

LOCAL_PATH := $(call my-dir)
CONFIG = Release
ifdef NDK_DEBUG
	CONFIG = Debug
endif

include $(CLEAR_VARS)
LOCAL_MODULE := LWCore
LOCAL_SRC_FILES := ../../../../../Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWCore.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWPlatform
LOCAL_SRC_FILES := ../../../../../Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWPlatform.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWNetwork
LOCAL_SRC_FILES := ../../../../../Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWNetwork.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
Includes := $(LOCAL_PATH)../../../../../../Includes/C++11/
Src := ../../../Source/

LOCAL_MODULE    := LWSimpleNetwork
LOCAL_SRC_FILES := $(Src)C++11/main.cpp
LOCAL_C_INCLUDES := $(Includes)
LOCAL_STATIC_LIBRARIES := LWNetwork LWCore LWAudio
LOCAL_SHARED_LIBRARIES :=
LOCAL_WHOLE_STATIC_LIBRARIES := LWPlatform
LOCAL_LDLIBS := -llog -landroid -latomic
LOCAL_CFLAGS :=
LOCAL_CPPFLAGS := -std=c++11
LOCAL_LDFLAGS :=
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
LOCAL_CFLAGS := -DPNG_USE_ARM_NEON
endif
include $(BUILD_SHARED_LIBRARY)
