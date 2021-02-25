#include <LWVideo/LWFont.h>
#include "LWEUI/LWEUILabel.h"
#include "LWELocalization.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include "LWPlatform/LWWindow.h"
#include "LWEJson.h"
#include <LWPlatform/LWFileStream.h>
#include <algorithm>
#include <cstdarg>
#include <iostream>

LWEUILabel *LWEUILabel::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[1024*2]; //max filesize of 2 kilobytes can be read into ValueSrc.
	char SBuffer[1024 * 2];
	LWFileStream Stream;
	LWAllocator &Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEAssetManager *AM = Manager->GetAssetManager();
	LWEUILabel *Label = Allocator.Create<LWEUILabel>("", nullptr, Allocator, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);

	LWEUI::XMLParse(Label, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	
	LWEXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWEXMLAttribute *ValueAttr = FindAttribute(Node, Style, "Value");
	LWEXMLAttribute *ValueSrcAttr = FindAttribute(Node, Style, "ValueSrc");
	LWEXMLAttribute *MaterAttr = FindAttribute(Node, Style, "Material");
	LWEXMLAttribute *ScaleAttr = FindAttribute(Node, Style, "Scale");
	LWFont *Font = nullptr;
	LWEUIMaterial *Mat = nullptr;
	if (FontAttr) Font = AM->GetAsset<LWFont>(ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (MaterAttr) Mat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), MaterAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	
	if (ValueAttr) {
		LWUTF8Iterator Text = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Text)) Text = LWUTF8Iterator(SBuffer);
		LWEJson::UnEscapeString(Text, Buffer, sizeof(Buffer));
		Label->SetText(LWUTF8Iterator(Buffer));	
	}else if (ValueSrcAttr) {
		LWUTF8Iterator Source = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueSrcAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Source)) Source = LWUTF8Iterator(SBuffer);
		if (!LWFileStream::OpenStream(Stream, Source, LWFileStream::ReadMode, Allocator)) {
			fmt::print("Error loading file: '{}'\n", Source);
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Label->SetText(Buffer);
		}
	}
	if (ScaleAttr) {
		LWUTF8Iterator Scale = ParseComponentAttribute(Buffer, sizeof(Buffer), ScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		Label->SetFontScale((float)atof((const char*)Scale()));
	}
	Label->SetFont(Font).SetMaterial(Mat);
	return Label;
}

LWEUI &LWEUILabel::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	bool wasOver = (m_Flag&MouseOver) != 0;
	m_Flag = (m_Flag&~MouseOver) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (m_Flag&MouseOver) != 0;
	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	return *this;
}

LWEUI &LWEUILabel::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	if (!m_Font) return *this;

	uint32_t HoriAlign = (m_Flag&(LabelLeftAligned | LabelCenterAligned | LabelRightAligned));
	uint32_t VertAlign = (m_Flag&(LabelBottomAligned | LabelVCenterAligned | LabelTopAligned));
	LWVector2f TSize = m_TextSize * Scale;
	LWVector2f Pos = VisiblePos - LWVector2f(0.0f, m_UnderHang * Scale);
	if (VertAlign == LabelVCenterAligned) Pos.y += (VisibleSize.y - TSize.y)*0.5f;
	else if (VertAlign == LabelTopAligned) Pos.y += (VisibleSize.y - TSize.y);
	LWVector4f Color = m_Material ? m_Material->m_ColorA : LWVector4f(1.0f);

	if (HoriAlign == LabelLeftAligned) m_Font->DrawTextm(m_Text.beginGrapheme(), Pos, m_FontScale*Scale, Color, &Frame, &LWEUIFrame::WriteFontGlyph);
	else {
		LWUTF8GraphemeIterator C = m_Text.beginGrapheme();
		LWUTF8GraphemeIterator N = C.NextLine();
		uint32_t l = 0;
		for(;!C.AtEnd(); C = N, N.AdvanceLine()) {
			LWUTF8GraphemeIterator Line = LWUTF8GraphemeIterator(C, N);
			LWVector4f LineSize = m_Font->MeasureText(Line, m_FontScale * Scale);
			float LS = (LineSize.z - LineSize.x);
			if (HoriAlign == LabelCenterAligned) Pos.x = (VisiblePos.x + VisibleSize.x * 0.5f - LS * 0.5f);
			else Pos.x = VisiblePos.x + VisibleSize.x - LS;
			m_Font->DrawTextm(Line, Pos, m_FontScale * Scale, Color, &Frame, &LWEUIFrame::WriteFontGlyph);
			Pos.y -= m_Font->GetLineSize() * m_FontScale * Scale;
		}
	}
	return *this;
}

void LWEUILabel::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUILabel &LWEUILabel::SetText(const LWUTF8Iterator &Text) {
	m_Text = Text;
	return SetFont(m_Font);
}

LWEUILabel &LWEUILabel::SetFont(LWFont *Font) {
	m_Font = Font;
	if (m_Font) {
		LWVector4f TextSize = Font->MeasureText(m_Text.beginGrapheme(), m_FontScale);
		m_TextSize = LWVector2f(TextSize.z - TextSize.x, TextSize.y-TextSize.w);
		if ((m_Flag&NoAutoWidthSize) == 0) m_Size.z = m_TextSize.x;
		if ((m_Flag&NoAutoHeightSize) == 0) m_Size.w = m_TextSize.y;
		m_UnderHang = TextSize.w;
	}
	return *this;
}

LWEUILabel &LWEUILabel::SetMaterial(LWEUIMaterial *Material) {
	m_Material = Material;
	return *this;
}

LWEUILabel &LWEUILabel::SetFontScale(float Scale) {
	m_FontScale = Scale;
	return SetFont(m_Font);
}

LWFont *LWEUILabel::GetFont(void) {
	return m_Font;
}

LWEUIMaterial *LWEUILabel::GetMaterial(void) {
	return m_Material;
}

const LWUTF8 &LWEUILabel::GetText(void) const {
	return m_Text;
}

float LWEUILabel::GetFontScale(void) const {
	return m_FontScale;
}

LWEUILabel::LWEUILabel(const LWUTF8Iterator &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_Text(Text, Allocator), m_Font(Font), m_Material(Material) {
	SetFont(Font);
}

LWEUILabel::LWEUILabel() : LWEUI(LWVector4f(), LWVector4f(), 0) {}

LWEUILabel::~LWEUILabel() {}
