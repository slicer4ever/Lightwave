#include "LWEUI/LWEUIButton.h"
#include "LWPlatform/LWWindow.h"
#include "LWELocalization.h"
#include <iostream>

LWEUIButton *LWEUIButton::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIButton *Button = Allocator->Allocate<LWEUIButton>(nullptr, nullptr, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), FocusAble | TabAble);
	LWEUI::XMLParse(Button, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent,  ActiveComponentNode, StyleMap, ComponentMap);
	LWXMLAttribute *OverAttr = LWEUI::FindAttribute(Node, Style, "OverMaterial");
	LWXMLAttribute *DownAttr = LWEUI::FindAttribute(Node, Style, "DownMaterial");
	LWXMLAttribute *OffAttr = LWEUI::FindAttribute(Node, Style, "OffMaterial");
	LWEUIMaterial *OverMat = nullptr;
	LWEUIMaterial *DownMat = nullptr;
	LWEUIMaterial *OffMat = nullptr;

	if (OverAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), OverAttr->m_Value, ActiveComponent, ActiveComponentNode);
		OverMat = Manager->GetMaterial(Res);
	}
	if (DownAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), DownAttr->m_Value, ActiveComponent, ActiveComponentNode);
		DownMat = Manager->GetMaterial(Res);
	}
	if (OffAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), OffAttr->m_Value, ActiveComponent, ActiveComponentNode);
		OffMat = Manager->GetMaterial(Res);
	}
	Button->SetOverMaterial(OverMat).SetDownMaterial(DownMat).SetOffMaterial(OffMat);

	return Button;
}

LWEUI &LWEUIButton::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager.GetWindow();
	LWEUINavigation &Navigator = Manager.GetNavigator();
	bool isNavigationEnabled = Navigator.isEnabled();
	bool isFocused = Manager.GetFocusedUI() == this;

	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();

	bool wasOver = (m_Flag&MouseOver);
	bool wasLDown = (m_Flag&MouseLDown);
	bool wasRDown = (m_Flag&MouseRDown);
	bool wasMDown = (m_Flag&MouseMDown);
	uint64_t Flag = (m_Flag&~(MouseOver | MouseLDown | MouseRDown | MouseMDown)) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (Flag&MouseOver);

	if (isNavigationEnabled) {
		if (isFocused) Flag |= (Navigator.isPressed() ? MouseLDown : 0);
	}

	if (Mouse) {
		if (Mouse->ButtonDown(LWMouseKey::Left)) Flag |= isOver ? MouseLDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Right)) Flag |= isOver ? MouseRDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Middle)) Flag |= isOver ? MouseMDown : 0;
	}
	if (Touch) {
		uint32_t TouchCnt = Touch->GetPointCount();
		for (uint32_t i = 0; i < TouchCnt; i++) {
			const LWTouchPoint &T = Touch->GetPoint(i);
			bool Over = PointInside(T.m_Position.CastTo<float>(), T.m_Size*Scale);
			if (T.m_State != LWTouchPoint::UP) Flag |= Over ? MouseLDown : 0;
		}
	}
	bool isLDown = (Flag&MouseLDown);
	bool isRDown = (Flag&MouseRDown);
	bool isMDown = (Flag&MouseMDown);
	bool isFocusable = (m_Flag&FocusAble);
	m_Flag = Flag;
	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	Manager.DispatchEvent(this, Event_Released, isOver && (!isLDown && wasLDown));
	Manager.DispatchEvent(this, Event_RReleased, isOver && (!isRDown && wasRDown));
	Manager.DispatchEvent(this, Event_MReleased, isOver && (!isMDown && wasMDown));
	if (Manager.DispatchEvent(this, Event_Pressed, isOver && isLDown && !wasLDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_RPressed, isOver && isRDown && !wasRDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_MPressed, isOver && isMDown && !wasMDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	return *this;
}

LWEUI &LWEUIButton::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	bool isOver = (m_Flag&MouseOver) != 0;
	bool isDown = (m_Flag&(MouseLDown | MouseMDown | MouseRDown)) != 0;
	LWEUIMaterial *Material = m_OffMaterial;
	if (isOver) Material = isDown ? m_DownMaterial : m_OverMaterial;
	Frame.WriteRect(Material, VisiblePos, VisibleSize);
	return *this;
}

void LWEUIButton::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUIButton &LWEUIButton::SetOverMaterial(LWEUIMaterial *OverMaterial) {
	m_OverMaterial = OverMaterial;
	return *this;
}

LWEUIButton &LWEUIButton::SetOffMaterial(LWEUIMaterial *OffMaterial) {
	m_OffMaterial = OffMaterial;
	return *this;
}

LWEUIButton &LWEUIButton::SetDownMaterial(LWEUIMaterial *DownMaterial) {
	m_DownMaterial = DownMaterial;
	return *this;
}

LWEUIMaterial *LWEUIButton::GetOverMaterial(void) {
	return m_OverMaterial;
}

LWEUIMaterial *LWEUIButton::GetOffMaterial(void) {
	return m_OffMaterial;
}

LWEUIMaterial *LWEUIButton::GetDownMaterial(void) {
	return m_DownMaterial;
}

LWEUIButton::LWEUIButton(LWEUIMaterial *OverMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *DownMaterial, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_OverMaterial(OverMaterial), m_OffMaterial(OffMaterial), m_DownMaterial(DownMaterial) {}

LWEUIButton::~LWEUIButton() {}