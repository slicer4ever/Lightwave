CFlags = -std=c++11 -pthread -Wall -Wfatal-errors -I../../../../Includes/C++11/
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../Binarys/$(Config)/$(PlatformTarget)/
LWFramework = ../../../../Binarys/$(Config)/$(PlatformTarget)/
Dependency = ../../../../../Dependency/Binarys/$(Config)/$(PlatformTarget)/
TargetName = LWSimple
Libs = -lLWPlatform -lLWVideo -lLWAudio -lLWCore -lGLEW -lpng -lz -lX11 -lXrandr -lGL -logg -lvorbis -lvorbisfile -lpulse -lfreetype
LibPath = -L$(LWFramework) -L$(Dependency)
Obj = $(Config)/$(PlatformTarget)/
Src = ../../Source/

Sources = $(Src)C++11/main.cpp
all: Dirs $(Sources)
	$(CC) $(Sources) $(LibPath) $(Libs) -o $(Target)$(TargetName)
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName)
