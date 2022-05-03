CFlags = -Wall -std=c++14 -Wfatal-errors -I../../Includes/C++11/ -I../../../Dependency/Includes/C++11/ -I../../../Framework/Includes/C++11/
CC = g++ $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = g++ -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libLWEngine
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../Source/

Sources = C++11/LWEAsset.cpp
Sources += C++11/LWECamera.cpp
Sources += C++11/LWEGeometry2D.cpp
Sources += C++11/LWEGeometry3D.cpp
Sources += C++11/LWEGLTFParser.cpp
Sources += C++11/LWEJobQueue.cpp
Sources += C++11/LWEJson.cpp
Sources += C++11/LWELocalization.cpp
Sources += C++11/LWEMesh.cpp
Sources += C++11/LWERenderer.cpp
Sources += C++11/LWERenderFrame.cpp
Sources += C++11/LWERenderPass.cpp
Sources += C++11/LWERenderTypes.cpp
Sources += C++11/LWESVG.cpp
Sources += C++11/LWEUIManager.cpp
Sources += C++11/LWEVideoPlayer.cpp
Sources += C++11/LWEVideoDecoderIVF.cpp
Sources += C++11/LWEVideoDecoderWEBM.cpp
Sources += C++11/LWEVideoPlayer.cpp
Sources += C++11/LWEXML.cpp
Sources += C++11/LWEProtocols/LWEProtocol_HTTP.cpp
Sources += C++11/LWEProtocols/LWEProtocol_HTTPS.cpp
Sources += C++11/LWEProtocols/LWEProtocol_TLS.cpp
Sources += C++11/LWEProtocols/LWEProtocol_WebSocket.cpp
Sources += C++11/LWEProtocols/LWEProtocol_WebSocketSecure.cpp
Sources += C++11/LWERenderPasses/LWEGausianBlurPass.cpp
Sources += C++11/LWERenderPasses/LWEGeometryPass.cpp
Sources += C++11/LWERenderPasses/LWEPPass.cpp
Sources += C++11/LWERenderPasses/LWEResolvePass.cpp
Sources += C++11/LWERenderPasses/LWEShadowMapPass.cpp
Sources += C++11/LWERenderPasses/LWEUIPass.cpp
Sources += C++11/LWEUI/LWEUI.cpp
Sources += C++11/LWEUI/LWEUIButton.cpp
Sources += C++11/LWEUI/LWEUIComponent.cpp
Sources += C++11/LWEUI/LWEUILabel.cpp
Sources += C++11/LWEUI/LWEUIListBox.cpp
Sources += C++11/LWEUI/LWEUIRect.cpp
Sources += C++11/LWEUI/LWEUIRichLabel.cpp
Sources += C++11/LWEUI/LWEUIScrollBar.cpp
Sources += C++11/LWEUI/LWEUITextInput.cpp
Sources += C++11/LWEUI/LWEUITreeList.cpp

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