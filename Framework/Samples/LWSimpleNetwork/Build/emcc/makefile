CFlags = -std=c++11 -pthread -Wall -Wfatal-errors -I../../../../Includes/C++11/
EFlags = 
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
TargetName = LWSimpleNetwork
ELibs = $(LWFramework)libLWCore.bc $(LWFramework)libLWPlatform.bc $(LWFramework)libLWNetwork.bc
Libs =
LibPath = -L$(LWFramework) -L$(Dependency)
Obj = $(Config)/$(PlatformTarget)/
Src = ../../Source/

Sources = $(Src)C++11/main.cpp
all: Dirs $(Sources)
	$(CC) $(EFlags) $(ELibs) $(Sources) $(LibPath) $(Libs) -o $(Target)$(TargetName).html
Dirs:
	mkdir -p $(Target)
clean:
	rm -f $(Target)$(TargetName).html
