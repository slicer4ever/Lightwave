LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../Includes/C++11/
Src = $(LOCAL_PATH)/../../../../../Source/

include $(CLEAR_VARS)
LOCAL_MODULE    := libLWNetwork
LOCAL_SRC_FILES := $(Src)C++11/LWNetwork/LWPacket.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWNetwork/LWPacketManager.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWNetwork/LWProtocol.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWNetwork/LWProtocolManager.cpp
LOCAL_SRC_FILES += $(Src)C++11/LWNetwork/LWSocket.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWNetwork/LWProtocolManager_NDK.cpp
LOCAL_SRC_FILES += $(Src)NDK/LWNetwork/LWSocket_NDK.cpp
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -frtti -std=c++14
include $(BUILD_STATIC_LIBRARY)
