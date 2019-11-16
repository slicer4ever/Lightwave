CFlags = -Wall -std=c++14 -Wfatal-errors -I../../Includes/C++11/ -I../../../Framework/Includes/C++11/
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
TargetLibName = libLWEngine
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../Source/

Sources = C++11/LWEAsset.cpp
Sources += C++11/LWEUIManager.cpp
Sources += C++11/LWEJobQueue.cpp
Sources += C++11/LWEJson.cpp
Sources += C++11/LWELocalization.cpp
Sources += C++11/LWEXML.cpp
#Sources += C++11/LWEProtocols/LWEProtocolHttp.cpp
#Sources += C++11/LWEProtocols/LWEProtocolWebSocket.cpp
Sources += C++11/LWEUI/LWEUI.cpp
Sources += C++11/LWEUI/LWEUIAdvLabel.cpp
Sources += C++11/LWEUI/LWEUIButton.cpp
Sources += C++11/LWEUI/LWEUIComponent.cpp
Sources += C++11/LWEUI/LWEUILabel.cpp
Sources += C++11/LWEUI/LWEUIListBox.cpp
Sources += C++11/LWEUI/LWEUIRect.cpp
Sources += C++11/LWEUI/LWEUIScrollBar.cpp
Sources += C++11/LWEUI/LWEUITextInput.cpp

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
