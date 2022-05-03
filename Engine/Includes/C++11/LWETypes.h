#ifndef LWETYPES_H
#define LWETYPES_H 
#include <functional>

class LWEAsset;

class LWEAssetManager;

class LWEUIManager;

class LWERenderer;

class LWERenderFrame;

class LWEMesh;

class LWERenderFrame;

class LWEUI;

class LWEUILabel;

class LWEUIRichLabel;

class LWEUIRect;

class LWEUIListBox;

class LWEUITextInput;

class LWEUIComponent;

class LWEUIButton;

class LWELocalization;

struct LWEUIMaterial;

class LWEJson;

struct LWEJObject;

class LWEXML;

struct LWEXMLNode;

struct LWEXMLAttribute;

struct LWEUIFrame;

struct LWEJob;

struct LWEJobThread;

class LWEJobQueue;

class LWEGLTFParser;

class LWEVideoPlayer;

class LWEVideoDecoder;

typedef std::function<void(LWEUI*, uint32_t, void*)> LWEUIEventCallback;

#endif