#include "LWEUI/LWEUI.h"
#include "LWEUI/LWEUIButton.h"
#include "LWEUI/LWEUILabel.h"
#include "LWEUI/LWEUIListBox.h"
#include "LWEUI/LWEUIRect.h"
#include "LWEUI/LWEUIScrollBar.h"
#include "LWEUI/LWEUITextInput.h"
#include "LWEUI/LWEUIRichLabel.h"
#include "LWEUI/LWEUITreeList.h"
#include "LWEUI/LWEUIComponent.h"
#include <LWPlatform/LWWindow.h>
#include "LWEAsset.h"
#include <iostream>
#include <algorithm>

LWVector4f LWEUI::EvaluatePerPixelAttr(const char *Value) {
	LWVector4f Val = LWVector4f(0.0f);

	auto CharIsNumber = [](char v) {
		return (v >= '0' && v <= '9') || v == '-' || v == '+' || v == '.';
	};
	const char *P = Value;
	bool x = true;
	for (const char *C = P; *C; C++) {
		if (*C == 'x') x = true;
		else if (*C == 'y') x = false;
		if (CharIsNumber(*C) && !CharIsNumber(*P)) P = C;
		if (*C == '%' || *C == 'p') {
			float v = (float)atof(P);
			if (x) {
				if (*C == '%') Val.x += v / 100.0f;
				else Val.z += v;
			} else {
				if (*C == '%') Val.y += v / 100.0f;
				else Val.w += v;
			}
			P = C;
		}
	}
	return Val;
}

LWXMLAttribute *LWEUI::FindAttribute(LWEXMLNode *Node, LWEXMLNode *Style, const LWText &Name){
	LWXMLAttribute *Res = Node->FindAttribute(Name);
	if (!Res && Style) return Style->FindAttribute(Name);
	return Res;
};

const char *LWEUI::ParseComponentAttribute(char *Buffer, uint32_t BufferSize, const char *SrcAttribute, LWEXMLNode *Component, LWEXMLNode *ComponentNode) {
	char AttributeNameBuffer[256];
	if (!ComponentNode) return SrcAttribute;
	char *B = Buffer;
	char *BufferLast = Buffer + BufferSize;
	for (const char *S = SrcAttribute; *S && B != BufferLast; S++) {
		if (*S == '\\' && *(S + 1) == '{') {
			*B++ = '{';
			S++;
		} else if (*S == '{') {
			char *A = AttributeNameBuffer;
			char *ALast = A + sizeof(AttributeNameBuffer);
			for (S++; *S && *S != '}' && A != ALast; S++) {
				*A++ = *S;
			}
			if (A == ALast) A--;
			*A = '\0';
			LWXMLAttribute *Attr = ComponentNode->FindAttribute(AttributeNameBuffer);
			if (!Attr) {
				Attr = Component->FindAttribute(AttributeNameBuffer);
				if (!Attr) continue;
			}
			const char *P = Attr->m_Value;
			for (; *P && B != BufferLast; P++) {
				*B++ = *P;
			}
		} else *B++ = *S;
	}
	if (B == BufferLast) B--;
	*B = '\0';
	return Buffer;
}

LWEUI *LWEUI::XMLParseSubNodes(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode *> &StyleMap, std::map<uint32_t, LWEXMLNode *> &ComponentMap) {
	char Buffer[256];
	char NameBuffer[256];
	uint32_t Idx = LWText::CompareMultiple(Node->m_Name, 8, "Label", "Button", "Rect", "TextInput", "ScrollBar", "ListBox", "RichLabel", "TreeList");
	LWEUI *U = nullptr;
	if (Idx == 0) U = LWEUILabel::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 1) U = LWEUIButton::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 2) U = LWEUIRect::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 3) U = LWEUITextInput::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 4) U = LWEUIScrollBar::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 5) U = LWEUIListBox::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 6) U = LWEUIRichLabel::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 7) U = LWEUITreeList::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else {
		uint32_t NameHash = LWText::MakeHash(Node->m_Name);
		auto Iter = ComponentMap.find(NameHash);
		if (Iter == ComponentMap.end()) {
			std::cout << "Error unknown node: '" << Node->m_Name << "'" << std::endl;
			return nullptr;
		}
		LWEXMLNode *Component = Iter->second;
		LWXMLAttribute *ComNameAttr = Node->FindAttribute("Name");
		LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
		LWEXMLNode *Style = nullptr;
		if (StyleAttr) {
			auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
			if (Iter != StyleMap.end()) Style = Iter->second;
		}
		NameBuffer[0] = '\0';
		const char *ActiveName = NameBuffer;
		if (ComNameAttr) {
			if (*ActiveComponentName) snprintf(NameBuffer, sizeof(NameBuffer), "%s.%s", ActiveComponentName, ParseComponentAttribute(Buffer, sizeof(Buffer), ComNameAttr->m_Value, ActiveComponent, ActiveComponentNode));
			else ActiveName = ParseComponentAttribute(Buffer, sizeof(Buffer), ComNameAttr->m_Value, ActiveComponent, ActiveComponentNode);
		} else if (*ActiveComponentName) ActiveName = ActiveComponentName;
		U = Manager->GetAllocator()->Allocate<LWEUIComponent>(LWVector4f(0.0f), LWVector4f(0.0f), 0);
		for (LWEXMLNode *C = XML->NextNode(nullptr, Component); C; C = XML->NextNode(C, Component, true)) {
			LWEUI *S = LWEUI::XMLParseSubNodes(U, C, XML, Manager, ActiveName, Component, Node, StyleMap, ComponentMap);
			if (S) ((LWEUIComponent*)U)->PushComponent(S);
		}
		LWEUI::XMLParse(U, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	}
	if (!U) return nullptr;
	Manager->InsertUIAfter(U, UI, UI?UI->GetLastChild():Manager->GetLastUI());
	return U;
}

bool LWEUI::XMLParse(LWEUI *UI, LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char NameBuffer[256];
	LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWXMLAttribute *FlagAttr = FindAttribute(Node, Style, "Flag");
	LWXMLAttribute *PosAttr = FindAttribute(Node, Style, "Position");
	LWXMLAttribute *SizeAttr = FindAttribute(Node, Style, "Size");
	LWXMLAttribute *TooltipAttr = FindAttribute(Node, Style, "Tooltip");
	const uint64_t FlagValues[] = { ParentAnchorTopLeft,   ParentAnchorTopCenter,   ParentAnchorTopRight,   ParentAnchorMidLeft,   ParentAnchorMidCenter,   ParentAnchorMidRight,   ParentAnchorBtmLeft,   ParentAnchorBtmCenter,   ParentAnchorBtmRight,   LocalAnchorTopLeft,   LocalAnchorTopCenter,   LocalAnchorTopRight,   LocalAnchorMidLeft,   LocalAnchorMidCenter,   LocalAnchorMidRight,   LocalAnchorBtmLeft,   LocalAnchorBtmCenter,   LocalAnchorBtmRight,   DrawAfter,   Invisible,  Invisible,  FocusAble,   TabAble,   FocusAble,      TabAble,      InvertAllowed,  LabelLeftAligned, LabelCenterAligned, LabelRightAligned, LabelBottomAligned, LabelVCenterAligned, LabelTopAligned,  PasswordField,   IgnoreOverCounter,   HorizontalBar,   VerticalBar,  ParentAnchorTopLeft, ParentAnchorTopCenter, ParentAnchorTopRight, ParentAnchorMidLeft, ParentAnchorMidCenter, ParentAnchorMidRight, ParentAnchorBtmLeft, ParentAnchorBtmCenter, ParentAnchorBtmRight, LocalAnchorTopLeft, LocalAnchorTopCenter, LocalAnchorTopRight, LocalAnchorMidLeft, LocalAnchorMidCenter, LocalAnchorMidRight, LocalAnchorBtmLeft, LocalAnchorBtmCenter, LocalAnchorBtmRight,  NoScalePos,   NoScaleSize, (NoScalePos | NoScaleSize), SizeToTexture, NoAutoSize, NoAutoHeightSize, NoAutoWidthSize };
	const char FlagNames[][32] = { "ParentAnchorTopLeft", "ParentAnchorTopCenter", "ParentAnchorTopRight", "ParentAnchorMidLeft", "ParentAnchorMidCenter", "ParentAnchorMidRight", "ParentAnchorBtmLeft", "ParentAnchorBtmCenter", "ParentAnchorBtmRight", "LocalAnchorTopLeft", "LocalAnchorTopCenter", "LocalAnchorTopRight", "LocalAnchorMidLeft", "LocalAnchorMidCenter", "LocalAnchorMidRight", "LocalAnchorBtmLeft", "LocalAnchorBtmCenter", "LocalAnchorBtmRight", "DrawAfter", "Invisible", "Visible", "FocusAble", "TabAble", "NotFocusable", "NotTabable", "InvertAllowed", "AlignLeft",      "AlignCenter",      "AlignRight",     "AlignBottom",       "AlignVCent",       "AlignTop",       "PasswordField", "IgnoreOverCounter", "HorizontalBar", "VerticalBar", "PATL",              "PATC",                "PATR",               "PAML",              "PAMC",                "PAMR",               "PABL",              "PABC",                "PABR",               "LATL",             "LATC",               "LATR",              "LAML",             "LAMC",               "LAMR",              "LABL",             "LABC",               "LABR",              "NoScalePos", "NoScaleSize", "NoScale",                "SizeToTexture", "NoAutoSize", "NoAutoHeightSize", "NoAutoWidthSize" };
	const uint64_t FlagCount = sizeof(FlagValues) / sizeof(uint64_t);
	LWVector4f Pos = LWVector4f(0.0f);
	LWVector4f Size = LWVector4f(0.0f);

	uint64_t Flag = UI->GetFlag();
	if (PosAttr) Pos = EvaluatePerPixelAttr(ParseComponentAttribute(Buffer, sizeof(Buffer), PosAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (SizeAttr) {
		Size = EvaluatePerPixelAttr(ParseComponentAttribute(Buffer, sizeof(Buffer), SizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
	}
	if (FlagAttr) {
		for (const char *C = ParseComponentAttribute(Buffer, sizeof(Buffer), FlagAttr->m_Value, ActiveComponent, ActiveComponentNode); *C; C++) {
			bool Found = false;
			for (uint32_t i = 0; i < FlagCount; i++) {
				if (LWText::Compare(C, FlagNames[i], (uint32_t)strlen(FlagNames[i]))) {
					Flag ^= FlagValues[i];
					Found = true;
					break;
				}
			}
			if (!Found) {
				std::cout << "Unknown flag found: '" << C << "' for: " << (char*)(NameAttr ? NameAttr->m_Value : Node->m_Name) << std::endl;
			}

			C = LWText::FirstToken(C, '|');
			if (!C) break;
		}
	}
	if (NameAttr) {
		const char *Name = ParseComponentAttribute(Buffer, sizeof(Buffer), NameAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if(*Name){
			if (*ActiveComponentName) {
				snprintf(NameBuffer, sizeof(NameBuffer), "%s.%s", ActiveComponentName, Name);
				Name = NameBuffer;
			}
			if (!Manager->InsertNamedUI(Name, UI)) {
				std::cout << "Conflict detected: " << Name << std::endl;
			}
		}
	}
	if (TooltipAttr) {
		const char *Tooltip = ParseComponentAttribute(Buffer, sizeof(Buffer), TooltipAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (*Tooltip) UI->SetTooltip(Tooltip, *Manager->GetAllocator());
	}
	UI->SetPosition(Pos).SetSize(Size).SetFlag(Flag);
	for (LWEXMLNode *C = XML->NextNode(nullptr, Node); C; C = XML->NextNode(C, Node, true)) {
		XMLParseSubNodes(UI, C, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	}

	return true;
}

//LWVector4f LWEUI::MakeVisibleBounds(uint32_t Flags, LWEUI *Parent, LWEUIManager &UIMan, const LWVector4f &Position, const LWVector4f &Size, float Scale) {
LWVector4f LWEUI::MakeVisibleBounds(uint64_t Flags, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWEUIManager &UIMan, const LWVector4f &Position, const LWVector4f &Size, float Scale){
	const LWVector2f PAnchors[] = { LWVector2f(0.0f, 1.0f), LWVector2f(0.5f, 1.0f),  LWVector2f(1.0f, 1.0f),  LWVector2f(0.0f, 0.5f),  LWVector2f(0.5f, 0.5f),  LWVector2f(1.0f, 0.5f),   LWVector2f(0.0f, 0.0f), LWVector2f(0.5f, 0.0f),  LWVector2f(1.0f, 0.0f) };
	const LWVector2f LAnchors[] = { LWVector2f(0.0f,-1.0f), LWVector2f(-0.5f,-1.0f), LWVector2f(-1.0f,-1.0f), LWVector2f(0.0f, -0.5f), LWVector2f(-0.5f, -0.5f), LWVector2f(-1.0f, -0.5f), LWVector2f(0.0f, 0.0f), LWVector2f(-0.5f, 0.0f), LWVector2f(-1.0f, 0.0f) };
	uint32_t ParentAnchor = (Flags&ParentAnchorBits) >> ParentAnchorOffsetBits;
	uint32_t LocalAnchor = (Flags&LocalAnchorBIts) >> LocalAnchorOffsetBits;
	float SP = Flags & NoScalePos ? 1.0f : Scale;
	float SS = Flags & NoScaleSize ? 1.0f : Scale;

	LWVector2f S = ParentVisibleSize * Size.xy() + Size.zw()*SS;
	LWVector2f P = ParentVisiblePos + ParentVisibleSize * PAnchors[ParentAnchor] + S * LAnchors[LocalAnchor] + ParentVisibleSize * Position.xy() + Position.zw()*SP;
	return LWVector4f(P, S);
}

LWVector4f LWEUI::MakeNewBounds(const LWVector4f &CurrBounds, const LWVector4f &NewBounds) {
	if (NewBounds.x == 0.0f && NewBounds.y == 0.0f && NewBounds.z == 0.0f && NewBounds.w == 0.0f) return CurrBounds;
	return LWVector4f(CurrBounds.xy().Min(NewBounds.xy()), CurrBounds.zw().Max(NewBounds.zw()));
}

bool LWEUI::PointInside(const LWVector2f &Pnt, float PntSize, const LWVector2f &VisiblePos, const LWVector2f &VisibleSize) {
	return Pnt.x + PntSize >= VisiblePos.x && Pnt.x - PntSize <= VisiblePos.x + VisibleSize.x && Pnt.y + PntSize >= VisiblePos.y && Pnt.y - PntSize <= VisiblePos.y + VisibleSize.y;
}

LWEUI &LWEUI::UpdateOverTime(LWEUIManager &Manager, const LWVector2f &VisiblePosition, const LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	LWEUINavigation &Navigator = Manager.GetNavigator();
	bool isNavigationEnabled = Navigator.isEnabled();
	bool isFocused = Manager.GetFocusedUI() == this;
	uint64_t Time = 0;
	if (isNavigationEnabled) {
		if (isFocused) Time = lCurrentTime;
	}
	if (Mouse) {
		if (Manager.DispatchEvent(this, LWEUI::Event_TempOverInc, PointInside(Mouse->GetPositionf()))) {
			Time = lCurrentTime;
		}
	}
	if (Touch) {
		uint32_t TouchPnts = Touch->GetPointCount();
		for (uint32_t i = 0; i < TouchPnts; i++) {
			const LWTouchPoint &T = Touch->GetPoint(i);
			if (Manager.DispatchEvent(this, LWEUI::Event_TempOverInc | (i << LWEUI::Event_OverOffset), PointInside(T.m_Position.CastTo<float>(), T.m_Size))) {
				Time = lCurrentTime;
			}
		}
	}
	m_TimeOver = Time ? (m_TimeOver ? m_TimeOver : Time) : Time;
	return *this;
}

LWVector4f LWEUI::Update(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, bool ParentWasVisible, uint64_t lCurrentTime){
	LWVector4f Bounds = MakeVisibleBounds(m_Flag, ParentVisiblePos, ParentVisibleSize, Manager, m_Position, m_Size, Scale);
	bool wasVisible = !(m_VisibleBounds.x == 0.0f && m_VisibleBounds.y == 0.0f && m_VisibleBounds.z == 0.0f && m_VisibleBounds.w == 0.0f) && ParentWasVisible;
	bool Visible = isVisible();
	bool DrawAfter = isDrawingAfter();
	if (!Visible) {
		m_VisiblePosition = m_VisibleSize = LWVector2f();
		m_VisibleBounds = LWVector4f();
		Manager.DispatchEvent(this, Event_Invisible, wasVisible);
		return m_VisibleBounds;
	}
	LWVector2f VisPosition = Bounds.xy();
	LWVector2f VisSize = Bounds.zw();
	LWVector4f VisBounds = LWVector4f(VisPosition, VisPosition+VisSize);
	
	UpdateOverTime(Manager, VisPosition, VisSize, lCurrentTime);
	Manager.GetNavigator().ProcessUI(this, VisPosition, VisSize, Manager);
	if (DrawAfter) {
		for (LWEUI *C = m_FirstChild; C; C = C->GetNext()) {
			VisBounds = MakeNewBounds(VisBounds, C->Update(Manager, Scale, VisPosition, VisSize, wasVisible, lCurrentTime));
		}
	}
	UpdateSelf(Manager, Scale, ParentVisiblePos, ParentVisibleSize, VisPosition, VisSize, lCurrentTime);
	if (!DrawAfter) {
		VisBounds = LWVector4f(VisPosition, VisPosition + VisSize);
		for (LWEUI *C = m_FirstChild; C; C = C->GetNext()) {
			VisBounds = MakeNewBounds(VisBounds, C->Update(Manager, Scale, VisPosition, VisSize, wasVisible, lCurrentTime));
		}
	}
	m_VisiblePosition = VisPosition;
	m_VisibleSize = VisSize;
	m_VisibleBounds = VisBounds;
	Manager.DispatchEvent(this, Event_Visible, !wasVisible);
	return m_VisibleBounds;
}


LWEUI &LWEUI::Draw(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, uint64_t lCurrentTime) {
	LWVector4f Bounds = MakeVisibleBounds(m_Flag, ParentVisiblePos, ParentVisibleSize, Manager, m_Position, m_Size, Scale);
	bool Visible = isVisible();
	bool DrawAfter = isDrawingAfter();
	if (!Visible) {
		m_VisiblePosition = m_VisibleSize = LWVector2f();
		return *this;
	}
	LWVector2f VisPosition = Bounds.xy();
	LWVector2f VisSize = Bounds.zw();
	if (DrawAfter) {
		for (LWEUI *C = m_FirstChild; C; C = C->GetNext()) C->Draw(Manager, Frame, Scale, VisPosition, VisSize, lCurrentTime);
	}
	DrawSelf(Manager, Frame, Scale, ParentVisiblePos, ParentVisibleSize, VisPosition, VisSize, lCurrentTime);
	if (!DrawAfter) {
		for (LWEUI *C = m_FirstChild; C; C = C->GetNext()) C->Draw(Manager, Frame, Scale, VisPosition, VisSize, lCurrentTime);
	}
	m_VisiblePosition = VisPosition;
	m_VisibleSize = VisSize;
	return *this;
}

LWEUI &LWEUI::DispatchEvent(uint32_t EventCode) {
	for (uint32_t i = 0; i < m_EventCount; i++) {
		if (m_EventTable[i].m_EventCode == EventCode) {
			m_EventTable[i].m_Callback(this, EventCode, m_EventTable[i].m_UserData);
			return *this;
		}
	}
	return *this;
}

bool LWEUI::RegisterEvent(uint32_t EventCode, LWEUIEventCallback Callback, void *UserData) {
	if (m_EventCount >= MaxEvents) return false;
	m_EventTable[m_EventCount] = { Callback, EventCode, UserData };
	m_EventCount++;
	return true;
}

bool LWEUI::UnregisterEvent(uint32_t EventCode) {
	for (uint32_t i = 0; i < m_EventCount; i++) {
		if (m_EventTable[i].m_EventCode == EventCode) {
			for (uint32_t d = i + 1; d < m_EventCount; d++) {
				m_EventTable[d - 1] = m_EventTable[d];
			}
			m_EventCount--;
			return true;
		}
	}
	return false;
}

LWEUI &LWEUI::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	return *this;
}

LWEUI &LWEUI::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {

	return *this;
}

void LWEUI::Destroy(void) {
	LWAllocator::Destroy(this);
}

LWEUI &LWEUI::SetTooltip(const LWText &Value, LWAllocator &Allocator) {
	m_Tooltip = LWText(Value, Allocator);
	return *this;
}

LWEUI &LWEUI::SetTooltipf(LWAllocator &Allocator, const char *Fmt, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	m_Tooltip = LWText(Buffer, Allocator);
	return *this;
}

LWEUI &LWEUI::SetFirstChild(LWEUI *UI) {
	m_FirstChild = UI;
	return *this;
}

LWEUI &LWEUI::SetLastChild(LWEUI *UI) {
	m_LastChild = UI;
	return *this;
}

LWEUI &LWEUI::SetNext(LWEUI *UI) {
	m_Next = UI;
	return *this;
}

LWEUI &LWEUI::SetParent(LWEUI *Parent) {
	m_Parent = Parent;
	return *this;
}

LWEUI &LWEUI::SetVisiblePosition(const LWVector2f &VisPos) {
	m_VisiblePosition = VisPos;
	return *this;
}

LWEUI &LWEUI::SetVisibleSize(const LWVector2f &VisSize) {
	m_VisibleSize = VisSize;
	return *this;
}

LWEUI &LWEUI::SetPosition(const LWVector4f &Position) {
	m_Position = Position;
	return *this;
}

LWEUI &LWEUI::SetSize(const LWVector4f &Size) {
	m_Size = Size;
	return *this;
}

LWEUI &LWEUI::SetVisible(bool Visible) {
	m_Flag = (m_Flag&~LWEUI::Invisible) | (Visible ? 0 : LWEUI::Invisible);
	return *this;
}

LWEUI &LWEUI::SetFocusAble(bool FocusAble) {
	m_Flag = (m_Flag&~LWEUI::FocusAble) | (FocusAble ? LWEUI::FocusAble : 0);
	return *this;
}

LWEUI &LWEUI::SetTabAble(bool TabAble) {
	m_Flag = (m_Flag&~LWEUI::TabAble) | (TabAble ? LWEUI::TabAble : 0);
	return *this;
}

LWEUI &LWEUI::SetFlag(uint64_t Flag) {
	m_Flag = Flag;
	return *this;
}

LWVector4f LWEUI::GetPosition(void) const {
	return m_Position;
}

LWVector4f LWEUI::GetSize(void) const {
	return m_Size;
}

bool LWEUI::PointInside(const LWVector2f &Point, float PntSize) const {
	return PointInside(Point, PntSize, m_VisiblePosition, m_VisibleSize);
}

bool LWEUI::PointInsideBounds(const LWVector2f &Point, float PntSize) const {
	return PointInside(Point, PntSize, m_VisibleBounds.xy(), m_VisibleBounds.zw() - m_VisibleBounds.xy());
}

LWVector4f LWEUI::GetVisibleBounds(void) const {
	return m_VisibleBounds;
}

LWVector2f LWEUI::GetVisiblePosition(void) const {
	return m_VisiblePosition;
}

LWVector2f LWEUI::GetVisibleSize(void) const {
	return m_VisibleSize;
}

bool LWEUI::isVisible(void) const {
	return (m_Flag&LWEUI::Invisible) == 0;
}

bool LWEUI::isInvisible(void) const {
	return (m_Flag&LWEUI::Invisible) != 0;
}

bool LWEUI::isFocusAble(void) const {
	return (m_Flag&LWEUI::FocusAble) != 0;
}

bool LWEUI::isTabAble(void) const {
	return (m_Flag&LWEUI::TabAble) != 0;
}

bool LWEUI::isDrawingAfter(void) const {
	return (m_Flag&LWEUI::DrawAfter) != 0;
}

bool LWEUI::isIgnoringOverCount(void) const {
	return (m_Flag&LWEUI::IgnoreOverCounter) != 0;
}

bool LWEUI::HasTooltip(void) const {
	return m_Tooltip.GetLength() > 0;
}

const LWText &LWEUI::GetTooltip(void) const {
	return m_Tooltip;
}

uint64_t LWEUI::GetOverTime(void) const {
	return m_TimeOver;
}

uint64_t LWEUI::GetFlag(void) const {
	return m_Flag;
}

uint64_t LWEUI::GetTimeOver(void) const {
	return m_TimeOver;
}

LWEUI *LWEUI::GetFirstChild(void) {
	return m_FirstChild;
}

LWEUI *LWEUI::GetLastChild(void) {
	return m_LastChild;
}

LWEUI *LWEUI::GetNext(void) {
	return m_Next;
}

LWEUI *LWEUI::GetParent(void) {
	return m_Parent;
}

LWEUI::LWEUI(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : m_Position(Position), m_Size(Size), m_FirstChild(nullptr), m_LastChild(nullptr), m_Next(nullptr), m_Parent(nullptr), m_Flag(Flag), m_VisibleBounds(LWVector4f()), m_TimeOver(0), m_EventCount(0) {}

