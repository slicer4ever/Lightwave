LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../Includes/C++11/
Src = $(LOCAL_PATH)/../../../../../Source/

include $(CLEAR_VARS)
LOCAL_MODULE    := libLWCore
LOCAL_SRC_FILES := $(Src)C++11/LWCore/LWAllocator.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWByteBuffer.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWText.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWMath.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWTimer.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWCrypto.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWAllocators/LWAllocator_Default.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWAllocators/LWAllocator_DefaultDebug.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWAllocators/LWAllocator_LocalCircular.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWAllocators/LWAllocator_ConcurrentCircular.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWCore/LWAllocators/LWAllocator_LocalHeap.cpp
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
