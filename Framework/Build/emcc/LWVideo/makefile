CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/
EFlags = -s USE_LIBPNG=1 -s USE_FREETYPE=1 -s USE_ZLIB=1
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWVideo
Defines = -DGLEW_STATIC
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWVideo/LWFrameBuffer.cpp
Sources += C++11/LWVideo/LWImage.cpp
Sources += C++11/LWVideo/LWMesh.cpp
Sources += C++11/LWVideo/LWShader.cpp
Sources += C++11/LWVideo/LWTexture.cpp
Sources += C++11/LWVideo/LWVideoBuffer.cpp
Sources += C++11/LWVideo/LWVideoDriver.cpp
Sources += C++11/LWVideo/LWVideoState.cpp
Sources += C++11/LWVideo/LWFont.cpp
Sources += Null/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL3_2_Null.cpp
Sources += Null/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL2_1_Null.cpp
Sources += Web/LWVideo/LWVideoDriver/LWVideoDriver_OpenGLES2_Web.cpp
Sources += C++11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGLES2.cpp
Sources += Null/LWVideo/LWVideoDriver/LWVideoDriver_DirectX11_1_Null.cpp

T = $(Sources:.cpp=.bc)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	$(CC) $(EFlags) $(Objs) -o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.cpp
	mkdir -p $(dir $@)
	$(CC) $(EFlags)  $(subst $(ObjPath),$(SrcPath), $(basename $@)).cpp -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).bc
