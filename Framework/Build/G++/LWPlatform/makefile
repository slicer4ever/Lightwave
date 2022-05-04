CFlags = -Wall -std=c++17 -Wfatal-errors -I../../../Includes/C++11/
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWPlatform
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWPlatform/LWApplication.cpp
Sources += C++11/LWPlatform/LWDirectory.cpp
Sources += C++11/LWPlatform/LWFileByteStream.cpp
Sources += C++11/LWPlatform/LWFileStream.cpp
Sources += C++11/LWPlatform/LWInputDevice.cpp
Sources += C++11/LWPlatform/LWThread.cpp
Sources += C++11/LWPlatform/LWVideoMode.cpp
Sources += X11/LWPlatform/LWApplication_X11.cpp
Sources += X11/LWPlatform/LWDirectory_X11.cpp
Sources += X11/LWPlatform/LWFileStream_X11.cpp
Sources += X11/LWPlatform/LWInputDevice_X11.cpp
Sources += X11/LWPlatform/LWVideoMode_X11.cpp
Sources += X11/LWPlatform/LWWindow_X11.cpp
Sources += Shared/LWPlatform/LWFileStream_Windows_X11_Mac.cpp

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
