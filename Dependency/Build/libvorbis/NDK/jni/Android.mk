LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../libvorbis/include/
Includes += $(LOCAL_PATH)/../../../../libogg/include/
Src = $(LOCAL_PATH)/../../../../../libvorbis/lib/

include $(CLEAR_VARS)
LOCAL_MODULE    := libvorbis
LOCAL_SRC_FILES := $(Src)analysis.c
LOCAL_SRC_FILES += $(Src)barkmel.c
LOCAL_SRC_FILES += $(Src)bitrate.c
LOCAL_SRC_FILES += $(Src)block.c
LOCAL_SRC_FILES += $(Src)codebook.c
LOCAL_SRC_FILES += $(Src)envelope.c
LOCAL_SRC_FILES += $(Src)floor0.c
LOCAL_SRC_FILES += $(Src)floor1.c
LOCAL_SRC_FILES += $(Src)info.c
LOCAL_SRC_FILES += $(Src)lookup.c
LOCAL_SRC_FILES += $(Src)lpc.c
LOCAL_SRC_FILES += $(Src)lsp.c
LOCAL_SRC_FILES += $(Src)mapping0.c
LOCAL_SRC_FILES += $(Src)mdct.c
LOCAL_SRC_FILES += $(Src)psy.c
LOCAL_SRC_FILES += $(Src)registry.c
LOCAL_SRC_FILES += $(Src)res0.c
LOCAL_SRC_FILES += $(Src)sharedbook.c
LOCAL_SRC_FILES += $(Src)smallft.c
LOCAL_SRC_FILES += $(Src)synthesis.c
LOCAL_SRC_FILES += $(Src)tone.c
LOCAL_SRC_FILES += $(Src)vorbisfile.c
LOCAL_SRC_FILES += $(Src)window.c
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
