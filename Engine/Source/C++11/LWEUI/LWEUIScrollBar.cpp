#include "LWEUI/LWEUIScrollBar.h"
#include "LWELocalization.h"
#include <algorithm>
#include <LWPlatform/LWWindow.h>

LWEUIScrollBar *LWEUIScrollBar::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIScrollBar *ScrollBar = Allocator->Allocate<LWEUIScrollBar>(nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, LWVector4f(0.0f), LWVector4f(0.0f), FocusAble);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
	LWEUI::XMLParse(ScrollBar, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWXMLAttribute *OverAttr = FindAttribute(Node, Style, "OverMaterial");
	LWXMLAttribute *DownAttr = FindAttribute(Node, Style, "DownMaterial");
	LWXMLAttribute *OffAttr = FindAttribute(Node, Style, "OffMaterial");
	LWXMLAttribute *BackAttr = FindAttribute(Node, Style, "BackgroundMaterial");
	LWXMLAttribute *ScrollAttr = FindAttribute(Node, Style, "Scroll");
	LWXMLAttribute *MaxScrollAttr = FindAttribute(Node, Style, "MaxScroll");
	LWXMLAttribute *ScrollSizeAttr = FindAttribute(Node, Style, "ScrollSize");
	LWEUIMaterial *OverMat = nullptr;
	LWEUIMaterial *DownMat = nullptr;
	LWEUIMaterial *OffMat = nullptr;
	LWEUIMaterial *BackMat = nullptr;
	float Scroll = 0.0f;
	float MaxScroll = 0.0f;
	float ScrollSize = 0.0f;

	if (OverAttr) OverMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), OverAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (DownAttr) DownMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), DownAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (OffAttr) OffMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), OffAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BackAttr) BackMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BackAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (ScrollAttr) Scroll = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), ScrollAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (MaxScrollAttr) MaxScroll = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), MaxScrollAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (ScrollSizeAttr) ScrollSize = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), ScrollSizeAttr->m_Value, ActiveComponent, ActiveComponentNode));

	ScrollBar->SetBarOffMaterial(OffMat).SetBarOverMaterial(OverMat).SetBarDownMaterial(DownMat).SetBackgroundMaterial(BackMat);
	ScrollBar->SetMaxScroll(MaxScroll).SetScrollSize(ScrollSize).SetScroll(Scroll);
	return ScrollBar;
}

LWEUI &LWEUIScrollBar::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	LWVector2f DownPnt = LWVector2f(-100.0f, -100.0f);
	float DownSize = 0.0f;
	float InitScroll = m_Scroll;
	uint32_t Flag = m_Flag;
	m_Flag &= ~(MouseOver);

	if (Mouse) DownPnt = Mouse->GetPositionf();
	if (Touch) {
		const LWGesture &Gest = Touch->GetGesture();
		//std::cout << "Gesture: " << Gest.m_Type << std::endl;
		if (Gest.m_Type == LWGesture::Drag || Gest.m_Type==LWGesture::PressAndDrag) {
			DownPnt = LWVector2f((float)Gest.m_Source.x, (float)Gest.m_Source.y)+LWVector2f((float)Gest.m_Direction.x, (float)Gest.m_Direction.y);
            DownSize = std::max<float>(Gest.m_Scale*Scale*2.0f, 5.0f); //set's minimum size for finger.
		}
	}

	LWVector2f BarVisiblePos;
	LWVector2f BarVisibleSize;
	if (m_Flag&HorizontalBar) {
		BarVisiblePos = LWVector2f(m_VisiblePosition.x + m_VisibleSize.x*(m_Scroll / m_MaxScroll), m_VisiblePosition.y);
		BarVisibleSize = LWVector2f(m_VisibleSize.x*(m_ScrollSize / m_MaxScroll), m_VisibleSize.y);
	} else {
		BarVisibleSize = LWVector2f(m_VisibleSize.x, m_VisibleSize.y*(m_ScrollSize / m_MaxScroll));
		BarVisiblePos = LWVector2f(m_VisiblePosition.x, m_VisiblePosition.y + (m_VisibleSize.y - BarVisibleSize.y) - m_VisibleSize.y*(m_Scroll / m_MaxScroll));
	}
	bool OverAll = DownPnt.x+DownSize >= m_VisiblePosition.x && DownPnt.x-DownSize <= m_VisiblePosition.x + m_VisibleSize.x && DownPnt.y+DownSize >= m_VisiblePosition.y && DownPnt.y-DownSize <= m_VisiblePosition.y + m_VisibleSize.y;
	bool OverBar = DownPnt.x+DownSize >= BarVisiblePos.x && DownPnt.x-DownSize <= BarVisiblePos.x + BarVisibleSize.x && DownPnt.y+DownSize >= BarVisiblePos.y && DownPnt.y-DownSize <= BarVisiblePos.y + BarVisibleSize.y;

	if (OverBar) m_Flag |= MouseOver;

	if (Mouse) {
		if (OverAll) {
			if (Mouse->GetScroll() > 0) SetScroll(m_Scroll - m_ScrollSize*0.25f);
			else if (Mouse->GetScroll() < 0) SetScroll(m_Scroll + m_ScrollSize*0.25f);
		}

		if (Mouse->ButtonDown(LWMouseKey::Left)) {
			if (m_Flag&MouseOver) {
				m_Flag |= MouseDown;
			}
		} else m_Flag &= ~MouseDown;
	}
	if (Touch) {
		if (OverBar) m_Flag |= MouseDown;
		else m_Flag &= ~MouseDown;
	}

	if (OverAll) Manager->DispatchEvent(this, Event_TempOverInc);
	if (m_Flag&MouseOver && (Flag&MouseOver) == 0) Manager->DispatchEvent(this, Event_MouseOver);
	if ((m_Flag&MouseOver) == 0 && Flag&MouseOver) Manager->DispatchEvent(this, Event_MouseOff);
	if ((m_Flag&(MouseDown | MouseOver)) == MouseOver && Flag&MouseDown) {
		Manager->DispatchEvent(this, Event_Released);
		m_InitialScroll = m_Scroll;
		if (m_Flag&FocusAble) Manager->SetFocused(this);
	}
	if ((m_Flag&(MouseDown | MouseOver)) == (MouseDown | MouseOver) && (Flag&MouseDown) == 0) {
		Manager->DispatchEvent(this, Event_Pressed);
	}
	if (m_Flag&MouseDown) {
		float BarPos = (m_Flag&HorizontalBar ? (DownPnt.x - m_VisiblePosition.x) : ((m_VisiblePosition.y + m_VisibleSize.y) - DownPnt.y));
		if (fabs(((m_Flag&HorizontalBar) ? (DownPnt.y - m_VisiblePosition.y) : (DownPnt.x - m_VisiblePosition.x))) > ((m_Flag&HorizontalBar) ? m_VisibleSize.y*4.0f+DownSize : m_VisibleSize.x*4.0f+DownSize)) {
			SetScroll(m_InitialScroll);
		} else {
			SetScroll(((m_Flag&HorizontalBar) ? (BarPos - BarVisibleSize.x*0.5f) * (m_MaxScroll / m_VisibleSize.x) : (BarPos - BarVisibleSize.y*0.5f) * (m_MaxScroll / (m_VisibleSize.y))));
		}
	}
	if (m_Scroll != InitScroll) Manager->DispatchEvent(this, Event_Changed);
	return *this;
}

LWEUI &LWEUIScrollBar::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	auto DrawRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, LWEUIFrame *F)->bool {
		if (!Mat) return false;
		if (!F->SetActiveTexture(Mat->m_Texture, false)) return false;
		LWVector4f SubRegion = Mat->m_SubRegion;

		uint32_t c = LWVertexUI::WriteRectangle(F->m_Mesh, Pos + LWVector2f(0.0f, Size.y), Pos + LWVector2f(Size.x, 0.0f), Mat->m_Color, LWVector2f(SubRegion.x, SubRegion.y), LWVector2f(SubRegion.z, SubRegion.w));
		F->m_VertexCount[F->m_TextureCount - 1] += c;
		return c != 0;
	};

	LWEUIMaterial *ActiveMaterial = m_BarOffMaterial;
	if (m_Flag&MouseOver) ActiveMaterial = (m_Flag&MouseDown) ? m_BarDownMaterial : m_BarOverMaterial;
	DrawRect(m_BackgroundMaterial, m_VisiblePosition, m_VisibleSize, Frame);

	LWVector2f BarVisiblePos;
	LWVector2f BarVisibleSize;
	if (m_Flag&HorizontalBar) {
		BarVisiblePos = LWVector2f(m_VisiblePosition.x + m_VisibleSize.x*(m_Scroll / m_MaxScroll), m_VisiblePosition.y);
		BarVisibleSize = LWVector2f(m_VisibleSize.x*(m_ScrollSize / m_MaxScroll), m_VisibleSize.y);
	} else {
		BarVisibleSize = LWVector2f(m_VisibleSize.x, m_VisibleSize.y*(m_ScrollSize / m_MaxScroll));
		BarVisiblePos = LWVector2f(m_VisiblePosition.x, m_VisiblePosition.y + (m_VisibleSize.y - BarVisibleSize.y) - m_VisibleSize.y*(m_Scroll / m_MaxScroll));
	}
	DrawRect(ActiveMaterial, BarVisiblePos, BarVisibleSize, Frame);
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetBarOffMaterial(LWEUIMaterial *Material) {
	m_BarOffMaterial = Material;
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetBarDownMaterial(LWEUIMaterial *Material) {
	m_BarDownMaterial = Material;
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetBarOverMaterial(LWEUIMaterial *Material) {
	m_BarOverMaterial = Material;
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetBackgroundMaterial(LWEUIMaterial *Material) {
	m_BackgroundMaterial = Material;
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetScroll(float Scroll) {
	m_Scroll = std::max<float>(std::min<float>(Scroll, m_MaxScroll - m_ScrollSize), 0.0f);
	return *this;
}

LWEUIScrollBar &LWEUIScrollBar::SetMaxScroll(float MaxScroll) {
	m_MaxScroll = MaxScroll;
	return SetScroll(m_Scroll);
}

LWEUIScrollBar &LWEUIScrollBar::SetScrollSize(float ScrollSize) {
	m_ScrollSize = ScrollSize;
	return SetScroll(m_Scroll);
}

LWEUIMaterial *LWEUIScrollBar::GetBarOffMaterial(void) {
	return m_BarOffMaterial;
}

LWEUIMaterial *LWEUIScrollBar::GetBarOverMaterial(void) {
	return m_BarOverMaterial;
}

LWEUIMaterial *LWEUIScrollBar::GetBarDownMaterial(void) {
	return m_BarDownMaterial;
}

LWEUIMaterial *LWEUIScrollBar::GetBackgroundMaterial(void) {
	return m_BackgroundMaterial;
}

float LWEUIScrollBar::GetScroll(void) {
	return m_Scroll;
}

float LWEUIScrollBar::GetMaxScroll(void) {
	return m_MaxScroll;
}

float LWEUIScrollBar::GetScrollSize(void) {
	return m_ScrollSize;
}

LWEUIScrollBar::LWEUIScrollBar(LWEUIMaterial *BarOffMaterial, LWEUIMaterial *BarOverMaterial, LWEUIMaterial *BarDownMaterial, LWEUIMaterial *BackgroundMaterial, float MaxScroll, float ScrollSize, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_BackgroundMaterial(BackgroundMaterial), m_BarOffMaterial(BarOffMaterial), m_BarOverMaterial(BarOverMaterial), m_BarDownMaterial(BarDownMaterial), m_Scroll(0.0f), m_MaxScroll(MaxScroll), m_ScrollSize(ScrollSize) {}

LWEUIScrollBar::~LWEUIScrollBar() {}