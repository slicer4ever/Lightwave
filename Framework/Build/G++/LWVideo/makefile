CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/ -I/usr/include/freetype2/
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWVideo
Defines = -DGLEW_STATIC
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWVideo/LWFont.cpp
Sources += C++11/LWVideo/LWFrameBuffer.cpp
Sources += C++11/LWVideo/LWImage.cpp
Sources += C++11/LWVideo/LWMesh.cpp
Sources += C++11/LWVideo/LWPipeline.cpp
Sources += C++11/LWVideo/LWShader.cpp
Sources += C++11/LWVideo/LWTexture.cpp
Sources += C++11/LWVideo/LWVideoBuffer.cpp
Sources += C++11/LWVideo/LWVideoDriver.cpp
Sources += C++11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL3_2.cpp
Sources += C++11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL2_1.cpp
Sources += X11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL3_2_X11.cpp
Sources += X11/LWVideo/LWVideoDriver/LWVideoDriver_OpenGL2_1_X11.cpp

T = $(Sources:.cpp=.o)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	ar crf $(Target)$(TargetLibName).a $(Objs)
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.o: $(SrcPath)%.cpp
	mkdir -p $(dir $@)
	$(CC) -c  $(subst $(ObjPath),$(SrcPath), $(basename $@)).cpp -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
