#include <LWVideo/LWFont.h>
#include <LWVideo/LWMesh.h>
#include <LWPlatform/LWFileStream.h>
#include <LWEUI/LWEUIAdvLabel.h>
#include <LWPlatform/LWWindow.h>
#include "LWELocalization.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include <algorithm>
#include <cstdarg>
#include <iostream>

LWEUIAdvLabel *LWEUIAdvLabel::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[1024*32]; //max of 32kb files.
	char SBuffer[1024 * 32];
	LWFileStream Stream;
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIAdvLabel *Label = Allocator->Allocate<LWEUIAdvLabel>("", nullptr, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
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
		Font = (LWFont*)FindAsset(Manager->GetAssetManager(), Res, LWEAsset::Font);
	}
	if (MaterAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), MaterAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Mat = Manager->GetMaterial(Res);
	}
	Label->SetFont(Font).SetMaterial(Mat);
	if (ValueAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		Label->SetText(Res);
	} else if (ValueSrcAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueSrcAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		if (!LWFileStream::OpenStream(Stream, Res, LWFileStream::ReadMode, *Allocator)) {
			std::cout << "Error opening file: '" << Res << "'" << std::endl;
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Label->SetText(Buffer);
		}

	}
	if (ScaleAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ScaleAttr->m_Value, ActiveComponent, ActiveComponentNode);
		Label->SetFontScale((float)atof(Res));
	}
	return Label;
}

LWEUI &LWEUIAdvLabel::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	uint32_t Flag = m_Flag;
	if (Mouse) {
		LWVector2i MP = Mouse->GetPosition();
		LWVector2f MousePnt = LWVector2f((float)MP.x, (float)MP.y);
		if (MousePnt.x >= m_VisiblePosition.x && MousePnt.x <= m_VisiblePosition.x + m_VisibleSize.x && MousePnt.y >= m_VisiblePosition.y && MousePnt.y <= m_VisiblePosition.y + m_VisibleSize.y) {
			Manager->DispatchEvent(this, Event_TempOverInc);
			m_Flag |= MouseOver;
		} else m_Flag &= ~MouseOver;
	}
	if (Touch) {
		for (uint32_t i = 0; i < Touch->GetPointCount(); i++) {
			auto Pnt = Touch->GetPoint(i);
			LWVector2f TouchPnt = LWVector2f((float)Pnt->m_Position.x, (float)Pnt->m_Position.y);
			bool Over = TouchPnt.x + Pnt->m_Size >= m_VisiblePosition.x && TouchPnt.x - Pnt->m_Size <= m_VisiblePosition.x + m_VisibleSize.x && TouchPnt.y + Pnt->m_Size >= m_VisiblePosition.y && TouchPnt.y - Pnt->m_Size <= m_VisiblePosition.y + m_VisibleSize.y;
			if (Over) {
				Manager->DispatchEvent(this, Event_TempOverInc | (i << Event_OverOffset));
				m_Flag |= MouseOver;
			} else m_Flag &= ~MouseOver;
		}
	}


	if (m_Flag&MouseOver && (Flag&MouseOver) == 0) Manager->DispatchEvent(this, Event_MouseOver);
	if ((m_Flag&MouseOver) == 0 && Flag&MouseOver) Manager->DispatchEvent(this, Event_MouseOff);
	return *this;
}

LWEUI &LWEUIAdvLabel::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	if (!m_Font) return *this;
	uint32_t Align = (m_Flag&(LabelLeftAligned | LabelCenterAligned | LabelRightAligned));
	LWVector2f Pos = m_VisiblePosition + LWVector2f(0.0f, m_Size.w*Scale - std::min<float>(m_Size.w*Scale, m_Font->GetLineSize()*m_FontScale*Scale)) - LWVector2f(0.0f, m_UnderHang*Scale);
	LWMesh<LWVertexUI> *Mesh = Frame->m_Mesh;

	uint32_t LineCnt = 0;
	LWVector2f FontPos = Pos;
	float LargestScale = 0.0f;
	LWGlyph *P = nullptr;
	for (uint32_t i = 0; i < m_TextPartCnt && LineCnt <= MaxParts; LineCnt++) {
		FontPos.x = Pos.x;
		if (Align == LabelCenterAligned) FontPos.x = (m_VisiblePosition.x + m_VisibleSize.x*0.5f - m_LineLength[LineCnt]*Scale*0.5f);
		else if (Align == LabelRightAligned) FontPos.x = (m_VisiblePosition.x + m_VisibleSize.x - m_LineLength[LineCnt]*Scale);
		for (uint32_t d = i; d < m_TextPartCnt; d++) {
			LargestScale = std::max<float>(LargestScale, m_TextParts[d].m_Scale);
			float LocalScale = m_TextParts[d].m_Scale*m_FontScale*Scale;
			LWVector4f Color = m_Material->m_Color*m_TextParts[d].m_ColorMult;
			const char *S = LWText::FirstCharacter(m_TextParts[d].m_Text);

			for(;S; S = LWText::Next(S)){
				uint32_t UTF = LWText::GetCharacter(S);
				LWGlyph *G = m_Font->GetGlyph(UTF);
				if (!G) {
					G = m_Font->GetErrorGlyph();
					if (!G) continue; //skip rendering characters we don't have a glyph for.
				}
				float Kern = 0;
				if (P) Kern = m_Font->GetKernBetween(P->m_Character, G->m_Character)*LocalScale;
				P = G;
				if (G->m_Size.x) {

					LWVector2f Position = LWVector2f(FontPos.x + Kern + G->m_Bearing.x*LocalScale, FontPos.y - G->m_Bearing.y*LocalScale);
					LWVector2f Size = G->m_Size*LocalScale;
					if (!Frame->WriteFontGlyph(m_Font->GetTexture(G->m_TextureIndex), Position, Size, G->m_TexCoord, Color)) break;
				}
				FontPos.x += (G->m_Advance.x*LocalScale + Kern);
			}

			if (m_TextParts[d].m_LineEnd) {
				i = d + 1;
				FontPos.y -= m_Font->GetLineSize()*m_FontScale*LargestScale*Scale;
				LargestScale = 0.0f;
				P = nullptr;
				break;
			}
		}
		
	}
	return *this;
}

LWEUIAdvLabel &LWEUIAdvLabel::SetText(const LWText &Text) {
	char SubBuffer[16];
	m_TextPartCnt = 0;
	uint32_t Len = 0;
	uint32_t Clr = 0;
	float Scale = 1.0f;
	LWVector4f ColorMult = LWVector4f(1.0f);
	m_TextParts[m_TextPartCnt].m_LineEnd = false;
	m_TextParts[m_TextPartCnt].m_Scale = Scale;
	m_TextParts[m_TextPartCnt].m_ColorMult = ColorMult;
	for (const char *C = (const char*)Text.GetCharacters(); *C; C++) {
		bool Split = (Len>=sizeof(LWETextPart::m_Text)-1 || *C=='\n');
		bool Skip = *C == '\n';
		if (*C == '\\') {
			if (*(C + 1) == '[') C++;
		}else if(*C=='['){
			const char *R = LWText::CopyToTokens(C + 1, SubBuffer, sizeof(SubBuffer), "]");
			if (R) {
				if (*SubBuffer == '#') {
					if (sscanf(SubBuffer, "#%x", &Clr)) {
						ColorMult.x = ((float)((Clr >> 24) & 0xFF) / 255.0f);
						ColorMult.y = ((float)((Clr >> 16) & 0xFF) / 255.0f);
						ColorMult.z = ((float)((Clr >> 8) & 0xFF) / 255.0f);
						ColorMult.w = ((float)(Clr & 0xFF) / 255.0f);
						Split = true;
					}
				} else if ((*SubBuffer >= '0' && *SubBuffer <= '9') || *SubBuffer == '.' || *SubBuffer == '+' || *SubBuffer == '-') {
					if (sscanf(SubBuffer, "%f", &Scale)) {
						Split = true;
					}
				} else {
					if (m_Font) {
						uint32_t Glyph = m_Font->GetGlyphName(SubBuffer);
						if (m_Font->GetGlyph(Glyph)) {
							uint32_t o = LWText::MakeUTF32To8(&Glyph, 1, (uint8_t*)(m_TextParts[m_TextPartCnt].m_Text+Len), sizeof(LWETextPart::m_Text) - Len);
							if(o) Len += (o-1);
						} else std::cout << "No glyph named: '" << SubBuffer << "'" << std::endl;
					} else std::cout << "no font bound." << std::endl;
				}
				Skip = true;
				C = R;
			}
		}

		if (Split) {
			if (*C == '\n') m_TextParts[m_TextPartCnt].m_LineEnd = true;
			m_TextParts[m_TextPartCnt].m_Text[Len] = '\0';
			m_TextPartCnt++;
			m_TextParts[m_TextPartCnt].m_LineEnd = false;
			m_TextParts[m_TextPartCnt].m_ColorMult = ColorMult;
			m_TextParts[m_TextPartCnt].m_Scale = Scale;
			Len = 0;
		}
		if(!Skip) m_TextParts[m_TextPartCnt].m_Text[Len++] = *C;
	}
	if (!Len && m_TextPartCnt) {
		m_TextParts[m_TextPartCnt - 1].m_LineEnd = true;
	} else {
		m_TextParts[m_TextPartCnt].m_Text[Len] = '\0';
		m_TextParts[m_TextPartCnt].m_LineEnd = true;
		m_TextPartCnt++;
	}
	return SetFont(m_Font);
}

LWEUIAdvLabel &LWEUIAdvLabel::SetTextf(const char *Format, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return SetText(Buffer);
}

LWEUIAdvLabel &LWEUIAdvLabel::SetFont(LWFont *Font) {
	m_Font = Font;
	if (m_Font) {
		uint32_t LineCnt = 0;
		LWGlyph *P = nullptr;
		float LargestScale = 0.0f;
		LWVector4f TextSize = LWVector4f();
		LWVector2f TextPos = LWVector2f();
		for (uint32_t i = 0; i < m_TextPartCnt && LineCnt<=MaxParts;LineCnt++) {
			for (uint32_t d = i; d < m_TextPartCnt; d++) {
				float LocalScale = m_FontScale*m_TextParts[d].m_Scale;
				const char *S = LWText::FirstCharacter(m_TextParts[d].m_Text);
				for (; S; S = LWText::Next(S)) {
					uint32_t UTF = LWText::GetCharacter(S);
					LWGlyph *G = m_Font->GetGlyph(UTF);
					float Kern = 0.0f;
					if (!G) {
						G = m_Font->GetErrorGlyph();
						if (!G) continue;
					}
					if (P) Kern = m_Font->GetKernBetween(P->m_Character, G->m_Character)*LocalScale;
					LWVector2f BtmLeftPnt = LWVector2f(TextPos.x + Kern + G->m_Bearing.x*LocalScale, TextPos.y - G->m_Bearing.y*LocalScale);
					LWVector2f TopRightPnt = LWVector2f(TextPos.x + Kern + G->m_Bearing.x*LocalScale + G->m_Size.x*LocalScale, TextPos.y - G->m_Bearing.y*LocalScale + G->m_Size.y*LocalScale);
					TextSize.x = std::min<float>(BtmLeftPnt.x, TextSize.x);
					TextSize.z = std::max<float>(TopRightPnt.x, TextSize.z);
					TextSize.y = std::max<float>(TopRightPnt.y, TextSize.y);
					TextSize.w = std::min<float>(BtmLeftPnt.y, TextSize.w);

					TextPos.x += (G->m_Advance.x*LocalScale + Kern);
				}

				if (m_TextParts[d].m_LineEnd) {
					m_LineLength[LineCnt] = TextPos.x;
					TextPos.y -= m_Font->GetLineSize()*m_FontScale*LargestScale;
					LargestScale = 0.0f;
					TextPos.x = 0.0f;

					i = d + 1;
					P = nullptr;
					break;
				}
			}
		}

		m_Size.z = (TextSize.z - TextSize.x);
		m_Size.w = (TextSize.y - TextSize.w);
		float LineCount = floorf((-TextSize.w / (Font->GetLineSize()*m_FontScale)))*Font->GetLineSize()*m_FontScale;
		m_UnderHang = (TextSize.w + LineCount);
	}
	return *this;
}

LWEUIAdvLabel &LWEUIAdvLabel::SetMaterial(LWEUIMaterial *Material) {
	m_Material = Material;
	return *this;
}

LWEUIAdvLabel &LWEUIAdvLabel::SetFontScale(float Scale) {
	m_FontScale = Scale;
	return SetFont(m_Font);
}

LWFont *LWEUIAdvLabel::GetFont(void) {
	return m_Font;
}

LWEUIMaterial *LWEUIAdvLabel::GetMaterial(void) {
	return m_Material;
}

const LWETextPart &LWEUIAdvLabel::GetTextPart(uint32_t i) const {
	return m_TextParts[i];
}

uint32_t LWEUIAdvLabel::GetTextPartCount(void) const {
	return m_TextPartCnt;
}

float LWEUIAdvLabel::GetFontScale(void) const {
	return m_FontScale;
}

LWEUIAdvLabel::LWEUIAdvLabel(const LWText &Text, LWFont *Font, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_Font(Font), m_Material(Material), m_FontScale(1.0f), m_TextPartCnt(0) {
	SetFont(Font);
	SetText(Text);
}

LWEUIAdvLabel::LWEUIAdvLabel() : LWEUI(LWVector4f(), LWVector4f(), 0), m_TextPartCnt(0), m_Material(nullptr), m_Font(nullptr), m_FontScale(1.0f), m_UnderHang(0.0f) {}

LWEUIAdvLabel::~LWEUIAdvLabel() {}
