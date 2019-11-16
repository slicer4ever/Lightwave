LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../Includes/C++11/
Includes += $(LOCAL_PATH)/../../../../../Dependency/libfreetype/include/
Includes += $(LOCAL_PATH)/../../../../../Dependency/libpng/
Src = $(LOCAL_PATH)/../../../../../Source/

include $(CLEAR_VARS)
LOCAL_MODULE    := libLWVideo
LOCAL_SRC_FILES := $(Src)C++11/LWVideo/LWFrameBuffer.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWImage.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWMesh.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWShader.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWTexture.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWVideoBuffer.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWVideoDriver.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWVideoState.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWFont.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGLES2.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWVideo/LWVideoDriver/LWVideoDriver_OpenGLES2_NDK.cpp
LOCAL_SRC_FILES += $(Src)Null/LWVideo/LWVideoDriver/LWVideoDriver_DirectX11_1_Null.cpp
LOCAL_SRC_FILES += $(Src)Null/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL3_2_Null.cpp
LOCAL_SRC_FILES += $(Src)Null/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL2_1_Null.cpp
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
include $(BUILD_STATIC_LIBRARY)
