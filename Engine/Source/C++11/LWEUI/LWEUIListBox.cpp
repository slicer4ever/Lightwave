#include <LWVideo/LWFont.h>
#include "LWEUI/LWEUIListBox.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include "LWELocalization.h"
#include <LWPlatform/LWWindow.h>
#include <algorithm>
#include <iostream>

LWEUIListBox *LWEUIListBox::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIListBox *ListBox = Allocator->Allocate<LWEUIListBox>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, LWVector4f(0.0f), LWVector4f(0.0f), FocusAble);
	LWEUI::XMLParse(ListBox, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWXMLAttribute *OverAttr = FindAttribute(Node, Style, "OverMaterial");
	LWXMLAttribute *DownAttr = FindAttribute(Node, Style, "DownMaterial");
	LWXMLAttribute *OffAttr = FindAttribute(Node, Style, "OffMaterial");
	LWXMLAttribute *BackAttr = FindAttribute(Node, Style, "BackgroundMaterial");
	LWXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWXMLAttribute *MinimumHeightAttr = FindAttribute(Node, Style, "MinimumHeight");
	LWXMLAttribute *BorderSizeAttr = FindAttribute(Node, Style, "BorderSize");
	LWXMLAttribute *ValuesAttr = FindAttribute(Node, Style, "Values");
	LWXMLAttribute *FontMatAttr = FindAttribute(Node, Style, "FontMaterial");
	LWXMLAttribute *FontScaleAttr = FindAttribute(Node, Style, "FontScale");
	LWEUIMaterial *OverMat = nullptr;
	LWEUIMaterial *DownMat = nullptr;
	LWEUIMaterial *OffMat = nullptr;
	LWEUIMaterial *BackMat = nullptr;
	LWEUIMaterial *FntMat = nullptr;
	LWFont *Font = nullptr;
	float MinHeight = 0.0f;
	float BorderSize = 0.0f;

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
	if (BackAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), BackAttr->m_Value, ActiveComponent, ActiveComponentNode);
		BackMat = Manager->GetMaterial(Res);
	}
	if (FontMatAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), FontMatAttr->m_Value, ActiveComponent, ActiveComponentNode);
		FntMat = Manager->GetMaterial(Res);
	}
	if (FontAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Font = Manager->GetAssetManager()->GetAsset<LWFont>(Res);
	}
	if (MinimumHeightAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), MinimumHeightAttr->m_Value, ActiveComponent, ActiveComponentNode);
		MinHeight = (float)atof(Res);
	}
	if (BorderSizeAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), BorderSizeAttr->m_Value, ActiveComponent, ActiveComponentNode);
		BorderSize = (float)atof(Res);
	}
	if (FontScaleAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), FontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode);
		ListBox->SetFontScale((float)atof(Res));
	}
	ListBox->SetOffMaterial(OffMat).SetOverMaterial(OverMat).SetDownMaterial(DownMat).SetBackgroundMaterial(BackMat).SetFontMaterial(FntMat);
	ListBox->SetFont(Font).SetBorderSize(BorderSize).SetMinimumHeightSize(MinHeight);
	if (ValuesAttr) {
		char TempBuffer[LWELISTBOX_MAXLENGTH];
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValuesAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		for (const char *C = LWText::FirstCharacter(Res); *C;) {
			C = LWText::CopyToTokens(C, TempBuffer, sizeof(TempBuffer), "|");
			ListBox->PushItem(TempBuffer, nullptr);
			if (*C) C++;
		}
	}

	return ListBox;
}

void LWEUIListBox::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUI &LWEUIListBox::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	const float ScrollScale = 0.1f;
	LWWindow *Wnd = Manager.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	
	bool wasOver = (m_Flag&MouseOver) != 0;
	bool wasLDown = (m_Flag&MouseLDown) != 0;
	bool wasMDown = (m_Flag&MouseMDown) != 0;
	bool wasRDown = (m_Flag&MouseRDown) != 0;
	uint64_t Flag = (m_Flag&~(MouseOver|MouseLDown|MouseMDown|MouseRDown)) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (Flag&MouseOver) != 0;
	bool Changed = false;
	if (!isOver) m_OverItem = NullItem;

	if (Mouse) {
		LWVector2f MP = Mouse->GetPositionf();
		if (isOver) {
			int32_t Scroll = Mouse->GetScroll();
			if (Scroll != 0) {
				Changed = true;
				if (Scroll > 0) SetScroll(m_Scroll - GetScrollPageSize()*ScrollScale, Scale);
				else SetScroll(m_Scroll + GetScrollPageSize()*ScrollScale, Scale);
			}
			if (Mouse->ButtonDown(LWMouseKey::Left)) Flag |= (isOver ? MouseLDown : 0);
			if (Mouse->ButtonDown(LWMouseKey::Right)) Flag |= (isOver ? MouseRDown : 0);
			if (Mouse->ButtonDown(LWMouseKey::Middle)) Flag |= (isOver ? MouseMDown : 0);
			float TotalDist = ((VisiblePos.y + VisibleSize.y) - MP.y) + m_Scroll;
			uint32_t Item = (uint32_t)(TotalDist / GetCellHeight(Scale));
			m_OverItem = Item >= m_ItemCount ? NullItem : Item;
		}
	}
	if (Touch) {
		const LWGesture &Gest = Touch->GetGesture();
		if (Gest.m_Type == LWGesture::Drag || Gest.m_Type == LWGesture::PressAndDrag) {
			if (m_InitScroll <= -1000.0f) m_InitScroll = m_Scroll;
			SetScroll(m_InitScroll + Gest.m_Direction.y, Scale);
			Changed = true;
		} else if (Gest.m_Type == LWGesture::Flick) {
			m_ScrollAcceleration = Gest.m_Direction.y*10.0f;
		} else m_InitScroll = -1000.0f;
		uint32_t TouchPntCnt = Touch->GetPointCount();
		for (uint32_t i = 0; i < TouchPntCnt; i++) {
			const LWTouchPoint &TP = Touch->GetPoint(i);
			if (TP.m_State == LWTouchPoint::UP) continue;
			LWVector2f TPos = TP.m_Position.CastTo<float>();
			bool Over = PointInside(TPos, TP.m_Size*Scale);
			if (!Over) continue;
			Flag |= MouseLDown;
			float TotalDist = ((VisiblePos.y + VisibleSize.y) - TPos.y) + m_Scroll;
			uint32_t Item = (uint32_t)(TotalDist / GetCellHeight(Scale));
			m_OverItem = Item >= m_ItemCount ? NullItem : Item;
		}
	}
	if (m_ScrollAcceleration > std::numeric_limits<float>::epsilon()) {
		Changed = true;
		SetScroll(m_Scroll + m_ScrollAcceleration, Scale);
		m_ScrollAcceleration *= 0.9f;
	}
	bool isLDown = (Flag&MouseLDown) != 0;
	bool isRDown = (Flag&MouseRDown) != 0;
	bool isMDown = (Flag&MouseMDown) != 0;
	bool isFocusable = (Flag&FocusAble) != 0;
	m_Flag = Flag;

	Manager.DispatchEvent(this, Event_Changed, Changed);
	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOver, !isOver && wasOver);
	Manager.DispatchEvent(this, Event_Pressed, isOver && isLDown && !wasLDown);
	Manager.DispatchEvent(this, Event_RPressed, isOver && isRDown && !wasRDown);
	Manager.DispatchEvent(this, Event_MPressed, isOver && isMDown && !wasMDown);
	if (Manager.DispatchEvent(this, Event_Released, isOver && !isLDown && wasLDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_RReleased, isOver && !isRDown && wasRDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	if (Manager.DispatchEvent(this, Event_MReleased, isOver && !isMDown && wasMDown)) {
		if (isFocusable) Manager.SetFocused(this);
	}
	return *this;
}

LWEUI &LWEUIListBox::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	Frame.WriteRect(m_BackgroundMaterial, VisiblePos - m_BorderSize, VisibleSize + m_BorderSize*2.0f);
	float CellHeight = GetCellHeight(Scale);
	float LineSize = 0.0f;
	if (m_Font) LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	uint32_t n = (uint32_t)((m_Scroll) / CellHeight);
	uint32_t c = (uint32_t)(ceilf(GetScrollPageSize() / CellHeight));
	n = n > 0 ? n - 1 : n;
	n = std::min<uint32_t>(n, m_ItemCount);
	c = std::min<uint32_t>(n + c + 2, m_ItemCount);
	for (uint32_t i = n; i < c; i++) {
		float CellPos = (float)i*CellHeight - m_Scroll;
		LWEUIListBoxItem &Item = m_ItemList[i];
		LWEUIMaterial *Mat = Item.m_OffMaterial?Item.m_OffMaterial:m_OffMaterial;
		if (i == m_OverItem) {
			Mat = Item.m_OverMaterial ? Item.m_OverMaterial : m_OverMaterial;
			if (m_Flag&(MouseLDown | MouseMDown | MouseRDown)) {
				Mat = Item.m_DownMaterial ? Item.m_DownMaterial : m_DownMaterial;
			}
		}
		LWVector2f CellP = LWVector2f(VisiblePos.x, VisiblePos.y + (VisibleSize.y - CellPos - CellHeight));
		Frame.WriteClippedRect(Mat, CellP, LWVector2f(VisibleSize.x, CellHeight), LWVector4f(VisiblePos, VisibleSize));
		
		CellP.y += (CellHeight*0.5f) - (Item.m_TextSize.y*0.5f)*Scale - Item.m_TextUnderhang*Scale;
		if (m_Flag&LabelRightAligned) CellP.x += VisibleSize.x - Item.m_TextSize.x*Scale;
		else if (m_Flag&LabelCenterAligned) CellP.x += VisibleSize.x*0.5f - Item.m_TextSize.x*0.5f*Scale;
		Frame.WriteClippedText(m_FontMaterial, Item.m_Name, m_Font, CellP, m_FontScale*Scale, LWVector4f(VisiblePos, VisibleSize));
	}
	return *this;
}

bool LWEUIListBox::PushItem(const LWText &ItemName, void *UserData, LWEUIMaterial *OffMat, LWEUIMaterial *OverMat, LWEUIMaterial *DownMat) {
	if (m_ItemCount >= LWELISTBOX_MAXITEMS) return false;
	strncpy(m_ItemList[m_ItemCount].m_Name, (const char*)ItemName.GetCharacters(), sizeof(char)*LWELISTBOX_MAXITEMS);
	m_ItemList[m_ItemCount].m_UserData = UserData;
	LWVector4f BV = LWVector4f();
	if (m_Font) BV = m_Font->MeasureText(ItemName, m_FontScale);
	m_ItemList[m_ItemCount].m_TextSize = LWVector2f(BV.z - BV.x, BV.y-BV.w);
	m_ItemList[m_ItemCount].m_TextUnderhang = BV.w;
	m_ItemList[m_ItemCount].m_OffMaterial = OffMat;
	m_ItemList[m_ItemCount].m_OverMaterial = OverMat;
	m_ItemList[m_ItemCount].m_DownMaterial = DownMat;
	m_ItemCount++;
	return true;
}

bool LWEUIListBox::PushItemf(const char *Fmt, void *UserData, LWEUIMaterial *OffMat, LWEUIMaterial *OverMat, LWEUIMaterial *DownMat, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, DownMat);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return PushItem(Buffer, UserData, OffMat, OverMat, DownMat);
}


bool LWEUIListBox::Clear(void) {
	m_ItemCount = 0;
	m_OverItem = 0xFFFFFFFF;
	m_Scroll = 0.0f;
	return true;
}

bool LWEUIListBox::RemoveItem(uint32_t i) {
	if (i >= m_ItemCount) return false;
	memcpy(m_ItemList + i, m_ItemList + i + 1, sizeof(LWEUIListBox)*(m_ItemCount - i - 1));
	m_ItemCount--;
	m_OverItem = 0xFFFFFFFF;
	SetScroll(m_Scroll);
	return true;
}

LWEUIListBox &LWEUIListBox::SetBackgroundMaterial(LWEUIMaterial *Mat) {
	m_BackgroundMaterial = Mat;
	return *this;
}

LWEUIListBox &LWEUIListBox::SetOverMaterial(LWEUIMaterial *Mat) {
	m_OverMaterial = Mat;
	return *this;
}

LWEUIListBox &LWEUIListBox::SetOffMaterial(LWEUIMaterial *Mat) {
	m_OffMaterial = Mat;
	return *this;
}

LWEUIListBox &LWEUIListBox::SetDownMaterial(LWEUIMaterial *Mat) {
	m_DownMaterial = Mat;
	return *this;
}

LWEUIListBox &LWEUIListBox::SetFontMaterial(LWEUIMaterial *Mat) {
	m_FontMaterial = Mat;
	return *this;
}

LWEUIListBox &LWEUIListBox::SetFont(LWFont *Font) {
	m_Font = Font;
	if (m_Font) {
		for (uint32_t i = 0; i < m_ItemCount; i++) {
			LWVector4f BV = m_Font->MeasureText(m_ItemList[i].m_Name, m_FontScale);
			m_ItemList[i].m_TextSize = LWVector2f(BV.z - BV.x, BV.y);
		}
	}
	return SetScroll(m_Scroll);
}

LWEUIListBox &LWEUIListBox::SetFontScale(float FontScale) {
	m_FontScale = FontScale;
	return SetFont(m_Font);
}

LWEUIListBox &LWEUIListBox::SetScroll(float Scroll, float Scale) {
	m_Scroll = std::max<float>(std::min<float>(Scroll, GetScrollMaxSize(Scale) - GetScrollPageSize()), 0.0f);
	return *this;
}

LWEUIListBox &LWEUIListBox::SetMinimumHeightSize(float MinimumHeight) {
	m_MinimumBoxHeight = MinimumHeight;
	return SetScroll(m_Scroll);
}

LWEUIListBox &LWEUIListBox::SetBorderSize(float BorderSize) {
	m_BorderSize = BorderSize;
	return *this;
}

LWEUIListBoxItem *LWEUIListBox::GetItem(uint32_t i) {
	return m_ItemList + i;
}

uint32_t LWEUIListBox::GetItemCount(void) const {
	return m_ItemCount;
}

uint32_t LWEUIListBox::GetItemOver(void) const{
	return m_OverItem;
}

float LWEUIListBox::GetScroll(void) const{
	return m_Scroll;
}

float LWEUIListBox::GetBorderSize(void) const {
	return m_BorderSize;
}

LWFont *LWEUIListBox::GetFont(void){
	return m_Font;
}

LWEUIMaterial *LWEUIListBox::GetBackgroundMaterial(void) {
	return m_BackgroundMaterial;
}

LWEUIMaterial *LWEUIListBox::GetOffMaterial(void) {
	return m_OffMaterial;
}

LWEUIMaterial *LWEUIListBox::GetDownMaterial(void) {
	return m_DownMaterial;
}

LWEUIMaterial *LWEUIListBox::GetOverMaterial(void) {
	return m_OverMaterial;
}

LWEUIMaterial *LWEUIListBox::GetFontMaterial(void) {
	return m_FontMaterial;
}

float LWEUIListBox::GetScrollPageSize(void) const{
	return m_VisibleSize.y;
}

float LWEUIListBox::GetScrollMaxSize(float Scale) const{
	return GetCellHeight(Scale)*m_ItemCount;
}

float LWEUIListBox::GetCellHeight(float Scale) const{
	float ch = m_MinimumBoxHeight;
	if (m_Font) ch += m_Font->GetLineSize()*m_FontScale;
	return ch*Scale;
}

float LWEUIListBox::GetFontScale(void) const {
	return m_FontScale;
}

LWEUIListBox::LWEUIListBox(LWEUIMaterial *BackgroundMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial, LWEUIMaterial *FntMaterial, LWFont *Font, float MinimumHeight, float BorderSize, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_BackgroundMaterial(BackgroundMaterial), m_OffMaterial(OffMaterial), m_DownMaterial(DownMaterial), m_OverMaterial(OverMaterial), m_FontMaterial(FntMaterial), m_Font(Font), m_ItemCount(0), m_OverItem(0xFFFFFFFF), m_MinimumBoxHeight(MinimumHeight), m_BorderSize(BorderSize), m_Scroll(0.0f), m_ScrollAcceleration(0.0f) {}

LWEUIListBox::~LWEUIListBox() {}
