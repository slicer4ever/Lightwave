CFlags = -std=c++11 -pthread -Wall -Wfatal-errors -I../../../../Includes/C++11/
EFlags = -s USE_FREETYPE=1 -s USE_LIBPNG=1 -s USE_ZLIB=1 -s USE_VORBIS=1 -s USE_OGG=1 --preload-file ../../Content/ -s ALLOW_MEMORY_GROWTH=1
CC = emcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = ../../Binarys/$(Config)/$(PlatformTarget)/
LWFramework = ../../../../Binarys/$(Config)/$(PlatformTarget)/
Dependency = ../../../../../Dependency/Binarys/$(Config)/$(PlatformTarget)/
TargetName = LWSimple
ELibs = $(LWFramework)libLWCore.bc $(LWFramework)libLWPlatform.bc $(LWFramework)libLWVideo.bc $(LWFramework)libLWAudio.bc
Libs = -lX11 -lopenAL
LibPath = -L$(LWFramework) -L$(Dependency)
Obj = $(Config)/$(PlatformTarget)/
Src = ../../Source/

Sources = $(Src)C++11/main.cpp
all: Dirs $(Sources)
	$(CC) $(EFlags) $(Sources) $(LibPath) $(Libs) $(ELibs) -o $(Target)$(TargetName).html
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName).html
