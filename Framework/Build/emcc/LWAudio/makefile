CFlags = -Wall -std=c++14 -Wfatal-errors -I../../../Includes/C++11/
EFlags = -s USE_VORBIS=1 -s USE_OGG=1
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWAudio
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../Source/

Sources = C++11/LWAudio/LWAudioDriver.cpp
Sources += C++11/LWAudio/LWAudioStream.cpp
Sources += C++11/LWAudio/LWSound.cpp
Sources += C++11/LWAudio/LWAudioDriver_OpenAL.cpp
T = $(Sources:.cpp=.bc)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	emcc $(Objs) -o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.cpp
	mkdir -p $(dir $@)
	$(CC) $(EFlags) $(subst $(ObjPath),$(SrcPath), $(basename $@)).cpp -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
