LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../libpng/
Src = $(LOCAL_PATH)/../../../../../libpng/

include $(CLEAR_VARS)
LOCAL_MODULE    := libpng
LOCAL_SRC_FILES := $(Src)png.c
LOCAL_SRC_FILES += $(Src)pngerror.c
LOCAL_SRC_FILES += $(Src)pngget.c
LOCAL_SRC_FILES += $(Src)pngmem.c
LOCAL_SRC_FILES += $(Src)pngpread.c
LOCAL_SRC_FILES += $(Src)pngread.c
LOCAL_SRC_FILES += $(Src)pngrio.c
LOCAL_SRC_FILES += $(Src)pngrtran.c
LOCAL_SRC_FILES += $(Src)pngrutil.c
LOCAL_SRC_FILES += $(Src)pngset.c
LOCAL_SRC_FILES += $(Src)pngtest.c
LOCAL_SRC_FILES += $(Src)pngtrans.c
LOCAL_SRC_FILES += $(Src)pngwio.c
LOCAL_SRC_FILES += $(Src)pngwrite.c
LOCAL_SRC_FILES += $(Src)pngwtran.c
LOCAL_SRC_FILES += $(Src)pngwutil.c
LOCAL_SRC_FILES += $(Src)arm/arm_init.c
LOCAL_SRC_FILES += $(Src)arm/filter_neon_intrinsics.c
LOCAL_SRC_FILES += $(Src)arm/filter_neon.S
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
