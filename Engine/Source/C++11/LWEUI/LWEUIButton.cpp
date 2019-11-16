#include "LWEUI/LWEUIButton.h"
#include "LWPlatform/LWWindow.h"
#include "LWELocalization.h"
#include <iostream>

LWEUIButton *LWEUIButton::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIButton *Button = Allocator->Allocate<LWEUIButton>(nullptr, nullptr, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), FocusAble | TabAble);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style"); 
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
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

LWEUI &LWEUIButton::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager->GetWindow();

	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();

	uint32_t Flag = m_Flag;
	m_Flag &= ~(MouseOver | MouseDown);

	if (Mouse) {
		LWVector2i MP = Mouse->GetPosition();
		LWVector2f MousePnt = LWVector2f((float)MP.x, (float)MP.y);
		if (MousePnt.x >= m_VisiblePosition.x && MousePnt.x <= m_VisiblePosition.x + m_VisibleSize.x && MousePnt.y >= m_VisiblePosition.y && MousePnt.y <= m_VisiblePosition.y + m_VisibleSize.y) m_Flag |= MouseOver;
		if (m_Flag&MouseOver) {
			Manager->DispatchEvent(this, Event_TempOverInc);
			if (Mouse->ButtonDown(LWMouseKey::Left)) m_Flag |= MouseDown;
		}
	}

	if (Touch) {
		for (uint32_t i = 0; i < Touch->GetPointCount(); i++) {
			auto Pnt = Touch->GetPoint(i);
			LWVector2f TouchPnt = LWVector2f((float)Pnt->m_Position.x, (float)Pnt->m_Position.y);
            float TouchSize = Pnt->m_Size*Scale;
			bool Over = TouchPnt.x + TouchSize >= m_VisiblePosition.x && TouchPnt.x - TouchSize <= m_VisiblePosition.x + m_VisibleSize.x && TouchPnt.y + TouchSize >= m_VisiblePosition.y && TouchPnt.y - TouchSize <= m_VisiblePosition.y + m_VisibleSize.y;
			if (Over) {
				m_Flag |= MouseOver;
				Manager->DispatchEvent(this, Event_TempOverInc | (i << Event_OverOffset));
				if (Pnt->m_State != LWTouchPoint::UP) m_Flag |= MouseDown;
			}
		}
	}
	if (m_Flag&MouseOver && (Flag&MouseOver) == 0) Manager->DispatchEvent(this, Event_MouseOver);
	if ((m_Flag&MouseOver) == 0 && Flag&MouseOver) Manager->DispatchEvent(this, Event_MouseOff);
	if ((m_Flag&(MouseDown | MouseOver)) == MouseOver && Flag&MouseDown) {
		Manager->DispatchEvent(this, Event_Released);
		if (m_Flag&FocusAble) Manager->SetFocused(this);
	}
	if ((m_Flag&(MouseDown | MouseOver)) == (MouseDown | MouseOver) && (Flag&MouseDown) == 0) {
		Manager->DispatchEvent(this, Event_Pressed);
	}
	return *this;
}

LWEUI &LWEUIButton::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	auto DrawRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, LWEUIFrame *F)->bool {
		if (!Mat) return false;
		if (!F->SetActiveTexture(Mat->m_Texture, false)) return false;
		LWVector4f SubRegion = Mat->m_SubRegion;

		uint32_t c = LWVertexUI::WriteRectangle(F->m_Mesh, Pos + LWVector2f(0.0f, Size.y), Pos + LWVector2f(Size.x, 0.0f), Mat->m_Color, LWVector2f(SubRegion.x, SubRegion.y), LWVector2f(SubRegion.z, SubRegion.w));
		F->m_VertexCount[F->m_TextureCount - 1] += c;
		return c != 0;
	};

	LWEUIMaterial *ActiveMaterial = m_OffMaterial;
	if (m_Flag&MouseOver) ActiveMaterial = (m_Flag&MouseDown) ? m_DownMaterial : m_OverMaterial;
	DrawRect(ActiveMaterial, m_VisiblePosition, m_VisibleSize, Frame);

	return *this;
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

LWEUIButton::LWEUIButton(LWEUIMaterial *OverMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *DownMaterial, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_OverMaterial(OverMaterial), m_OffMaterial(OffMaterial), m_DownMaterial(DownMaterial) {}

LWEUIButton::~LWEUIButton() {}