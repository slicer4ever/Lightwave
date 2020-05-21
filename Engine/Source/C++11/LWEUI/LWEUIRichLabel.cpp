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

LWEUIRichLabel *LWEUIRichLabel::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[1024*32]; //max of 32kb files.
	char SBuffer[1024 * 32];
	LWFileStream Stream;
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIRichLabel *Label = Allocator->Allocate<LWEUIRichLabel>("", nullptr, *Allocator, nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);
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
		Font = Manager->GetAssetManager()->GetAsset<LWFont>(Res);
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
	const char *C = m_TextBuffer;
	if (!*C) return *this;

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
		uint32_t Len = Style.m_Length;
		while(Len>0){
			Len -= LWText::UTF8ByteSize(C);
			if (*C == '\n') {
				Line++;
				LSize = m_LineSizes[Line] * Scale;
				TextPos.x = Pos.x;
				TextPos.y -= LSize.y;
				if (Align == LabelCenterAligned) TextPos.x = (VisiblePos.x + VisibleSize.x*0.5f - LSize.x*0.5f);
				else if (Align == LabelRightAligned) TextPos.x = (VisiblePos.x + VisibleSize.x - LSize.x);
				P = nullptr;
			} else {
				uint32_t UTF = LWText::GetCharacter(C);
				LWGlyph *G = m_Font->GetGlyph(UTF);
				if (!G) G = m_Font->GetErrorGlyph();
				if (G) {
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
			C = LWText::Next(C);
		}		
	}
	return *this;
}

void LWEUIRichLabel::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUIRichLabel &LWEUIRichLabel::SetText(const LWText &Text) {
	char SubBuffer[256];
	const char *C = (const char*)Text.GetCharacters();
	uint32_t Len = (uint32_t)strlen(C)+1;
	char *oBuffer = nullptr;
	char *nBuffer = m_TextBuffer;
	if (Len > m_BufferLength) {
		LWAllocator *Alloc = LWAllocator::GetAllocator(m_TextBuffer);
		if (!Alloc) return *this;
		oBuffer = m_TextBuffer;
		nBuffer = Alloc->AllocateArray<char>(Len);
		m_BufferLength = Len;
	}
	m_StyleList.clear();
	LWEUITextStyle LStyle;
	uint32_t Color;
	uint32_t CallbackID = -1;
	char *V = nBuffer;
	*(V + Len - 1) = '\0';
	for (; *C; C++) {
		bool WriteChar = true;
		if (*C == '\\') {
			if (*(C + 1) == '[') C++;
		} else if (*C == '[') {
			WriteChar = false;
			const char *R = LWText::CopyToTokens(C + 1, SubBuffer, sizeof(SubBuffer), "]");
			if (!R) WriteChar = true;
			else {
				LWEUITextStyle NewStyle = LStyle;
				if (sscanf(SubBuffer, "#%x", &Color)) NewStyle.m_ColorMult = LWUNPACK_COLORVEC4f(Color);
				else if (sscanf(SubBuffer, "B#%x", &Color)) NewStyle.m_BackgroundColorMult = LWUNPACK_COLORVEC4f(Color);
				else if (sscanf(SubBuffer, "$%d", &CallbackID)) NewStyle.m_CallbackID = CallbackID;
				else if (*SubBuffer == '/' || *SubBuffer=='\\') NewStyle.m_CallbackID = -1;
				else if (!sscanf(SubBuffer, "%f", &NewStyle.m_Scale)) WriteChar = true;				
				if (!WriteChar) {
					if (LStyle.m_Length) m_StyleList.push_back(LStyle);
					LStyle = NewStyle;
					LStyle.m_Offset = (uint32_t)(uintptr_t)(V - m_TextBuffer);
					LStyle.m_Length = 0;
					C = R;
				}
			}
		}
		if (!WriteChar) continue;
		*V++ = *C;
		LStyle.m_Length++;
		if (*C == '\n') {
			m_StyleList.push_back(LStyle);
			LStyle.m_Offset = (uint32_t)(uintptr_t)(V - m_TextBuffer);
			LStyle.m_Length = 0;
		}
	}
	if (LStyle.m_Length) m_StyleList.push_back(LStyle);
	*V = '\0';
	m_TextBuffer = nBuffer;
	LWAllocator::Destroy(oBuffer);
	return SetFont(m_Font);
}

LWEUIRichLabel &LWEUIRichLabel::SetTextf(const char *Format, ...) {
	char Buffer[1024];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return SetText(Buffer);
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
	if (!*m_TextBuffer) return *this;
	uint32_t Count = (uint32_t)m_StyleList.size();
	const char *C = m_TextBuffer;
	LWVector2f LineSize = LWVector2f();
	//Calculate Line heights first.
	for (uint32_t i = 0; i < Count; i++) {
		LWEUITextStyle &Style = m_StyleList[i];
		uint32_t Len = Style.m_Length;
		float LScale = Style.m_Scale*m_FontScale;
		LineSize.y = std::max<float>(LScale*m_Font->GetLineSize(), LineSize.y);
		while (Len > 0) {
			Len -= LWText::UTF8ByteSize(C);
			if (*C == '\n') {
				m_LineSizes.push_back(LineSize);
				LineSize = LWVector2f();
			}
			C = LWText::Next(C);
		}
	}
	m_LineSizes.push_back(LineSize);
	LineSize = LWVector2f();

	C = m_TextBuffer;
	LWGlyph *P = nullptr;
	LWVector2f TextPos = LWVector2f();
	LWVector4f TextBounds = LWVector4f();
	uint32_t LineCount = 0;
	for(uint32_t i=0;i<Count;i++){
		LWEUITextStyle &Style = m_StyleList[i];
		float LScale = Style.m_Scale * m_FontScale;
		LWVector2f InitPos = TextPos;
		LWVector4f StyleBounds = LWVector4f(TextPos, TextPos);
		uint32_t Len = Style.m_Length;
		while(Len>0){
			Len -= LWText::UTF8ByteSize(C);
			if (*C == '\n') {
				P = nullptr;
				m_LineSizes[LineCount].x = LineSize.x;
				LineCount++;
				TextPos = LWVector2f(0.0f, TextPos.y - m_LineSizes[LineCount].y);
				LineSize = LWVector2f();
			}else{
				uint32_t UTF = LWText::GetCharacter(C);
				LWGlyph *G = m_Font->GetGlyph(UTF);
				if (!G) G = m_Font->GetErrorGlyph();
				if (G) {
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
			C = LWText::Next(C);
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

const char *LWEUIRichLabel::GetText(void) const {
	return m_TextBuffer;
}

LWEUIRichLabel::LWEUIRichLabel(const LWText &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_TextBuffer(Allocator.AllocateArray<char>(MinimumBufferSize)), m_Font(Font), m_Material(Material), m_FontScale(1.0f), m_BufferLength(MinimumBufferSize) {
	SetFont(Font);
	SetText(Text);
}

LWEUIRichLabel::LWEUIRichLabel() : LWEUI(LWVector4f(), LWVector4f(), 0), m_TextBuffer(nullptr), m_BufferLength(0), m_Material(nullptr), m_Font(nullptr), m_FontScale(1.0f), m_Overhang(0.0f) {}

LWEUIRichLabel::~LWEUIRichLabel() {
	LWAllocator::Destroy(m_TextBuffer);
}
