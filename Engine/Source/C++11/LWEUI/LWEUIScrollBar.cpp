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

LWEUI &LWEUIScrollBar::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	const float MinimumTouchSize = 5.0f;
	const float TouchScale = 2.0f;
	const float ScrollScale = 0.25f;
	LWWindow *Wnd = Manager.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	LWVector2f DownPnt = LWVector2f(-100.0f, -100.0f);
	float DownSize = 0.0f;
	float InitScroll = m_Scroll;
	bool wasOver = (m_Flag&MouseOver) != 0;
	bool wasLDown = (m_Flag&MouseLDown) != 0;
	bool wasMDown = (m_Flag&MouseMDown) != 0;
	bool wasRDown = (m_Flag&MouseRDown) != 0;
	bool isHori = isHorizontal();
	uint64_t Flag = (m_Flag&~(MouseOver | MouseLDown | MouseMDown | MouseRDown));
	if (Mouse) DownPnt = Mouse->GetPositionf();
	if (Touch) {
		const LWGesture &Gest = Touch->GetGesture();
		//std::cout << "Gesture: " << Gest.m_Type << std::endl;
		if (Gest.m_Type == LWGesture::Drag || Gest.m_Type==LWGesture::PressAndDrag) {
			DownPnt = LWVector2f((float)Gest.m_Source.x, (float)Gest.m_Source.y)+LWVector2f((float)Gest.m_Direction.x, (float)Gest.m_Direction.y);
            DownSize = std::max<float>(Gest.m_Scale*Scale*TouchScale, MinimumTouchSize); //set's minimum size for finger.
		}
	}

	LWVector2f BarVisiblePos;
	LWVector2f BarVisibleSize;
	if(isHori){
		BarVisiblePos = LWVector2f(VisiblePos.x + VisibleSize.x*(m_Scroll / m_MaxScroll), VisiblePos.y);
		BarVisibleSize = LWVector2f(VisibleSize.x*(m_ScrollSize / m_MaxScroll), VisibleSize.y);
	} else {
		BarVisibleSize = LWVector2f(VisibleSize.x, VisibleSize.y*(m_ScrollSize / m_MaxScroll));
		BarVisiblePos = LWVector2f(VisiblePos.x, VisiblePos.y + (VisibleSize.y - BarVisibleSize.y) - VisibleSize.y*(m_Scroll / m_MaxScroll));
	}

	bool OverAll = PointInside(DownPnt, DownSize, VisiblePos, VisibleSize);
	bool OverBar = PointInside(DownPnt, DownSize, BarVisiblePos, BarVisibleSize) || wasLDown || wasRDown || wasMDown;
	if (OverBar) Flag |= MouseOver;
	bool isOver = OverBar;

	if (Mouse) {
		if (OverAll) {
			int32_t Scroll = Mouse->GetScroll();
			if (Scroll != 0) {
				if (Scroll > 0) SetScroll(m_Scroll - m_ScrollSize * ScrollScale);
				else SetScroll(m_Scroll + m_ScrollSize * ScrollScale);
			}
		}
		if (Mouse->ButtonDown(LWMouseKey::Left)) Flag |= ((wasLDown && OverAll) || isOver) ? MouseLDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Right)) Flag |= ((wasRDown && OverAll) || isOver) ? MouseRDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Middle)) Flag |= ((wasMDown && OverAll) || isOver) ? MouseMDown : 0;
	}
	if (Touch) Flag |= ((wasLDown && OverAll) || isOver) ? MouseLDown : 0;
	bool isLDown = (Flag&MouseLDown) != 0;
	bool isRDown = (Flag&MouseRDown) != 0;
	bool isMDown = (Flag&MouseMDown) != 0;
	bool isFocusable = (Flag&FocusAble) != 0;
	m_Flag = Flag;
	if(isLDown || isRDown || isMDown){
		float t = 0.0f;
		if (isHori) {
			t = (DownPnt.x - VisiblePos.x);
			if (fabs(DownPnt.y - VisiblePos.y) > VisibleSize.y*4.0f+DownSize){
				SetScroll(m_InitialScroll);
			} else {
				SetScroll((t - BarVisibleSize.x*0.5f)*(m_MaxScroll / VisibleSize.x));
			}
		} else {
			t = ((VisiblePos.y + VisibleSize.y) - DownPnt.y);
			if (fabs(DownPnt.x - VisiblePos.x) > VisibleSize.x*4.0f + DownSize) {
				SetScroll(m_InitialScroll);
			} else {
				SetScroll((t - BarVisibleSize.y*0.5f)*(m_MaxScroll / VisibleSize.y));
			}			
		}
	}

	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	Manager.DispatchEvent(this, Event_Changed, InitScroll!=m_Scroll);
	Manager.DispatchEvent(this, Event_Pressed, isOver && isLDown && !wasLDown);
	Manager.DispatchEvent(this, Event_RPressed, isOver && isRDown && !wasRDown);
	Manager.DispatchEvent(this, Event_MPressed, isOver && isMDown && !wasMDown);
	if (Manager.DispatchEvent(this, Event_Released, isOver && !isLDown && wasLDown)) {
		m_InitialScroll = m_Scroll;
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_RReleased, isOver && !isRDown && wasRDown)) {
		m_InitialScroll = m_Scroll;
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_MReleased, isOver && !isMDown && wasMDown)) {
		m_InitialScroll = m_Scroll;
		if (isFocusable) Manager.SetFocused(this);
	}
	return *this;
}

LWEUI &LWEUIScrollBar::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWEUIMaterial *ActiveMaterial = m_BarOffMaterial;
	bool isOver = (m_Flag&MouseOver) != 0;
	bool isDown = (m_Flag&(MouseLDown|MouseMDown|MouseRDown)) != 0;
	if (isOver) ActiveMaterial = isDown ? m_BarDownMaterial : (isOver ? m_BarOverMaterial : m_BarOffMaterial);
	Frame.WriteRect(m_BackgroundMaterial, VisiblePos, VisibleSize);

	LWVector2f BarVisiblePos;
	LWVector2f BarVisibleSize;
	if (m_Flag&HorizontalBar) {
		BarVisiblePos = LWVector2f(VisiblePos.x + VisibleSize.x*(m_Scroll / m_MaxScroll), VisiblePos.y);
		BarVisibleSize = LWVector2f(VisibleSize.x*(m_ScrollSize / m_MaxScroll), VisibleSize.y);
	} else {
		BarVisibleSize = LWVector2f(VisibleSize.x, VisibleSize.y*(m_ScrollSize / m_MaxScroll));
		BarVisiblePos = LWVector2f(VisiblePos.x, VisiblePos.y + (VisibleSize.y - BarVisibleSize.y) - VisibleSize.y*(m_Scroll / m_MaxScroll));
	}
	Frame.WriteRect(ActiveMaterial, BarVisiblePos, BarVisibleSize);
	return *this;
}

void LWEUIScrollBar::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
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

LWEUIScrollBar &LWEUIScrollBar::SetDirection(bool isHorizontal) {
	m_Flag = (m_Flag&~HorizontalBar) | (isHorizontal ? HorizontalBar : 0);
	return *this;
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

float LWEUIScrollBar::GetScroll(void) const {
	return m_Scroll;
}

float LWEUIScrollBar::GetMaxScroll(void) const {
	return m_MaxScroll;
}

float LWEUIScrollBar::GetScrollSize(void) const {
	return m_ScrollSize;
}

bool LWEUIScrollBar::isHorizontal(void) const {
	return (m_Flag&HorizontalBar) != 0;
}

bool LWEUIScrollBar::isVertical(void) const {
	return (m_Flag&HorizontalBar) == 0;
}

LWEUIScrollBar::LWEUIScrollBar(LWEUIMaterial *BarOffMaterial, LWEUIMaterial *BarOverMaterial, LWEUIMaterial *BarDownMaterial, LWEUIMaterial *BackgroundMaterial, float MaxScroll, float ScrollSize, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_BackgroundMaterial(BackgroundMaterial), m_BarOffMaterial(BarOffMaterial), m_BarOverMaterial(BarOverMaterial), m_BarDownMaterial(BarDownMaterial), m_Scroll(0.0f), m_MaxScroll(MaxScroll), m_ScrollSize(ScrollSize) {}

LWEUIScrollBar::~LWEUIScrollBar() {}