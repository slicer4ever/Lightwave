LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../libz/
Src = $(LOCAL_PATH)/../../../../../libz/

include $(CLEAR_VARS)
LOCAL_MODULE    := libz
LOCAL_SRC_FILES := $(Src)adler32.c
LOCAL_SRC_FILES += $(Src)compress.c
LOCAL_SRC_FILES += $(Src)crc32.c
LOCAL_SRC_FILES += $(Src)deflate.c
LOCAL_SRC_FILES += $(Src)gzclose.c
LOCAL_SRC_FILES += $(Src)gzlib.c
LOCAL_SRC_FILES += $(Src)gzread.c
LOCAL_SRC_FILES += $(Src)gzwrite.c
LOCAL_SRC_FILES += $(Src)infback.c
LOCAL_SRC_FILES += $(Src)inffast.c
LOCAL_SRC_FILES += $(Src)inflate.c
LOCAL_SRC_FILES += $(Src)inftrees.c
LOCAL_SRC_FILES += $(Src)trees.c
LOCAL_SRC_FILES += $(Src)uncompr.c
LOCAL_SRC_FILES += $(Src)zutil.c
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
