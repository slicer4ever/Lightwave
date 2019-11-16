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
TargetLibName = libLWCore
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWCore/LWAllocator.cpp
Sources += C++11/LWCore/LWByteBuffer.cpp
Sources += C++11/LWCore/LWText.cpp
Sources += C++11/LWCore/LWTimer.cpp
Sources += C++11/LWCore/LWMath.cpp
Sources += C++11/LWCore/LWCrypto.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_Default.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_DefaultDebug.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_ConcurrentCircular.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_LocalCircular.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_LocalHeap.cpp

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
