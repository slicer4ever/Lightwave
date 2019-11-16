CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/
EFlags = 
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWPlatform
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWPlatform/LWDirectory.cpp
Sources += C++11/LWPlatform/LWFileStream.cpp
Sources += C++11/LWPlatform/LWInputDevice.cpp
Sources += C++11/LWPlatform/LWVideoMode.cpp
Sources += Web/LWPlatform/LWThread_Web.cpp
Sources += Web/LWPlatform/LWApplication_Web.cpp
Sources += X11/LWPlatform/LWDirectory_X11.cpp
Sources += Web/LWPlatform/LWFileStream_Web.cpp
Sources += Web/LWPlatform/LWInputDevice_Web.cpp
Sources += Web/LWPlatform/LWVideoMode_Web.cpp
Sources += Web/LWPlatform/LWWindow_Web.cpp
Sources += Shared/LWPlatform/LWFileStream_Windows_X11_Mac.cpp
T = $(Sources:.cpp=.bc)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	$(CC) $(Objs) -o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.cpp
	mkdir -p $(dir $@)
	$(CC)  $(subst $(ObjPath),$(SrcPath), $(basename $@)).cpp -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).bc
