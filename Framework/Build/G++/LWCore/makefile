CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/ -mavx2
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWCore
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/


Sources = C++11/LWCore/LWAllocator.cpp
Sources += C++11/LWCore/LWByteBuffer.cpp
Sources += C++11/LWCore/LWByteStream.cpp
Sources += C++11/LWCore/LWCrypto.cpp
Sources += C++11/LWCore/LWLogger.cpp
Sources += C++11/LWCore/LWMath.cpp
Sources += C++11/LWCore/LWRef.cpp
Sources += C++11/LWCore/LWUnicode.cpp
Sources += C++11/LWCore/LWUnicodeGraphemeIterator.cpp
Sources += C++11/LWCore/LWUnicodeIterator.cpp
Sources += C++11/LWCore/LWTimer.cpp
Sources += C++11/LWCore/LWSMatrix_AVX2_Double.cpp
Sources += C++11/LWCore/LWSMatrix_AVX2_Float.cpp
Sources += C++11/LWCore/LWSQuaternion_AVX2_Double.cpp
Sources += C++11/LWCore/LWSQuaternion_AVX_Float.cpp
Sources += C++11/LWCore/LWSVector_AVX2_Double.cpp
Sources += C++11/LWCore/LWSVector_AVX_Float.cpp
Sources += C++11/LWCore/LWSVector_AVX_Int.cpp
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
