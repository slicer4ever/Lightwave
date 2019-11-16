CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/
CC = clang $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = clang -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWNetwork
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWNetwork/LWPacket.cpp
Sources += C++11/LWNetwork/LWPacketManager.cpp
Sources += C++11/LWNetwork/LWProtocol.cpp
Sources += C++11/LWNetwork/LWProtocolManager.cpp
Sources += C++11/LWNetwork/LWSocket.cpp
Sources += X11/LWNetwork/LWProtocolManager_X11.cpp
Sources += X11/LWNetwork/LWSocket_X11.cpp

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