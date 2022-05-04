CFlags = -Wall -Wfatal-errors -I../../../libcompressonator/cmp_core/source/ -I../../../libcompressonator/cmp_core/shaders/ -std=c++17
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = gcc -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libz
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../libcompressonator/cmp_core/shaders/
Sources = bc1_encode_kernel.cpp
Sources += bc2_encode_kernel.cpp
Sources += bc3_encode_kernel.cpp
Sources += bc4_encode_kernel.cpp
Sources += bc5_encode_kernel.cpp
Sources += bc6_encode_kernel.cpp
Sources += bc7_encode_kernel.cpp

T = $(Sources:.cpp=.o)
Objs = $(addprefix $(ObjPath), $(T))
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
