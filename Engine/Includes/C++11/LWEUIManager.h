#ifndef LWEUIMANAGER_H
#define LWEUIMANAGER_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWUnicode.h>
#include <LWPlatform/LWTypes.h>
#include <LWPlatform/LWInputDevice.h>
#include <LWVideo/LWMesh.h>
#include <unordered_map>
#include <map>
#include "LWEXML.h"
#include "LWETypes.h"
#include "LWEUI/LWEUI.h"
#include <cstdlib>

/*!< \brief LWEUI's tooltip decoration. parsing the xmltag Tooltip has the following attributes:
	 Font: Named LWEAssetManager font for the tooltip ui to use.
	 FontMaterial: Colored material to use for the font.
	 BorderMaterial: Material to use for the border around the tooltip.
	 BackgroundMaterial: Material to use for the background.
	 BorderSize: The size of the border around the tooltip.
	 FontScale: The scale for the font of the tooltip.
*/
struct LWEUITooltip {
	LWFont *m_Font = nullptr;
	LWEUIMaterial *m_FontMaterial = nullptr;
	LWEUIMaterial *m_BorderMaterial = nullptr;
	LWEUIMaterial *m_BackgroundMaterial = nullptr;
	LWEUI *m_TempTooltipedUI = nullptr;
	LWEUI *m_TooltipedUI = nullptr;
	LWVector2f m_TooltipSize;
	float m_UnderHang = 0.0f;
	float m_BorderSize = 1.0f;
	float m_FontScale = 1.0f;

	LWEUITooltip &Draw(LWEUIFrame &Frame, LWEUIManager &UIMan, float Scale, uint64_t lCurrentTime);

	LWEUITooltip &Update(float Scale);

	LWEUITooltip() = default;
};

struct LWEUINavigation {
	static const uint32_t GamepadEnabled = 0x1;
	static const uint32_t KeyboardEnabled = 0x2;
	static const uint32_t Pressed = 0x4;
	static const uint32_t Back = 0x8;

	uint32_t m_Flag = 0;
	LWVector2f m_Center;
	LWVector2f m_Direction;
	LWVector2f m_PerpDirection;
	float m_ClosestD = 0.0f;
	LWEUI *m_ClosestUI = nullptr;
	
	bool isPressed(void) const;

	bool isBack(void) const;

	bool isEnabled(void) const;

	LWEUINavigation &ProcessUI(LWEUI *UI, const LWVector2f &VisiblePosition, const LWVector2f &VisibleSize, LWEUIManager &UIManager);

	LWEUINavigation &Update(LWWindow *Window, LWEUIManager &UIManager);

	LWEUINavigation() = default;
};

/*!< \brief UIMaterial each ui component use's materials for determining what colors to use for drawing.
*	 \note XML propertys:
*			Name: Name of material.
*			Color: Overall color of material(Set's colorA property).
*			ColorA: Gradient color left/bottom side.
*			ColorB: Gradient color right/top side.
*			Fill: Full, Gradient, VGradient.  how material colors will be applied if a graident is used when drawing.
*			Texture: name of the texture to be used.
*			SubRegion: x|y|width|height in pixels of the texture to make an atlas from it.
*/
struct LWEUIMaterial {
	static const uint32_t FillFull = 0; //XML Fill="Full"
	static const uint32_t FillGradient = 1; //XML Fill="Gradient"
	static const uint32_t FillVGradient = 2; //XML Fill="VGradient" (Vertical Gradient)

	uint32_t m_FillType = FillFull;
	LWVector4f m_ColorA = LWVector4f(1.0f);
	LWVector4f m_ColorB = LWVector4f(1.0f);
	LWTexture *m_Texture = nullptr;
	LWVector4f m_SubRegion = LWVector4f(0.0f, 0.0f, 1.0f, 1.0f); /*!< \brief Subregion, x,y is bottom left, z,w is top right of texture. */

	/*!< \brief generates the fill type's color for each quadrant. */
	LWEUIMaterial &MakeColors(LWVector4f &TLColor, LWVector4f &BLColor, LWVector4f &TRColor, LWVector4f &BRColor);

	LWEUIMaterial &MakeClippedColors(LWVector4f &TLColor, LWVector4f &BLColor, LWVector4f &TRColor, LWVector4f &BRColor, const LWVector4f &ClipRatios);

	LWEUIMaterial(const LWVector4f &Color);

	LWEUIMaterial(const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillType);

	LWEUIMaterial(const LWVector4f &Color, LWTexture *Tex, const LWVector4f &SubRegion);

	LWEUIMaterial(const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillType, LWTexture *Tex, const LWVector4f &SubRegion);

	LWEUIMaterial() = default;
};

//Callback parameters passed to UI xml parsing function, return true if processed, or false on error. */
typedef std::function<LWEUI*(LWEXMLNode *, LWEXML *, LWEUIManager *, LWEXMLNode *, const LWUTF8Iterator &, LWEXMLNode *, LWEXMLNode *, std::map<uint32_t, LWEXMLNode*> &, std::map<uint32_t, LWEXMLNode*> &)> LWEUIXMLParseCallback;

struct LWEUIFrame {
	static const uint32_t MaxTextures = 256;
	static const uint32_t ExhaustedTextures = -1;
	LWTexture *m_Textures[MaxTextures];
	uint32_t m_VertexCount[MaxTextures];
	bool m_FontTexture[MaxTextures];
	LWMesh<LWVertexUI> *m_Mesh = nullptr;
	uint32_t m_FirstVertex = 0;
	uint32_t m_TextureCount = 0;
	
	uint32_t SetActiveTexture(LWTexture *Texture, bool FontTexture);

	//x = Left Ratio, y = BottomRatio, z = Right Ratio, w = Top Ratio);
	//AABB: x = left, y = bottom, z = width, w = height.
	bool MakeClipRatios(LWVector4f &RatioRes, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &AABB);

	LWEUIFrame &ApplyClipRatios(LWVector2f &TopLeft, LWVector2f &BtmLeft, LWVector2f &TopRight, LWVector2f &BtmRight, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &Ratio);

	bool WriteFontGlyph(LWTexture *Texture, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector2f &SignedDistance, const LWVector4f &Color);

	bool WriteClippedRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &AABB);

	bool WriteClippedText(LWEUIMaterial *Mat, const LWUTF8GraphemeIterator &Text, LWFont *Fnt, const LWVector2f &Pos, float Scale, const LWVector4f &AABB);

	bool WriteRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size);

	bool WriteRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, float Theta);

	bool WriteLine(LWEUIMaterial *Mat, const LWVector2f &APos, const LWVector2f &BPos, float Thickness);

	bool WriteClippedLine(LWEUIMaterial *Mat, const LWVector2f &APos, const LWVector2f &BPos, float Thickness, const LWVector4f &AABB);

	uint32_t WriteVertices(uint32_t VertexCount);

	LWEUIFrame &operator = (LWEUIFrame &&F);

	LWEUIFrame &operator = (LWEUIFrame &F);

	LWEUIFrame(LWEUIFrame &&F);

	LWEUIFrame(LWEUIFrame &F);

	LWEUIFrame(LWMesh<LWVertexUI> *Mesh);

	LWEUIFrame();
};

struct LWEUIScreenScale {
	uint32_t m_ScreenArea = 0;
	float m_Scale = 1.0f;
};

struct LWEUIDPIScale {
	uint32_t m_ScreenDPI = 0;
	float m_Scale = 1.0f;
};

class LWEUIManager {
public:
	enum {
		MaxMaterials = 256,
		MaxScreenScales = 32,
		MaxDPIScales = 32
	};
	static const uint32_t TabNavigation = 0x1;

	static bool XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X);

	LWEUIManager &Update(const LWVector2f &Position, const LWVector2f &Size, float Scale, uint64_t lCurrentTime);

	LWEUIManager &Update(float Scale, uint64_t lCurrentTime);

	LWEUIManager &Update(uint64_t lCurrentTime);

	LWEUIManager &Draw(LWEUIFrame &Frame, float Scale, uint64_t lCurrentTime);

	LWEUIManager &Draw(LWEUIFrame &Frame, uint64_t lCurrentTime);

	LWEUI *GetNext(LWEUI *Current, bool SkipChildren = false);

	LWEUIManager &InsertUIAfter(LWEUI *UI, LWEUI *Parent, LWEUI *Prev);

	LWEUIManager &RemoveUI(LWEUI *UI, bool Destroy=true);

	LWEUIManager &SetNavigationMode(bool Enabled, bool GamepadEnabled = true, bool KeyboardEnabled = true);

	LWEUIManager &SetTabNavigation(bool bEnabled);

	bool RegisterEvent(LWEUI *UI, uint32_t EventCode, LWEUIEventCallback Callback, void *UserData);

	bool RegisterEvent(const LWUTF8Iterator &UIName, uint32_t EventCode, LWEUIEventCallback Callback, void *UserData);

	template<class T, class Y>
	bool RegisterMethodEvent(LWEUI *UI, uint32_t EventCode, Y CallBack, T* Object, void *UserData) {
		return RegisterEvent(UI, EventCode, std::bind(CallBack, Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	template<class T, class Y>
	bool RegisterMethodEvent(const LWUTF8Iterator &UIName, uint32_t EventCode, Y CallBack, T* Object, void *UserData) {
		return RegisterEvent(UIName, EventCode, std::bind(CallBack, Object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), UserData);
	}

	bool UnregisterEvent(LWEUI *UI, uint32_t EventCode);

	bool UnregisterEvent(const LWUTF8Iterator &UIName, uint32_t EventCode);

	bool DispatchEvent(LWEUI *Dispatchee, uint32_t EventCode, bool DoDispatch = true);

	bool DispatchEvent(const LWUTF8Iterator &DispatcheeName, uint32_t EventCode, bool DoDispatch = true);

	//Note built in types are automatically registered in the callback map upon LWEUIManager creation.
	bool RegisterXMLParseMethod(const LWUTF8Iterator &NodeName, LWEUIXMLParseCallback Callback);

	bool RegisterXMLParseMethod(uint32_t NodeNameHash, LWEUIXMLParseCallback Callback);

	/*!< \brief search's the m_XMLParseCallbackMap for any callback with the Hash'd Node->Name, then calls the function, returning it's result, if the result is not found, then the component checks if it's a user defined component, and tries to deserialize the node as if it were a component. If a relevant component is not found, then null is returned. */
	LWEUI *DispatchXMLParser(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	LWEUIManager &SetFocused(LWEUI *UI);

	bool PushScreenScale(const LWVector2i &Resolution, float Scale);

	bool PushDPIScale(uint32_t DPI, float Scale);

	float FindScaleForSize(const LWVector2i &Size);

	bool InsertNamedUI(const LWUTF8Iterator &Name, LWEUI *UI);

	bool isTextInputFocused(void);

	bool HasNamedUI(const LWUTF8Iterator &Name);

	LWEUI *GetNamedUI(const LWUTF8Iterator &Name);

	template<class Type>
	Type *GetNamedUI(const LWUTF8Iterator &Name) {
		return (Type*)GetNamedUI(Name);
	}

	LWEUIMaterial *InsertMaterial(const LWUTF8Iterator &Name, const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillMode, LWTexture *Texture, const LWVector4f &SubRegion);

	LWEUIMaterial *GetMaterial(const LWUTF8Iterator &Name);

	LWVector2f GetVisibleSize(void) const;

	LWVector2f GetVisiblePosition(void) const;

	LWEAssetManager *GetAssetManager(void);

	LWELocalization *GetLocalization(void);

	LWEUITooltip &GetTooltipDecoration(void);

	LWEUINavigation &GetNavigator(void);

	LWAllocator &GetAllocator(void);

	LWWindow *GetWindow(void);
	
	LWEUI *GetFirstUI(void);

	LWEUI *GetLastUI(void);

	LWEUI *GetFocusedUI(void);

	bool isNavigationModeEnabled(void) const;

	bool isTabNavigationEnabled(void) const;

	float GetScale(void) const;
	
	uint32_t GetScreenDPI(void) const;

	uint32_t GetOverCount(uint32_t PointerIdx=0) const;

	LWEUIManager(LWWindow *Window, uint32_t ScreenDPI, LWAllocator &Allocator, LWELocalization *Localization, LWEAssetManager *AssetManager);

	~LWEUIManager();
private:
	LWEUIMaterial m_MaterialTable[MaxMaterials];

	std::unordered_map<uint32_t, LWEUI*> m_NameMap;
	std::unordered_map<uint32_t, LWEUIMaterial*> m_MatTable;
	std::unordered_map<uint32_t, LWEUIXMLParseCallback> m_XMLParseCallbackMap;
	LWEUIScreenScale m_ResScaleMap[MaxScreenScales];
	LWEUIDPIScale m_DPIScaleMap[MaxDPIScales];
	LWEUITooltip m_Tooltip;
	LWEUINavigation m_Navigator;
	LWVector2f m_VisibleSize;
	LWVector2f m_VisiblePosition;
	LWAllocator &m_Allocator;
	LWEAssetManager *m_AssetManager = nullptr;
	LWELocalization *m_Localization = nullptr;
	uint32_t m_ScreenDPI;
	LWWindow *m_Window = nullptr;
	LWEUI *m_FirstUI = nullptr;
	LWEUI *m_LastUI = nullptr;
	LWEUI *m_FocusedUI = nullptr;
	float m_Scale = 1.0f;
	float m_CachedDPIScale = 0.0f;
	uint32_t m_MaterialCount = 0;
	uint32_t m_EventCount = 0;
	uint32_t m_OverCount[LWTouch::MaxTouchPoints] = {}; //0's array
	uint32_t m_TempCount[LWTouch::MaxTouchPoints] = {};
	uint32_t m_ResScaleCount = 0;
	uint32_t m_DPIScaleCount = 0;
	uint32_t m_Flag = TabNavigation;
};


#endif
