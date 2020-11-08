#include <LWVideo/LWFont.h>
#include <LWVideo/LWMesh.h>
#include <LWPlatform/LWFileStream.h>
#include <LWEUI/LWEUIRichLabel.h>
#include <LWPlatform/LWWindow.h>
#include "LWELocalization.h"
#include "LWEAsset.h"
#include "LWETypes.h"
#include <algorithm>
#include <cstdarg>
#include <iostream>

LWEUIRichLabel *LWEUIRichLabel::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[1024*32]; //max of 32kb files.
	char SBuffer[1024 * 32];
	LWFileStream Stream;
	LWAllocator &Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEAssetManager *AM = Manager->GetAssetManager();
	LWEUIRichLabel *Label = Allocator.Create<LWEUIRichLabel>("", nullptr, Allocator, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);
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
	Label->SetFont(Font).SetMaterial(Mat);
	if (ValueAttr) {
		LWUTF8Iterator Text = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Text)) Text = LWUTF8Iterator(SBuffer);
		Label->SetText(Text);
	} else if (ValueSrcAttr) {
		LWUTF8Iterator Path = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueSrcAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Path)) Path = LWUTF8Iterator(SBuffer);
		if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode, Allocator)) {
			fmt::print("Error opening file: '{}'\n", Path);
		} else {
			Stream.ReadText(Buffer, sizeof(Buffer));
			Label->SetText(Buffer);
		}
	}
	if (ScaleAttr) {
		LWUTF8Iterator Scale = ParseComponentAttribute(Buffer, sizeof(Buffer), ScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		Label->SetFontScale((float)atof((const char*)Scale()));
	}
	return Label;
}

LWEUI &LWEUIRichLabel::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	bool wasOver = (m_Flag&MouseOver) != 0;
	bool wasLDown = (m_Flag&MouseLDown) != 0;
	bool wasRDown = (m_Flag&MouseRDown) != 0;
	bool wasMDown = (m_Flag&MouseMDown) != 0;
	uint64_t Flag = (m_Flag&~(MouseOver|MouseLDown|MouseRDown|MouseMDown)) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (Flag&MouseOver) != 0;

	auto DispatchStyleEvent = [this, &Manager](LWEUIRichLabelCallback &Callback, LWEUITextStyle &Style, uint32_t Event, bool doDispatch)->bool {
		if (!doDispatch) return false;
		Callback(*this, Style, Event, Manager);
		return true;
	};
	LWVector2f Pnt = LWVector2f(-100.0f);

	if (Mouse) {
		if (Mouse->ButtonDown(LWMouseKey::Left)) Flag |= isOver ? MouseLDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Middle)) Flag |= isOver ? MouseMDown : 0;
		if (Mouse->ButtonDown(LWMouseKey::Right)) Flag |= isOver ? MouseRDown : 0;
		Pnt = Mouse->GetPositionf();
	}
	bool isLDown = (Flag&MouseLDown) != 0;
	bool isRDown = (Flag&MouseRDown) != 0;
	bool isMDown = (Flag&MouseMDown) != 0;

	if (m_CallbackMap.size()) {
		uint32_t Count = (uint32_t)m_StyleList.size();
		for (uint32_t i = 0; i < Count; i++) {
			LWEUITextStyle &Style = m_StyleList[i];
			auto Iter = m_CallbackMap.find(Style.m_CallbackID);
			if (Iter == m_CallbackMap.end()) continue;
			LWVector4f Bounds = Style.m_VisibileBounds;
			bool SWasOver = (Style.m_Flag&MouseOver) != 0;
			bool SWasLDown = (Style.m_Flag&MouseLDown) != 0;
			bool SWasRDown = (Style.m_Flag&MouseRDown) != 0;
			bool SWasMDown = (Style.m_Flag&MouseMDown) != 0;
			bool SisOver = Pnt.x >= Bounds.x && Pnt.x <= Bounds.z && Pnt.y >= Bounds.w && Pnt.y <= Bounds.y;
			Style.m_Flag = (Style.m_Flag&~(MouseOver | MouseLDown | MouseMDown | MouseRDown)) | (SisOver ? (MouseOver | (Flag&(MouseLDown | MouseMDown | MouseRDown))) : 0);
			bool SisLDown = (Style.m_Flag&MouseLDown) != 0;
			bool SisRDown = (Style.m_Flag&MouseRDown) != 0;
			bool SisMDown = (Style.m_Flag&MouseMDown) != 0;

			DispatchStyleEvent(Iter->second, Style, Event_MouseOver, SisOver && !SWasOver);
			DispatchStyleEvent(Iter->second, Style, Event_MouseOff, !SisOver && SWasOver);
			DispatchStyleEvent(Iter->second, Style, Event_Released, SisOver && (!SisLDown && SWasLDown));
			DispatchStyleEvent(Iter->second, Style, Event_RReleased, SisOver && (!SisRDown && SWasRDown));
			DispatchStyleEvent(Iter->second, Style, Event_MReleased, SisOver && (!SisMDown && SWasMDown));
			DispatchStyleEvent(Iter->second, Style, Event_Pressed, SisOver && SisLDown && !SWasLDown);
			DispatchStyleEvent(Iter->second, Style, Event_RPressed, SisOver && SisRDown && !SWasRDown);
			DispatchStyleEvent(Iter->second, Style, Event_MPressed, SisOver && SisMDown && !SWasMDown);
		}
	}
	m_Flag = Flag;
	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	return *this;
}

LWEUI &LWEUIRichLabel::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	if (!m_Font) return *this;
	if (!m_StyleList.size()) return *this;
	uint32_t Align = (m_Flag&(LabelLeftAligned | LabelCenterAligned | LabelRightAligned));
	uint32_t VAlign = (m_Flag&(LabelBottomAligned | LabelVCenterAligned | LabelTopAligned));
	
	float LineSize = m_LineSizes[0].y*Scale;
	LWVector2f Pos = VisiblePos + LWVector2f(0.0f, VisibleSize.y-m_Overhang*Scale);
	LWVector2f TSize = m_TextSize * Scale;
	
	if (VAlign == LabelVCenterAligned) Pos.y += (VisibleSize.y - TSize.y)*0.5f;
	else if (VAlign = LabelTopAligned) Pos.y += (VisibleSize.y - TSize.y);
	
	LWVector4f Color = m_Material ? m_Material->m_ColorA : LWVector4f(1.0f);
	LWVector2f TextPos = Pos;
	uint32_t Count = (uint32_t)m_StyleList.size();
	uint32_t Line = 0;
	LWGlyph *P = nullptr;
	LWVector2f LSize = m_LineSizes[Line] * Scale;
	if (Align == LabelCenterAligned) TextPos.x = (VisiblePos.x + VisibleSize.x*0.5f - LSize.x*0.5f);
	else if (Align == LabelRightAligned) TextPos.x = (VisiblePos.x + VisibleSize.x - LSize.x);
	
	for (uint32_t i = 0; i < Count; i++) {
		LWEUITextStyle &Style = m_StyleList[i];
		float LScale = Style.m_Scale*m_FontScale*Scale;
		float iLScale = 1.0f / LScale;
		LWVector4f LColor = Color * Style.m_ColorMult;
		Style.m_VisibileBounds = Style.m_Bounds*Scale + LWVector4f(TextPos, TextPos);
		if (Style.m_BackgroundColorMult.w > 0.0f) {
			LWEUIMaterial Mat;
			Mat.m_ColorA = Color * Style.m_BackgroundColorMult;
			if (!Frame.WriteRect(&Mat, Style.m_VisibileBounds.xw(), Style.m_VisibileBounds.zy() - Style.m_VisibileBounds.xw())) break;
		}
		for(LWUTF8GraphemeIterator G = Style.m_Iterator; !G.AtEnd(); ++G) {
			for(LWUTF8Iterator C = G.ClusterIterator(); !C.AtEnd(); ++C) {
				uint32_t CP = *C;
				if(CP=='\n') {
					Line++;
					LSize = m_LineSizes[Line] * Scale;
					TextPos.x = Pos.x;
					TextPos.y -= LSize.y;
					if (Align == LabelCenterAligned) TextPos.x = (VisiblePos.x + VisibleSize.x*0.5f - LSize.x*0.5f);
					else if (Align == LabelRightAligned) TextPos.x = (VisiblePos.x + VisibleSize.x - LSize.x);
					P = nullptr;
				} else {
					LWGlyph *G = m_Font->GetGlyph(CP);
					if (!G) {
						G = m_Font->GetErrorGlyph();
						if (!G) continue;
					}
					float Kern = 0.0f;
					if (P) Kern = m_Font->GetKernBetween(P->m_Character, G->m_Character)*LScale;
					P = G;
					if (G->m_Size.x) {
						LWVector2f Ps = LWVector2f(TextPos.x + Kern + G->m_Bearing.x*LScale, TextPos.y - G->m_Bearing.y*LScale);
						LWVector2f Size = G->m_Size*LScale;
						if (!Frame.WriteFontGlyph(m_Font->GetTexture(G->m_TextureIndex), Ps, Size, G->m_TexCoord, G->m_SignedRange*iLScale, LColor)) break;
					}
					TextPos.x += G->m_Advance.x*LScale + Kern;
				}
			}
		}
	}
	return *this;
}

void LWEUIRichLabel::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUIRichLabel &LWEUIRichLabel::SetText(const LWUTF8Iterator &Text) {
	m_Text = Text;
	m_StyleList.clear();
	LWEUITextStyle LStyle;
	uint32_t Color;
	uint32_t CallbackID = -1;
	LWUTF8GraphemeIterator P = m_Text.beginGrapheme();
	for (LWUTF8GraphemeIterator G = P; !G.AtEnd(); ++G) {
		if (*G == '\\') {
			if (*(G + 1) == '[') ++G;
		} else if (*G == '[') {
			LWUTF8GraphemeIterator CloseBrack = G.NextToken(']');
			if (CloseBrack.AtEnd()) continue;
			LStyle.m_Iterator = LWUTF8GraphemeIterator(P, G);
			LWEUITextStyle pStyle = LStyle;
			G = (G + 1).NextWord(true);
			bool Valid = true;
			if (sscanf((const char*)G(), "#%x", &Color)) LStyle.m_ColorMult = LWUNPACK_COLORVEC4f(Color);
			else if (sscanf((const char*)G(), "B#%x", &Color)) LStyle.m_BackgroundColorMult = LWUNPACK_COLORVEC4f(Color);
			else if (sscanf((const char*)G(), "$%d", &CallbackID)) LStyle.m_CallbackID = CallbackID;
			else if (*G == '/' || *G == '\\') LStyle.m_CallbackID = -1;
			else sscanf((const char*)G(), "%f", &LStyle.m_Scale);
			if (!pStyle.m_Iterator.AtEnd()) m_StyleList.push_back(pStyle);
			G = CloseBrack + 1;
			P = G;

		}
	}
	return SetFont(m_Font);
}

LWEUIRichLabel &LWEUIRichLabel::RegisterCallback(uint32_t CallbackID, LWEUIRichLabelCallback Callback) {
	m_CallbackMap.insert_or_assign(CallbackID, Callback);
	return *this;
}

LWEUIRichLabel &LWEUIRichLabel::UnregisterCallback(uint32_t CallbackID) {
	auto Iter = m_CallbackMap.find(CallbackID);
	if (Iter == m_CallbackMap.end()) return *this;
	m_CallbackMap.erase(Iter);
	return *this;
}

LWEUIRichLabel &LWEUIRichLabel::SetFont(LWFont *Font) {
	m_Font = Font;
	m_LineSizes.clear();
	if (!m_Font) return *this;
	if (!m_StyleList.size()) return *this;
	uint32_t Count = (uint32_t)m_StyleList.size();
	LWVector2f LineSize = LWVector2f();

	//Need to first calculate heights first.
	for (uint32_t i = 0; i < Count; i++) {
		LWEUITextStyle &Style = m_StyleList[i];
		LWUTF8GraphemeIterator G = m_StyleList[i].m_Iterator;
		float LScale = Style.m_Scale * m_FontScale;
		LineSize.y = std::max<float>(LScale * m_Font->GetLineSize(), LineSize.y);
		for (; !G.AtEnd(); ++G) {
			if (*G == '\n') {
				m_LineSizes.push_back(LineSize);
				LineSize = LWVector2f();
			}
		}
	}
	m_LineSizes.push_back(LineSize);
	LineSize = LWVector2f();
	
	//Calculate widths now:
	LWGlyph *P = nullptr;
	LWVector2f TextPos = LWVector2f();
	LWVector4f TextBounds = LWVector4f();
	uint32_t LineCount = 0;
	for(uint32_t i=0;i<Count;i++){
		LWEUITextStyle &Style = m_StyleList[i];
		float LScale = Style.m_Scale * m_FontScale;
		LWVector2f InitPos = TextPos;
		LWVector4f StyleBounds = LWVector4f(TextPos, TextPos);
		LWUTF8GraphemeIterator G = Style.m_Iterator;
		for(;!G.AtEnd();++G) {
			for(LWUTF8Iterator C = G.ClusterIterator(); !C.AtEnd(); ++C) {
				uint32_t CP = *C;
				if (CP == '\n') {
					P = nullptr;
					m_LineSizes[LineCount].x = LineSize.x;
					LineCount++;
					TextPos = LWVector2f(0.0f, TextPos.y - m_LineSizes[LineCount].y);
					LineSize = LWVector2f();
				}else {
					LWGlyph *G = m_Font->GetGlyph(CP);
					if (!G) {
						G = m_Font->GetErrorGlyph();
						if (!G) continue;
					}
					float Kern = 0.0f;
					if (P) Kern = m_Font->GetKernBetween(P->m_Character, G->m_Character)*LScale;
					P = G;
					LWVector2f BtmLeftPnt = LWVector2f(TextPos.x + Kern + G->m_Bearing.x*LScale, TextPos.y - G->m_Bearing.y*LScale);
					LWVector2f TopRightPnt = BtmLeftPnt + G->m_Size*LScale;
					TextBounds.x = std::min<float>(BtmLeftPnt.x, TextBounds.x);
					TextBounds.z = std::max<float>(TopRightPnt.x, TextBounds.z);
					TextBounds.y = std::max<float>(TopRightPnt.y, TextBounds.y);
					TextBounds.w = std::min<float>(BtmLeftPnt.y, TextBounds.w);

					StyleBounds.x = std::min<float>(BtmLeftPnt.x, StyleBounds.x);
					StyleBounds.z = std::max<float>(TopRightPnt.x, StyleBounds.z);
					StyleBounds.y = std::max<float>(TopRightPnt.y, StyleBounds.y);
					StyleBounds.w = std::min<float>(BtmLeftPnt.y, StyleBounds.w);
					TextPos.x += G->m_Advance.x*LScale + Kern;
					LineSize.x = TextPos.x;
				}
			}
		}
		m_StyleList[i].m_Bounds = StyleBounds - LWVector4f(InitPos, InitPos);
	}
	m_LineSizes[LineCount].x = LineSize.x;
	LineCount++;

	m_TextSize = LWVector2f(TextBounds.z - TextBounds.x, TextBounds.y - TextBounds.w);
	if ((m_Flag&NoAutoWidthSize) == 0) m_Size.z = m_TextSize.x;
	if ((m_Flag&NoAutoHeightSize) == 0) m_Size.w = m_TextSize.y;
	m_Overhang = TextBounds.y;
	return *this;
}

LWEUIRichLabel &LWEUIRichLabel::SetMaterial(LWEUIMaterial *Material) {
	m_Material = Material;
	return *this;
}

LWEUIRichLabel &LWEUIRichLabel::SetFontScale(float Scale) {
	m_FontScale = Scale;
	return SetFont(m_Font);
}

LWFont *LWEUIRichLabel::GetFont(void) {
	return m_Font;
}

LWEUIMaterial *LWEUIRichLabel::GetMaterial(void) {
	return m_Material;
}

float LWEUIRichLabel::GetFontScale(void) const {
	return m_FontScale;
}

const LWEUITextStyle &LWEUIRichLabel::GetTextStyle(uint32_t i) const {
	return m_StyleList[i];
}

uint32_t LWEUIRichLabel::GetStyleCount(void) const {
	return (uint32_t)m_StyleList.size();
}

LWVector2f LWEUIRichLabel::GetLineSize(uint32_t i) const {
	return m_LineSizes[i];
}

uint32_t LWEUIRichLabel::GetLineCount(void) const {
	return (uint32_t)m_LineSizes.size();
}

const LWUTF8 &LWEUIRichLabel::GetText(void) const {
	return m_Text;
}

LWEUIRichLabel::LWEUIRichLabel(const LWUTF8Iterator &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_Font(Font), m_Material(Material), m_FontScale(1.0f), m_BufferLength(MinimumBufferSize) {
	SetFont(Font);
	SetText(Text);
}

LWEUIRichLabel::~LWEUIRichLabel() {}
