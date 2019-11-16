#include "LWEUI/LWEUI.h"
#include "LWEUI/LWEUIButton.h"
#include "LWEUI/LWEUILabel.h"
#include "LWEUI/LWEUIListBox.h"
#include "LWEUI/LWEUIRect.h"
#include "LWEUI/LWEUIScrollBar.h"
#include "LWEUI/LWEUITextInput.h"
#include "LWEUI/LWEUIAdvLabel.h"
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

void *LWEUI::FindAsset(LWEAssetManager *AM, const LWText &Name, uint32_t Type){
	LWEAsset *A = AM->GetAsset(Name);
	if (!A || A->GetType() != Type) return nullptr;
	return A->GetAsset();
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
	uint32_t Idx = LWText::CompareMultiple(Node->m_Name, 7, "Label", "Button", "Rect", "TextInput", "ScrollBar", "ListBox", "AdvLabel");
	LWEUI *U = nullptr;
	if (Idx == 0) U = LWEUILabel::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 1) U = LWEUIButton::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 2) U = LWEUIRect::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 3) U = LWEUITextInput::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 4) U = LWEUIScrollBar::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 5) U = LWEUIListBox::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	else if (Idx == 6) U = LWEUIAdvLabel::XMLParse(Node, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
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
			LWEUI *SubU = LWEUI::XMLParseSubNodes(U, C, XML, Manager, ActiveName, Component, Node, StyleMap, ComponentMap);
			if (SubU) ((LWEUIComponent*)U)->PushComponent(SubU);
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
	const uint32_t FlagValues[] = { ParentAnchorTopLeft,   ParentAnchorTopCenter,   ParentAnchorTopRight,   ParentAnchorMidLeft,   ParentAnchorMidCenter,   ParentAnchorMidRight,   ParentAnchorBtmLeft,   ParentAnchorBtmCenter,   ParentAnchorBtmRight,   LocalAnchorTopLeft,   LocalAnchorTopCenter,   LocalAnchorTopRight,   LocalAnchorMidLeft,   LocalAnchorMidCenter,   LocalAnchorMidRight,   LocalAnchorBtmLeft,   LocalAnchorBtmCenter,   LocalAnchorBtmRight,   DrawAfter,   Invisible,  Invisible,  FocusAble,   TabAble,   FocusAble,      TabAble,      InvertAllowed,  LabelLeftAligned, LabelCenterAligned, LabelRightAligned, PasswordField,   IgnoreOverCounter,   HorizontalBar,   VerticalBar,  ParentAnchorTopLeft, ParentAnchorTopCenter, ParentAnchorTopRight, ParentAnchorMidLeft, ParentAnchorMidCenter, ParentAnchorMidRight, ParentAnchorBtmLeft, ParentAnchorBtmCenter, ParentAnchorBtmRight, LocalAnchorTopLeft, LocalAnchorTopCenter, LocalAnchorTopRight, LocalAnchorMidLeft, LocalAnchorMidCenter, LocalAnchorMidRight, LocalAnchorBtmLeft, LocalAnchorBtmCenter, LocalAnchorBtmRight,  NoScalePos,   NoScaleSize, (NoScalePos | NoScaleSize), SizeToTexture, NoAutoSize };
	const char FlagNames[][32] = { "ParentAnchorTopLeft", "ParentAnchorTopCenter", "ParentAnchorTopRight", "ParentAnchorMidLeft", "ParentAnchorMidCenter", "ParentAnchorMidRight", "ParentAnchorBtmLeft", "ParentAnchorBtmCenter", "ParentAnchorBtmRight", "LocalAnchorTopLeft", "LocalAnchorTopCenter", "LocalAnchorTopRight", "LocalAnchorMidLeft", "LocalAnchorMidCenter", "LocalAnchorMidRight", "LocalAnchorBtmLeft", "LocalAnchorBtmCenter", "LocalAnchorBtmRight", "DrawAfter", "Invisible", "Visible", "FocusAble", "TabAble", "NotFocusable", "NotTabable", "InvertAllowed", "AlignLeft",      "AlignCenter",      "AlignRight",     "PasswordField", "IgnoreOverCounter", "HorizontalBar", "VerticalBar", "PATL",              "PATC",                "PATR",               "PAML",              "PAMC",                "PAMR",               "PABL",              "PABC",                "PABR",               "LATL",             "LATC",               "LATR",              "LAML",             "LAMC",               "LAMR",              "LABL",             "LABC",               "LABR",              "NoScalePos", "NoScaleSize", "NoScale",                "SizeToTexture", "NoAutoSize" };
	const uint32_t FlagCount = sizeof(FlagValues) / sizeof(uint32_t);
	LWVector4f Pos = LWVector4f(0.0f);
	LWVector4f Size = LWVector4f(0.0f);
	uint32_t Flag = UI->GetFlag();
	if (PosAttr) Pos = EvaluatePerPixelAttr(ParseComponentAttribute(Buffer, sizeof(Buffer), PosAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (SizeAttr) Size = EvaluatePerPixelAttr(ParseComponentAttribute(Buffer, sizeof(Buffer), SizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
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
	UI->SetPosition(Pos).SetSize(Size).SetFlag(Flag);
	for (LWEXMLNode *C = XML->NextNode(nullptr, Node); C; C = XML->NextNode(C, Node, true)) {
		XMLParseSubNodes(UI, C, XML, Manager, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	}

	return true;
}

bool LWEUI::PointInside(const LWVector2f &Pnt, float PntSize, const LWVector2f &VisiblePos, const LWVector2f &VisibleSize) {
	return Pnt.x + PntSize >= VisiblePos.x && Pnt.x - PntSize <= VisiblePos.x + VisibleSize.x && Pnt.y + PntSize >= VisiblePos.y && Pnt.y - PntSize <= VisiblePos.y + VisibleSize.y;
}

LWVector4f LWEUI::Update(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	LWVector2f ParentPosition = Manager->GetVisiblePosition();
	LWVector2f ParentSize = Manager->GetVisibleSize();
	if (m_Parent) {
		ParentPosition = m_Parent->GetVisiblePosition();
		ParentSize = m_Parent->GetVisibleSize();
	}
	uint32_t ParentAnchor = (m_Flag&ParentAnchorBits) >> ParentAnchorOffsetBits;
	uint32_t LocalAnchor = (m_Flag&LocalAnchorBIts) >> LocalAnchorOffsetBits;
	LWVector2f PAnchors[] = { LWVector2f(0.0f, 1.0f), LWVector2f(0.5f, 1.0f),  LWVector2f(1.0f, 1.0f),  LWVector2f(0.0f, 0.5f),  LWVector2f(0.5f, 0.5f),  LWVector2f(1.0f, 0.5f),   LWVector2f(0.0f, 0.0f), LWVector2f(0.5f, 0.0f),  LWVector2f(1.0f, 0.0f) };
	LWVector2f CAnchors[] = { LWVector2f(0.0f,-1.0f), LWVector2f(-0.5f,-1.0f), LWVector2f(-1.0f,-1.0f), LWVector2f(0.0f, -0.5f), LWVector2f(-0.5f, -0.5f), LWVector2f(-1.0f, -0.5f), LWVector2f(0.0f, 0.0f), LWVector2f(-0.5f, 0.0f), LWVector2f(-1.0f, 0.0f) };
	float SP = m_Flag&NoScalePos ? 1.0f : Scale;
	float SS = m_Flag&NoScaleSize ? 1.0f : Scale;
	
	m_VisibleSize = ParentSize*LWVector2f(m_Size.x, m_Size.y) + LWVector2f(m_Size.z, m_Size.w)*SS;
	m_VisiblePosition = ParentPosition + ParentSize*PAnchors[ParentAnchor] + m_VisibleSize*CAnchors[LocalAnchor] + ParentSize*LWVector2f(m_Position.x, m_Position.y) + LWVector2f(m_Position.z, m_Position.w)*SP;
	
	LWVector4f VisBounds = LWVector4f(m_VisiblePosition, m_VisiblePosition + m_VisibleSize);
	auto UpdateWithBounds = [](LWEUI *N, LWEUIManager *Manager, float Scale, uint64_t lCurrentTime, LWVector4f &Bounds, bool First)->LWVector4f{
		if (!N) return Bounds;
		LWVector4f NBounds = N->Update(Manager, Scale, lCurrentTime);
		if (First) Bounds = NBounds;
		else {
			if (NBounds.x == 0.0f && NBounds.y == 0.0f && NBounds.z == 0.0f && NBounds.w == 0.0f) return Bounds;
			Bounds.x = std::min<float>(Bounds.x, NBounds.x);
			Bounds.y = std::min<float>(Bounds.y, NBounds.y);
			Bounds.z = std::max<float>(Bounds.z, NBounds.z);
			Bounds.w = std::max<float>(Bounds.w, NBounds.w);
		}
		return Bounds;
	};


	if (m_Flag&Invisible) {
		m_VisibleBounds = LWVector4f();
		VisBounds = LWVector4f();
		return UpdateWithBounds(m_Next, Manager, Scale, lCurrentTime, VisBounds, true);
	}
	if ((m_Flag&DrawAfter) != 0) UpdateWithBounds(m_FirstChild, Manager, Scale, lCurrentTime, VisBounds, false);
	
	if (m_Flag&VisibilityChange) {
		m_Flag &= ~VisibilityChange;
		Manager->DispatchEvent(this, (m_Flag&Invisible) ? Event_Invisible : Event_Visible);
	}

	UpdateSelf(Manager, Scale, lCurrentTime);
	if ((m_Flag&DrawAfter) == 0) UpdateWithBounds(m_FirstChild, Manager, Scale, lCurrentTime, VisBounds, false);
	m_VisibleBounds = VisBounds;
	return UpdateWithBounds(m_Next, Manager, Scale, lCurrentTime, VisBounds, false);
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

bool LWEUI::RegisterEvent(uint32_t EventCode, std::function<void(LWEUI*, uint32_t, void*)> Callback, void *UserData) {
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

LWEUI &LWEUI::Draw(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	if (m_Flag&Invisible) return m_Next ? m_Next->Draw(Manager, Frame, Scale, lCurrentTime) : *this;
	if ((m_Flag&DrawAfter) != 0) if (m_FirstChild) m_FirstChild->Draw(Manager, Frame, Scale, lCurrentTime);
	DrawSelf(Manager, Frame, Scale, lCurrentTime);
	if ((m_Flag&DrawAfter) == 0) if (m_FirstChild) m_FirstChild->Draw(Manager, Frame, Scale, lCurrentTime);
	return m_Next ? m_Next->Draw(Manager, Frame, Scale, lCurrentTime) : *this;
}

LWEUI &LWEUI::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	return *this;
}

LWEUI &LWEUI::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {

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

LWEUI &LWEUI::SetFlag(uint32_t Flag) {
	if ((m_Flag&Invisible) && (Flag&Invisible) == 0) Flag |= VisibilityChange;
	if ((m_Flag&Invisible) == 0 && (Flag&Invisible)) Flag |= VisibilityChange;
	m_Flag = Flag;

	return *this;
}

LWVector4f LWEUI::GetPosition(void) const {
	return m_Position;
}

LWVector4f LWEUI::GetSize(void) const {
	return m_Size;
}

bool LWEUI::PointInside(const LWVector2f &Point) const {
	return Point.x >= m_VisiblePosition.x && Point.x <= m_VisiblePosition.x + m_VisibleSize.x && Point.y >= m_VisiblePosition.y && Point.y <= m_VisiblePosition.y + m_VisibleSize.y;
}

bool LWEUI::PointInsideBounds(const LWVector2f &Point) const {
	return Point.x >= m_VisibleBounds.x && Point.x <= m_VisibleBounds.z && Point.y >= m_VisibleBounds.y && Point.y <= m_VisibleBounds.w;
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

bool LWEUI::GetVisible(void) const {
	return (m_Flag&LWEUI::Invisible) == 0;
}

uint32_t LWEUI::GetFlag(void) const {
	return m_Flag;
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

LWEUI::LWEUI(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : m_Position(Position), m_Size(Size), m_FirstChild(nullptr), m_LastChild(nullptr), m_Next(nullptr), m_Parent(nullptr), m_Flag(Flag), m_VisibleBounds(LWVector4f()), m_EventCount(0) {}

