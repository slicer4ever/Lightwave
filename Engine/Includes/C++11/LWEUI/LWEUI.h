#ifndef LWEUI_H
#define LWEUI_H
#include "LWEUIManager.h"

struct LWEUIEvent {
	LWEUIEventCallback m_Callback;
	uint32_t m_EventCode;
	void *m_UserData;
};

class LWEUIButton;

class LWEUIComponent;

class LWEUILabel;

class LWEUIListBox;

class LWEUIRect;

class LWEUIRichLabel;

class LWEUIScrollBar;

class LWEUITextInput;

class LWEUITreeList;

class LWEUI {
public:
	enum : uint64_t {
		ParentAnchorTopLeft = 0x0, //XML Flags: ParentAnchorTopLeft
		ParentAnchorTopCenter = 0x1, //XML Flags: ParentAnchorTopCenter
		ParentAnchorTopRight = 0x2, //XML Flags: ParentAnchorTopRight
		ParentAnchorMidLeft = 0x3, //XML Flags: ParentAnchorMidLeft
		ParentAnchorMidCenter = 0x4, //XML Flags: ParentAnchorMidCenter
		ParentAnchorMidRight = 0x5,  //XML Flags: ParentAnchorMidRight
		ParentAnchorBtmLeft = 0x6,  //XML Flags: ParentAnchorBtmLeft
		ParentAnchorBtmCenter = 0x7, //XML Flags: ParentAnchorBtmCenter
		ParentAnchorBtmRight = 0x8, //XML Flags: ParentAnchorBtmRight
		LocalAnchorTopLeft = 0x0, //XML Flags: LocalAnchorTopLeft
		LocalAnchorTopCenter = 0x10, //XML Flags: LocalAnchorTopCenter
		LocalAnchorTopRight = 0x20, //XML Flags: LocalAnchorTopRight
		LocalAnchorMidLeft = 0x30, //XML Flags: LocalAnchorMidLeft
		LocalAnchorMidCenter = 0x40, //XML Flags: LocalAnchorMidCenter
		LocalAnchorMidRight = 0x50,  //XML Flags: LocalAnchorMidRight
		LocalAnchorBtmLeft = 0x60,  //XML Flags: LocalAnchorBtmLeft
		LocalAnchorBtmCenter = 0x70, //XML Flags: LocalAnchorBtmCenter
		LocalAnchorBtmRight = 0x80, //XML Flags: LocalAnchorBtmRight
		ParentAnchorBits = 0xF,
		ParentAnchorOffsetBits = 0x0,
		LocalAnchorBIts = 0xF0,
		LocalAnchorOffsetBits = 0x4,
		DrawAfter = 0x100, //XML Flags: DrawAfter
		Invisible = 0x200, //XML Flags: Invisible
		FocusAble = 0x400, //XML Flags: FocusAble
		TabAble = 0x800, //XML Flags: TabAble
		InvertAllowed = 0x1000, //XML Flags: InvertAllowed (TextInput flag for inverted limited only allowed characters)
		IgnoreOverCounter = 0x2000, //XML Flags: IgnoreOverCounter (ui element does not increment the over counter for the mice or touchscreen.)
		NoScalePos = 0x4000, //NoScalePos the position of the ui element is not scaled by resolution+dpi calculated scale.
		NoScaleSize = 0x8000, //NoScaleSize the size of the ui element is not scaled by resolution+dpi calculated scale.
		SizeToTexture = 0x10000, //SizeToTexture Rect, Buttons are automatically scaled to their material's texture.
		NoAutoHeightSize = 0x20000, //NoAutoHeightSize ui elements that autoscale(labels, AdvLabel, Component) will ignore sizing their height components automatically.
		NoAutoWidthSize = 0x40000, //NoAutoWidthSize same as NoAutoHeightSize but for width instead of height.
		NoAutoSize = NoAutoWidthSize | NoAutoHeightSize, //Combines both NoAutoHeightSize+NoAutoWidthSize.

		LabelLeftAligned = 0x0, //XML Flags: AlignLeft (Label text will be left aligned)
		LabelRightAligned = 0x80000, //XML Flags: AlignRight (Label text will be right aligned).
		LabelCenterAligned = 0x100000, //XML Flags: AlignCenter (Label text will be horizontally center aligned).
		LabelBottomAligned = 0x0, //XML Flags: AlignBottom (Label text will be bottom aligned).
		LabelVCenterAligned = 0x20000, //XML Flags: AlignVCenter (Label text will be vertically center aligned).
		LabelTopAligned = 0x400000, //XML Flags: AlignTop (Label text will be aligned to the top).

		PasswordField = 0x800000, //XML Flags: PasswordField

		VerticalBar = 0x0, //XML Flags: VerticalBar
		HorizontalBar = 0x1000000, //XML Flags: HorizontalBar

		/*!< Some common events that need to be tracked. */
		MouseOver = 0x10000000,
		MouseLDown = 0x20000000, /*!< \brief flag for mouse left down. */
		MouseRDown = 0x40000000, /*!< \brief flag for mouse right down. */
		MouseMDown = 0x80000000, /*!< \brief flag for mouse middle down. */
		TouchEnabled = 0x100000000,
		SelectEnabled = 0x200000000,

		Event_MouseOver = 0x0,
		Event_MouseOff,
		Event_Visible,
		Event_Invisible,
		Event_Pressed, /*!< \brief default pressed event, left mouse button, touch events, or navigator events happen here. */
		Event_Released, /*!< \brief default released event, left mouse button, touch events, or navigator events happen here. */
		Event_RPressed, /*!< \brief event for right mouse button pressed. */
		Event_RReleased, /*!< \brief event for right mouse button released. */
		Event_MPressed, /*!< \brief event for middle mouse button pressed. */
		Event_MReleased, /*!< \brief event for middle mouse button released. */
		Event_Focused,
		Event_LostFocus,
		Event_Changed,
		Event_TempOverInc,

		Event_Flags = 0xFF,
		Event_OverIdx = 0xFF00,
		Event_OverOffset = 8,

		TooltipTime=500, //Time till tooltip will appear, in milliseconds.

		MaxEvents = 16, 

		PATL = ParentAnchorTopLeft, //XML Flags: PATL
		PATC = ParentAnchorTopCenter, //XML Flags: PATC
		PATR = ParentAnchorTopRight, //XML Flags: PATR
		PAML = ParentAnchorMidLeft, //XML Flags: PAML
		PAMC = ParentAnchorMidCenter, //XML Flags: PAMC
		PAMR = ParentAnchorMidRight, //XML Flags: PAMR
		PABL = ParentAnchorBtmLeft, //XML Flags: PABL
		PABC = ParentAnchorBtmCenter, //XML Flags: PABC
		PABR = ParentAnchorBtmRight, //XML Flags: PABR

		LATL = LocalAnchorTopLeft, //XML Flags: LATL
		LATC = LocalAnchorTopCenter, //XML Flags: LATC
		LATR = LocalAnchorTopRight, //XML Flags: LATR
		LAML = LocalAnchorMidLeft, //XML Flags: LAML
		LAMC = LocalAnchorMidCenter, //XML Flags: LAMC
		LAMR = LocalAnchorMidRight, //XML Flags: LAMR
		LABL = LocalAnchorBtmLeft, //XML Flags: LABL
		LABC = LocalAnchorBtmCenter, //XML Flags: LABC
		LABR = LocalAnchorBtmRight, //XML Flags: LABR
	};

	static LWVector4f EvaluatePerPixelAttr(const LWUTF8Iterator &Value);

	static LWEXMLAttribute *FindAttribute(LWEXMLNode *Node, LWEXMLNode *Style, const LWUTF8Iterator &Name);

	static LWUTF8Iterator ParseComponentAttribute(char8_t *Buffer, uint32_t BufferSize, const LWUTF8Iterator &SrcAttribute, LWEXMLNode *Component, LWEXMLNode *ComponentNode);

	static LWEUI *XMLParseSubNodes(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode *> &StyleMap, std::map<uint32_t, LWEXMLNode *> &ComponentMap);

	/*!< \brief parse an xml node of attributes, supported attributes across all LWEUI elements:
				Name: an identifiable name for the UI element, which can be accessed through the UIManager->GetNameUI function.
				Flag: A series of flag's for the ui element, each flag is separated by a '|' symbol.
				Position: The position of the ui element, supporting both percentage distance from parent, and pixel offsets.  an example of distance is: "x: 100% + 10px y: 100% + 10px", or "x: 10px y: 100%", any combination is supported of specifying pixel vs percentage for both dimensions.
				Size: The size of the ui element, supporting both percentage size of parent container, and pixel size.  an example of size is the same as for Position attribute.
				ToolTip: Text tooltip to appear when hovering over the element for half a second(uses the manager's tooltip xml node for decorating).
				Style: Name of style to use which contains default values for parameters which simplify's decorating ui elements(these attributes will be overridden if found in the xml's node).
		*/
	static bool XMLParse(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	static bool PointInside(const LWVector2f &Pnt, float PntSize, const LWVector2f &VisiblePos, const LWVector2f &VisibleSize);

	//x,y = Visible Pos, z,w = Visible Size.
	static LWVector4f MakeVisibleBounds(uint64_t Flags, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWEUIManager &UIMan, const LWVector4f &Position, const LWVector4f &Size, float Scale);

	static LWVector4f MakeNewBounds(const LWVector4f &CurrBounds, const LWVector4f &NewBounds);

	LWVector4f Update(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, bool ParentWasVisible, uint64_t lCurrentTime);

	LWEUI &Draw(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, uint64_t lCurrentTime);

	LWEUI &DispatchEvent(uint32_t EventCode);

	bool RegisterEvent(uint32_t EventCode, LWEUIEventCallback Callback, void *UserData);

	bool UnregisterEvent(uint32_t EventCode);

	LWEUI &SetTooltip(const LWUTF8Iterator &Value, LWAllocator &Allocator);

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

	LWEUI &SetFlag(uint64_t Flag);

	LWEUI &SetVisible(bool Visible);

	LWEUI &SetFocusAble(bool FocusAble);

	LWEUI &SetTabAble(bool TabAble);

	LWEUI &UpdateOverTime(LWEUIManager &Manager, const LWVector2f &VisiblePosition, const LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) = 0;

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) = 0;

	virtual void Destroy(void);

	LWVector4f GetPosition(void) const;

	LWVector4f GetSize(void) const;

	LWVector4f GetVisibleBounds(void) const;

	LWVector2f GetVisiblePosition(void) const;

	LWVector2f GetVisibleSize(void) const;

	bool PointInside(const LWVector2f &Point, float PntSize=0.0f) const;

	bool PointInsideBounds(const LWVector2f &Point, float PntSize =0.0f) const;

	bool isVisible(void) const;

	bool isInvisible(void) const;

	bool isFocusAble(void) const;

	bool isTabAble(void) const;

	bool isDrawingAfter(void) const;

	bool isIgnoringOverCount(void) const;

	bool HasTooltip(void) const;

	const LWUTF8 &GetTooltip(void) const;

	uint64_t GetOverTime(void) const;

	uint64_t GetFlag(void) const;

	uint64_t GetTimeOver(void) const;

	LWEUI *GetFirstChild(void);

	LWEUI *GetLastChild(void);

	LWEUI *GetNext(void);

	LWEUI *GetParent(void);

	LWEUI(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

protected:
	LWEUIEvent m_EventTable[MaxEvents];
	LWUTF8 m_Tooltip;
	LWVector4f m_Position;
	LWVector4f m_Size;
	LWVector4f m_VisibleBounds;
	LWVector2f m_VisiblePosition;
	LWVector2f m_VisibleSize;
	LWEUI *m_FirstChild = nullptr;
	LWEUI *m_LastChild = nullptr;
	LWEUI *m_Next = nullptr;
	LWEUI *m_Parent = nullptr;
	uint64_t m_Flag;
	uint64_t m_TimeOver = 0;
	uint32_t m_EventCount = 0;
};

#endif