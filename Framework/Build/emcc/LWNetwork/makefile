CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
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
Sources += Web/LWNetwork/LWSocket_Web.cpp

T = $(Sources:.cpp=.bc)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	$(CC) $(Objs) -o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.cpp
	mkdir -p $(dir $@)
	$(CC) $(subst $(ObjPath),$(SrcPath), $(basename $@)).cpp -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).bc