#ifndef LWEUIMANAGER_H
#include "LWEUIManager.h"
#endif
#ifndef LWEUI_H
#define LWEUI_H


struct LWEUIEvent {
	std::function<void(LWEUI*, uint32_t, void*)> m_Callback;
	uint32_t m_EventCode;
	void *m_UserData;
};

class LWEUI {
public:
	enum {
		ParentAnchorTopLeft = 0x0,
		ParentAnchorTopCenter = 0x1,
		ParentAnchorTopRight = 0x2,
		ParentAnchorMidLeft = 0x3,
		ParentAnchorMidCenter = 0x4,
		ParentAnchorMidRight = 0x5,
		ParentAnchorBtmLeft = 0x6,
		ParentAnchorBtmCenter = 0x7,
		ParentAnchorBtmRight = 0x8,
		LocalAnchorTopLeft = 0x0,
		LocalAnchorTopCenter = 0x10,
		LocalAnchorTopRight = 0x20,
		LocalAnchorMidLeft = 0x30,
		LocalAnchorMidCenter = 0x40,
		LocalAnchorMidRight = 0x50,
		LocalAnchorBtmLeft = 0x60,
		LocalAnchorBtmCenter = 0x70,
		LocalAnchorBtmRight = 0x80,
		ParentAnchorBits = 0xF,
		ParentAnchorOffsetBits = 0x0,
		LocalAnchorBIts = 0xF0,
		LocalAnchorOffsetBits = 0x4,
		DrawAfter = 0x100,
		Invisible = 0x200,
		FocusAble = 0x400,
		TabAble = 0x800,
		InvertAllowed = 0x1000,
		VisibilityChange = 0x2000,
		IgnoreOverCounter = 0x4000,
		NoScalePos = 0x8000,
		NoScaleSize = 0x10000,
		SizeToTexture = 0x20000,
		NoAutoSize = 0x40000,

		LabelLeftAligned = 0x0,
		LabelRightAligned = 0x80000,
		LabelCenterAligned = 0x100000,
		PasswordField = 0x200000,

		VerticalBar = 0x0,
		HorizontalBar = 0x400000,

		/*!< Some common events that need to be tracked. */
		MouseOver = 0x10000000,
		MouseDown = 0x20000000,
		TouchEnabled = 0x40000000,
		SelectEnabled = 0x80000000,

		Event_MouseOver = 0x0,
		Event_MouseOff,
		Event_Visible,
		Event_Invisible,
		Event_Pressed,
		Event_Released,
		Event_Focused,
		Event_LostFocus,
		Event_Changed,
		Event_TempOverInc,

		Event_Flags = 0xFF,
		Event_OverIdx = 0xFF00,
		Event_OverOffset = 8,

		MaxEvents = 16

	};

	static LWVector4f EvaluatePerPixelAttr(const char *Value);

	static LWXMLAttribute *FindAttribute(LWEXMLNode *Node, LWEXMLNode *Style, const LWText &Name);

	static const char *ParseComponentAttribute(char *Buffer, uint32_t BufferSize, const char *SrcAttribute, LWEXMLNode *Component, LWEXMLNode *ComponentNode);

	static void *FindAsset(LWEAssetManager *AM, const LWText &Name, uint32_t Type);

	static LWEUI *XMLParseSubNodes(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode *> &StyleMap, std::map<uint32_t, LWEXMLNode *> &ComponentMap);

	static bool XMLParse(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	static bool PointInside(const LWVector2f &Pnt, float PntSize, const LWVector2f &VisiblePos, const LWVector2f &VisibleSize);

	LWVector4f Update(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	LWEUI &DispatchEvent(uint32_t EventCode);

	bool RegisterEvent(uint32_t EventCode, std::function<void(LWEUI*, uint32_t, void*)> Callback, void *UserData);

	bool UnregisterEvent(uint32_t EventCode);

	LWEUI &Draw(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUI &SetFirstChild(LWEUI *UI);

	LWEUI &SetLastChild(LWEUI *UI);

	LWEUI &SetNext(LWEUI *UI);

	LWEUI &SetParent(LWEUI *UI);

	LWEUI &SetVisiblePosition(const LWVector2f &VisPos);

	LWEUI &SetVisibleSize(const LWVector2f &VisSize);

	/*!< \brief Position, x/y = % of parent's bounds, z/w = pixels of parents bound. */
	LWEUI &SetPosition(const LWVector4f &Position);

	/*!< \brief Size, x/y = % of parent's bounds, z/w = pixels of parents bound. */
	LWEUI &SetSize(const LWVector4f &Size);

	LWEUI &SetFlag(uint32_t Flag);

	LWEUI &SetVisible(bool Visible);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) = 0;

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) = 0;

	LWVector4f GetPosition(void) const;

	LWVector4f GetSize(void) const;

	LWVector4f GetVisibleBounds(void) const;

	LWVector2f GetVisiblePosition(void) const;

	LWVector2f GetVisibleSize(void) const;

	bool PointInside(const LWVector2f &Point) const;

	bool PointInsideBounds(const LWVector2f &Point) const;

	bool GetVisible(void) const;

	uint32_t GetFlag(void) const;

	LWEUI *GetFirstChild(void);

	LWEUI *GetLastChild(void);

	LWEUI *GetNext(void);

	LWEUI *GetParent(void);

	LWEUI(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

protected:
	LWEUIEvent m_EventTable[MaxEvents];
	LWVector4f m_Position;
	LWVector4f m_Size;
	LWVector4f m_VisibleBounds;
	LWVector2f m_VisiblePosition;
	LWVector2f m_VisibleSize;
	LWEUI *m_FirstChild;
	LWEUI *m_LastChild;
	LWEUI *m_Next;
	LWEUI *m_Parent;
	uint32_t m_EventCount;
	uint32_t m_Flag;
};

#endif