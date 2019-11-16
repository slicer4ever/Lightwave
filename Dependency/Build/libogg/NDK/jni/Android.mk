LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../libogg/include/
Src = $(LOCAL_PATH)/../../../../../libogg/src/

include $(CLEAR_VARS)
LOCAL_MODULE    := libogg
LOCAL_SRC_FILES := $(Src)bitwise.c
LOCAL_SRC_FILES += $(Src)framing.c
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
