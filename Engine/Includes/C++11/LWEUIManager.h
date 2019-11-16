#ifndef LWEUIMANAGER_H
#define LWEUIMANAGER_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWText.h>
#include <LWPlatform/LWTypes.h>
#include <LWPlatform/LWInputDevice.h>
#include <LWVideo/LWMesh.h>
#include <unordered_map>
#include <map>
#include <functional>
#include "LWEXML.h"
#include "LWETypes.h"
#include "LWEUI/LWEUI.h"
#include "LWEUI/LWEUIButton.h"
#include "LWEUI/LWEUILabel.h"
#include "LWEUI/LWEUIRect.h"
#include "LWEUI/LWEUIListBox.h"
#include "LWEUI/LWEUIScrollBar.h"
#include "LWEUI/LWEUITextInput.h"
#include "LWEUI/LWEUIComponent.h"
#include "LWEUI/LWEUIAdvLabel.h"
#include <cstdlib>

struct LWEUIMaterial {
	LWVector4f m_Color;
	LWTexture *m_Texture;
	LWVector4f m_SubRegion; /*!< \brief Subregion, x,y is top left, z,w is bottom right of texture. */

	LWEUIMaterial(const LWVector4f &Color);

	LWEUIMaterial(const LWVector4f &Color, LWTexture *Tex, const LWVector4f &SubRegion);

	LWEUIMaterial() = default;
};

struct LWEUIFrame {
	enum {
		MaxTextures = 256
	};
	LWTexture *m_Textures[MaxTextures];
	uint32_t m_VertexCount[MaxTextures];
	bool m_FontTexture[MaxTextures];
	LWMesh<LWVertexUI> *m_Mesh;
	uint32_t m_FirstVertex;
	uint32_t m_TextureCount;
	
	bool SetActiveTexture(LWTexture *Texture, bool FontTexture);

	bool WriteFontGlyph(LWTexture *Texture, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector4f &Color);

	uint32_t WriteVertices(uint32_t VertexCount);

	LWEUIFrame &operator = (LWEUIFrame &&F);

	LWEUIFrame &operator = (LWEUIFrame &F);

	LWEUIFrame(LWEUIFrame &&F);

	LWEUIFrame(LWEUIFrame &F);

	LWEUIFrame(LWMesh<LWVertexUI> *Mesh);

	LWEUIFrame();
};

struct LWEUIScreenScale {
	uint32_t m_ScreenArea;
	float m_Scale;
};

struct LWEUIDPIScale {
	uint32_t m_ScreenDPI;
	float m_Scale;
};

class LWEUIManager {
public:
	enum {
		MaxMaterials = 256,
		MaxScreenScales = 32,
		MaxDPIScales = 32
	};

	static const char *GetTextureShaderSource(void);

	static const char *GetColorShaderSource(void);

	static const char *GetYUVTextureShaderSource(void);

	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X);

	LWEUIManager &Update(const LWVector2f &Position, const LWVector2f &Size, float Scale, uint64_t lCurrentTime);

	LWEUIManager &Update(float Scale, uint64_t lCurrentTime);

	LWEUIManager &Update(uint64_t lCurrentTime);

	LWEUIManager &Draw(LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUIManager &Draw(LWEUIFrame *Frame, uint64_t lCurrentTime);

	LWEUI *GetNext(LWEUI *Current, bool SkipChildren = false);

	LWEUIManager &InsertUIAfter(LWEUI *UI, LWEUI *Parent, LWEUI *Prev);

	LWEUIManager &RemoveUI(LWEUI *UI, bool Destroy=true);

	bool RegisterEvent(LWEUI *UI, uint32_t EventCode, std::function<void(LWEUI*, uint32_t, void*)> Callback, void *UserData);

	bool RegisterEvent(const LWText &UIName, uint32_t EventCode, std::function<void(LWEUI *, uint32_t, void*)> Callback, void *UserData);

	template<class T, class Y>
	bool RegisterMethodEvent(LWEUI *UI, uint32_t EventCode, Y CallBack, T* Object, void *UserData) {
		return RegisterEvent(UI, EventCode, std::bind(CallBack, Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	template<class T, class Y>
	bool RegisterMethodEvent(const LWText &UIName, uint32_t EventCode, Y CallBack, T* Object, void *UserData) {
		return RegisterEvent(UIName, EventCode, std::bind(CallBack, Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	bool UnregisterEvent(LWEUI *UI, uint32_t EventCode);

	bool UnregisterEvent(const LWText &UIName, uint32_t EventCode);

	LWEUIManager &DispatchEvent(LWEUI *Dispatchee, uint32_t EventCode);

	LWEUIManager &DispatchEvent(const char *DispatcheeName, uint32_t EventCode);

	LWEUIManager &DispatchEventf(const char *DispathceeNameFmt, uint32_t EventCode, ...);

	LWEUIManager &SetFocused(LWEUI *UI);

	bool PushScreenScale(const LWVector2i &Resolution, float Scale);

	bool PushDPIScale(uint32_t DPI, float Scale);

	float FindScaleForSize(const LWVector2i &Size);

	bool InsertNamedUI(const LWText &Name, LWEUI *UI);

	bool isTextInputFocused(void);

	LWEUI *GetNamedUI(const LWText &Name);

	LWEUI *GetNamedUIf(const char *Format, ...);

	LWEUIMaterial *InsertMaterial(const LWText &Name, const LWVector4f &Color, LWTexture *Texture, const LWVector4f &SubRegion);

	LWEUIMaterial *GetMaterial(const LWText &Name);

	LWVector2f GetVisibleSize(void) const;

	LWVector2f GetVisiblePosition(void) const;

	LWEAssetManager *GetAssetManager(void);

	LWELocalization *GetLocalization(void);

	LWAllocator *GetAllocator(void);

	LWWindow *GetWindow(void);
	
	LWEUI *GetFirstUI(void);

	LWEUI *GetLastUI(void);

	LWEUI *GetFocusedUI(void);

	float GetLastScale(void) const;
	
	uint32_t GetScreenDPI(void) const;

	uint32_t GetOverCount(uint32_t PointerIdx=0) const;

	LWEUIManager(LWWindow *Window, uint32_t ScreenDPI, LWAllocator *Allocator, LWELocalization *Localization, LWEAssetManager *AssetManager);

	~LWEUIManager();
private:
	LWEUIMaterial m_MaterialTable[MaxMaterials];

	std::unordered_map<uint32_t, LWEUI*> m_NameMap;
	std::unordered_map<uint32_t, LWEUIMaterial*> m_MatTable;
	LWEUIScreenScale m_ResScaleMap[MaxScreenScales];
	LWEUIDPIScale m_DPIScaleMap[MaxDPIScales];
	LWVector2f m_VisibleSize;
	LWVector2f m_VisiblePosition;
	LWAllocator *m_Allocator;
	LWEAssetManager *m_AssetManager;
	LWELocalization *m_Localization;
	uint32_t m_ScreenDPI;
	LWWindow *m_Window;
	LWEUI *m_FirstUI;
	LWEUI *m_LastUI;
	LWEUI *m_FocusedUI;
	float m_LastScale;
	float m_CachedDPIScale;
	uint32_t m_MaterialCount;
	uint32_t m_EventCount;
	uint32_t m_OverCount[LWTouch::MaxTouchPoints];
	uint32_t m_TempCount[LWTouch::MaxTouchPoints];
	uint32_t m_ResScaleCount;
	uint32_t m_DPIScaleCount;
};


#endif
