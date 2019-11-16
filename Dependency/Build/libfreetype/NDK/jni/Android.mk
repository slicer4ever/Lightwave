LOCAL_PATH := $(call my-dir)

Includes = $(LOCAL_PATH)/../../../../libfreetype/include/
Src = $(LOCAL_PATH)/../../../../../libfreetype/src/

include $(CLEAR_VARS)
LOCAL_MODULE    := libfreetype
LOCAL_SRC_FILES = $(Src)autofit/autofit.c
LOCAL_SRC_FILES += $(Src)base/ftbase.c
LOCAL_SRC_FILES += $(Src)base/ftbbox.c
LOCAL_SRC_FILES += $(Src)base/ftbitmap.c
LOCAL_SRC_FILES += $(Src)base/ftdebug.c
LOCAL_SRC_FILES += $(Src)base/ftfntfmt.c
LOCAL_SRC_FILES += $(Src)base/ftfstype.c
LOCAL_SRC_FILES += $(Src)base/ftgasp.c
LOCAL_SRC_FILES += $(Src)base/ftglyph.c
LOCAL_SRC_FILES += $(Src)base/ftgxval.c
LOCAL_SRC_FILES += $(Src)base/ftinit.c
LOCAL_SRC_FILES += $(Src)base/ftlcdfil.c
LOCAL_SRC_FILES += $(Src)base/ftmm.c
LOCAL_SRC_FILES += $(Src)base/ftotval.c
LOCAL_SRC_FILES += $(Src)base/ftpatent.c
LOCAL_SRC_FILES += $(Src)base/ftpfr.c
LOCAL_SRC_FILES += $(Src)base/ftstroke.c
LOCAL_SRC_FILES += $(Src)base/ftsynth.c
LOCAL_SRC_FILES += $(Src)base/ftsystem.c
LOCAL_SRC_FILES += $(Src)base/fttype1.c
LOCAL_SRC_FILES += $(Src)base/ftwinfnt.c
LOCAL_SRC_FILES += $(Src)bdf/bdf.c
LOCAL_SRC_FILES += $(Src)cache/ftcache.c
LOCAL_SRC_FILES += $(Src)cff/cff.c
LOCAL_SRC_FILES += $(Src)cid/type1cid.c
LOCAL_SRC_FILES += $(Src)gzip/ftgzip.c
LOCAL_SRC_FILES += $(Src)lzw/ftlzw.c
LOCAL_SRC_FILES += $(Src)pcf/pcf.c
LOCAL_SRC_FILES += $(Src)pfr/pfr.c
LOCAL_SRC_FILES += $(Src)psaux/psaux.c
LOCAL_SRC_FILES += $(Src)pshinter/pshinter.c
LOCAL_SRC_FILES += $(Src)psnames/psmodule.c
LOCAL_SRC_FILES += $(Src)raster/raster.c
LOCAL_SRC_FILES += $(Src)sfnt/sfnt.c
LOCAL_SRC_FILES += $(Src)smooth/smooth.c
LOCAL_SRC_FILES += $(Src)truetype/truetype.c
LOCAL_SRC_FILES += $(Src)type1/type1.c
LOCAL_SRC_FILES += $(Src)type42/type42.c
LOCAL_SRC_FILES += $(Src)winfonts/winfnt.c
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
endif

LOCAL_C_INCLUDES := $(Includes)
LOCAL_CPPFLAGS := -std=c++14
LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY
include $(BUILD_STATIC_LIBRARY)
