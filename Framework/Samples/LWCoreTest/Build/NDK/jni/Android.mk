CONFIG = Release
ifdef NDK_DEBUG
	CONFIG = Debug
endif
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := LWCore
LOCAL_SRC_FILES := ../../../../../Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWCore.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
Includes = $(LOCAL_PATH)../../../../../../Includes/C++11/
Src = $(LOCAL_PATH)../../../../../Source/

LOCAL_MODULE    := LWCoreTest
LOCAL_SRC_FILES := $(Src)C++11/main.cpp
LOCAL_SRC_FILES += $(Src)NDK/main_NDK.cpp
LOCAL_C_INCLUDES := $(Includes)
LOCAL_STATIC_LIBRARIES := libLWCore
LOCAL_SHARED_LIBRARIES := 
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv1_CM -latomic
LOCAL_CFLAGS :=
LOCAL_CPPFLAGS := -std=c++14
LOCAL_LDFLAGS := 
include $(BUILD_SHARED_LIBRARY)