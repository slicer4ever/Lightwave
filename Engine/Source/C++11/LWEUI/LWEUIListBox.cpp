#include <LWVideo/LWFont.h>
#include "LWEUI/LWEUIListBox.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include "LWELocalization.h"
#include <LWPlatform/LWWindow.h>
#include <algorithm>
#include <iostream>

LWEUIListBox *LWEUIListBox::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIListBox *ListBox = Allocator->Allocate<LWEUIListBox>(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0.0f, 0.0f, LWVector4f(0.0f), LWVector4f(0.0f), FocusAble);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
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
		Font = (LWFont*)FindAsset(Manager->GetAssetManager(), Res, LWEAsset::Font);
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

LWEUI &LWEUIListBox::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	
	uint32_t Flag = m_Flag;
	m_Flag &= ~(MouseOver | MouseDown);
	bool OverAll = false;

	if (Mouse) {
		LWVector2i MP = Mouse->GetPosition();
		LWVector2f MousePnt = LWVector2f((float)MP.x, (float)MP.y);
		bool Over = LWEUI::PointInside(MousePnt, 0.0f, m_VisiblePosition, m_VisibleSize);
		if (Over) {
			Manager->DispatchEvent(this, Event_TempOverInc);
			m_Flag |= MouseOver;
			int32_t Scroll = Mouse->GetScroll();
			if(Scroll){
				if(Scroll>0) SetScroll(m_Scroll - GetScrollPageSize()*0.1f, Scale);
				else SetScroll(m_Scroll + GetScrollPageSize()*0.1f, Scale);
				Manager->DispatchEvent(this, Event_Changed);
			}
			if (Mouse->ButtonDown(LWMouseKey::Left)) m_Flag |= MouseDown;

			float TotalDist = ((m_VisiblePosition.y + m_VisibleSize.y) - MousePnt.y) + m_Scroll;
			m_OverItem = (uint32_t)(TotalDist / (GetCellHeight(Scale)));
			m_OverItem = m_OverItem >= m_ItemCount ? 0xFFFFFFFF : m_OverItem;
			OverAll = true;
		} else m_OverItem = 0xFFFFFFFF;
	}
	if (Touch) {
		const LWGesture &Gest = Touch->GetGesture();
		if (Gest.m_Type == LWGesture::Drag || Gest.m_Type == LWGesture::PressAndDrag) {
			if (m_InitScroll <= -1000.0f) m_InitScroll = m_Scroll;
			SetScroll(m_InitScroll + Gest.m_Direction.y, Scale);
		}else if(Gest.m_Type==LWGesture::Flick){
			m_ScrollAcceleration = Gest.m_Direction.y*10.0f;
		}else m_InitScroll = -1000.0f;
		for (uint32_t i = 0; i < Touch->GetPointCount(); i++) {
			const LWTouchPoint *TP = Touch->GetPoint(i);
			if (TP->m_State != LWTouchPoint::UP) {
				LWVector2f T = LWVector2f((float)TP->m_Position.x, (float)TP->m_Position.y);
				bool Over = LWEUI::PointInside(T, TP->m_Size*Scale, m_VisiblePosition, m_VisibleSize);
				if (Over) {
					Manager->DispatchEvent(this, Event_TempOverInc | (i << Event_OverOffset));
					OverAll = true;
					m_Flag |= MouseOver;
					if (TP->m_State == LWTouchPoint::DOWN) m_Flag |= MouseDown;
					float TotalDist = ((m_VisiblePosition.y + m_VisibleSize.y) - TP->m_Position.y) + m_Scroll;
					m_OverItem = (uint32_t)(TotalDist / (GetCellHeight(Scale)));
					m_OverItem = m_OverItem >= m_ItemCount ? 0xFFFFFFFF : m_OverItem;
				}
			}
			if (!OverAll) m_OverItem = 0xFFFFFFFF;
		}
	}
	SetScroll(m_Scroll + m_ScrollAcceleration, Scale);
	m_ScrollAcceleration *= 0.9f;
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

LWEUI &LWEUIListBox::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	auto DrawRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, LWEUIFrame *F)->bool {
		if (!Mat) return false;
		if (!F->SetActiveTexture(Mat->m_Texture, false)) return false;
		LWVector4f SubRegion = Mat->m_SubRegion;

		uint32_t c = LWVertexUI::WriteRectangle(F->m_Mesh, Pos + LWVector2f(0.0f, Size.y), Pos + LWVector2f(Size.x, 0.0f), Mat->m_Color, LWVector2f(SubRegion.x, SubRegion.y), LWVector2f(SubRegion.z, SubRegion.w));
		F->m_VertexCount[F->m_TextureCount - 1] += c;
		return c != 0;
	};

	auto DrawClippedRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, LWEUIFrame *F, const LWVector4f &AABB)->bool {
		if (!Mat) return false;
		if (!F->SetActiveTexture(Mat->m_Texture, false)) return false;
		LWVector4f SubRegion = Mat->m_SubRegion;

		uint32_t c = LWVertexUI::WriteRectangle(F->m_Mesh, Pos + LWVector2f(0.0f, Size.y), Pos + LWVector2f(Size.x, 0.0f), Mat->m_Color, LWVector2f(SubRegion.x, SubRegion.y), LWVector2f(SubRegion.z, SubRegion.w), AABB);
		F->m_VertexCount[F->m_TextureCount - 1] += c;
		if (F->m_VertexCount[F->m_TextureCount - 1] == 0) F->m_TextureCount--;
		return c != 0;
	};

	auto DrawClippedText = [](LWEUIMaterial *Mat, const LWText &Text, LWFont *Fnt, const LWVector2f &Pos, float Scale, const LWVector4f &AABB, LWEUIFrame *F)->bool {
		if (!Fnt) return false;
		LWVector4f Clr = Mat ? Mat->m_Color : LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
		Fnt->DrawClippedTextm(Text, Pos, Scale, Clr, AABB, F, &LWEUIFrame::WriteFontGlyph);
		return true;

	};

	DrawRect(m_BackgroundMaterial, m_VisiblePosition - m_BorderSize, m_VisibleSize + m_BorderSize*2.0f, Frame);
	float CellHeight = GetCellHeight(Scale);
	uint32_t n = (uint32_t)((m_Scroll) / CellHeight);
	uint32_t c = (uint32_t)((GetScrollPageSize()) / CellHeight);
	n = n > 0 ? n - 1 : n;
	n = std::min<uint32_t>(n, m_ItemCount);
	c = std::min<uint32_t>(n + c + 2, m_ItemCount);
	for (uint32_t i = n; i < c; i++) {
		float CellPos = (float)i*CellHeight - (m_Scroll);
		LWEUIListBoxItem &Item = m_ItemList[i];
		LWEUIMaterial *Mat = Item.m_OffMaterial?Item.m_OffMaterial:m_OffMaterial;
		if (i == m_OverItem) {
			Mat = Item.m_OverMaterial ? Item.m_OverMaterial : m_OverMaterial;
			if (m_Flag&MouseDown) Mat = Item.m_DownMaterial ? Item.m_DownMaterial : m_DownMaterial;
		}
		LWVector2f CellP = LWVector2f(m_VisiblePosition.x, m_VisiblePosition.y + (m_VisibleSize.y - CellPos - CellHeight));
		DrawClippedRect(Mat, CellP, LWVector2f(m_VisibleSize.x, CellHeight), Frame, LWVector4f(m_VisiblePosition, m_VisibleSize));
		CellP.y += (m_MinimumBoxHeight*0.5f*Scale) + ((m_Font->GetLineSize()*m_FontScale*Scale) - (m_ItemList[i].m_TextSize.y*Scale))*0.5f*Scale;
		if (m_Flag&LabelRightAligned) CellP.x += m_VisibleSize.x - Item.m_TextSize.x*Scale;
		else if (m_Flag&LabelCenterAligned) CellP.x += m_VisibleSize.x*0.5f - Item.m_TextSize.x*0.5f*Scale;
		DrawClippedText(m_FontMaterial, Item.m_Name, m_Font, CellP, m_FontScale*Scale, LWVector4f(m_VisiblePosition, m_VisibleSize), Frame);
	}
	return *this;
}

bool LWEUIListBox::PushItem(const LWText &ItemName, void *UserData, LWEUIMaterial *OffMat, LWEUIMaterial *OverMat, LWEUIMaterial *DownMat) {
	if (m_ItemCount >= LWELISTBOX_MAXITEMS) return false;
	strncpy(m_ItemList[m_ItemCount].m_Name, (const char*)ItemName.GetCharacters(), sizeof(char)*LWELISTBOX_MAXITEMS);
	m_ItemList[m_ItemCount].m_UserData = UserData;
	LWVector4f BV = LWVector4f();
	if (m_Font) BV = m_Font->MeasureText(ItemName, m_FontScale);
	m_ItemList[m_ItemCount].m_TextSize = LWVector2f(BV.z - BV.x, BV.y);
	m_ItemList[m_ItemCount].m_OffMaterial = OffMat;
	m_ItemList[m_ItemCount].m_OverMaterial = OverMat;
	m_ItemList[m_ItemCount].m_DownMaterial = DownMat;
	m_ItemCount++;
	return true;
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

LWEUIListBox::LWEUIListBox(LWEUIMaterial *BackgroundMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial, LWEUIMaterial *FntMaterial, LWFont *Font, float MinimumHeight, float BorderSize, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_BackgroundMaterial(BackgroundMaterial), m_OffMaterial(OffMaterial), m_DownMaterial(DownMaterial), m_OverMaterial(OverMaterial), m_FontMaterial(FntMaterial), m_Font(Font), m_ItemCount(0), m_OverItem(0xFFFFFFFF), m_MinimumBoxHeight(MinimumHeight), m_BorderSize(BorderSize), m_Scroll(0.0f), m_ScrollAcceleration(0.0f) {}

LWEUIListBox::~LWEUIListBox() {}
