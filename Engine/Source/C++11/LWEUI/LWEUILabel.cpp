#include <LWVideo/LWFont.h>
#include "LWEUI/LWEUILabel.h"
#include "LWELocalization.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include "LWPlatform/LWWindow.h"
#include <LWPlatform/LWFileStream.h>
#include <algorithm>
#include <cstdarg>
#include <iostream>

LWEUILabel *LWEUILabel::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[1024*2]; //max filesize of 2 kilobytes can be read into ValueSrc.
	char SBuffer[1024 * 2];
	LWFileStream Stream;
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUILabel *Label = Allocator->Allocate<LWEUILabel>("", nullptr, *Allocator, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);

	auto FormatValue = [](const char *In, char *Buffer, uint32_t BufferLen)->uint32_t {
		char *BE = Buffer + BufferLen;
		char *B = Buffer;
		uint32_t o = 0;
		for (const char *C = In; B != BE && *C; B++, C++, o++) {
			if (*C == '\\') {
				if (*(C + 1) == 'n') {
					C++;
					*B = '\n';
					continue;
				} else if (*(C + 1) == 't') {
					C++;
					*B = '\t';
					continue;
				}
			}
			*B = *C;
		}
		if (B == BE) B--;
		if (BufferLen) *B = '\0';
		return o;
	};

	LWEUI::XMLParse(Label, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	
	LWXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWXMLAttribute *ValueAttr = FindAttribute(Node, Style, "Value");
	LWXMLAttribute *ValueSrcAttr = FindAttribute(Node, Style, "ValueSrc");
	LWXMLAttribute *MaterAttr = FindAttribute(Node, Style, "Material");
	LWXMLAttribute *ScaleAttr = FindAttribute(Node, Style, "Scale");
	LWFont *Font = nullptr;
	LWEUIMaterial *Mat = nullptr;
	if (FontAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Font = Manager->GetAssetManager()->GetAsset<LWFont>(Res);
	}
	if (MaterAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), MaterAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Mat = Manager->GetMaterial(Res);
	
	}
	if (ValueAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		FormatValue(Res, Buffer, sizeof(Buffer));
		Label->SetText(Buffer);
	
	}else if (ValueSrcAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueSrcAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		if (!LWFileStream::OpenStream(Stream, Res, LWFileStream::ReadMode, *Allocator)) {
			std::cout << "Error loading file: '" << Res << "'" << std::endl;
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Label->SetText(Buffer);
		
		}
	}
	if (ScaleAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ScaleAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Label->SetFontScale((float)atof(Res));
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

	if (HoriAlign == LabelLeftAligned) m_Font->DrawTextm(m_Text, Pos, m_FontScale*Scale, m_Material->m_ColorA, &Frame, &LWEUIFrame::WriteFontGlyph);
	else {
		const uint8_t *C = m_Text.GetCharacters();
		const uint8_t *N = LWText::FirstToken(C, '\n');
		uint32_t l = 0;
		for (; C; C = N ? (N + 1) : N, N = N ? (LWText::FirstToken(N + 1, '\n')) : N) {
			uint32_t Len = N ? (uint32_t)(uintptr_t)(N - C) : 0xFFFFFFFF;
			LWVector4f LineSize = m_Font->MeasureText(C, Len, m_FontScale*Scale);
			float LS = (LineSize.z - LineSize.x);
			if (HoriAlign == LabelCenterAligned) Pos.x = (VisiblePos.x + VisibleSize.x*0.5f - LS*0.5f);
			else Pos.x = VisiblePos.x + VisibleSize.x - LS;
			m_Font->DrawTextm(C, Len, Pos, m_FontScale*Scale, m_Material->m_ColorA, &Frame, &LWEUIFrame::WriteFontGlyph);
			Pos.y -= m_Font->GetLineSize()*m_FontScale*Scale;
		}
	}
	return *this;
}

void LWEUILabel::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUILabel &LWEUILabel::SetText(const LWText &Text) {
	m_Text.Set(Text.GetCharacters());
	return SetFont(m_Font);
}

LWEUILabel &LWEUILabel::SetTextf(const char *Format, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return SetText(Buffer);
}

LWEUILabel &LWEUILabel::SetFont(LWFont *Font) {
	m_Font = Font;
	if (m_Font) {
		LWVector4f TextSize = Font->MeasureText(m_Text, m_FontScale);
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

const LWText &LWEUILabel::GetText(void) const {
	return m_Text;
}

float LWEUILabel::GetFontScale(void) const {
	return m_FontScale;
}

LWEUILabel::LWEUILabel(const LWText &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_Text(LWText(Text.GetCharacters(), Allocator)), m_Font(Font), m_Material(Material), m_FontScale(1.0f) {
	SetFont(Font);
}

LWEUILabel::LWEUILabel() : LWEUI(LWVector4f(), LWVector4f(), 0), m_Font(nullptr), m_Material(nullptr), m_FontScale(1.0f), m_UnderHang(0.0f){}

LWEUILabel::~LWEUILabel() {}
