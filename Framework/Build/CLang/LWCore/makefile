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
TargetLibName = libLWCore
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWCore/LWAllocator.cpp
Sources += C++11/LWCore/LWByteBuffer.cpp
Sources += C++11/LWCore/LWText.cpp
Sources += C++11/LWCore/LWTimer.cpp
Sources += C++11/LWCore/LWCrypto.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_Default.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_DefaultDebug.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_ConcurrentCircular.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_LocalCircular.cpp
Sources += C++11/LWCore/LWAllocators/LWAllocator_LocalHeap.cpp

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
