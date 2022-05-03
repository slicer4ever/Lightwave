#include <LWPlatform/LWWindow.h>
#include <LWVideo/LWFont.h>
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWInputDevice.h>
#include <LWPlatform/LWWindow.h>
#include "LWEUI/LWEUITextInput.h"
#include "LWELocalization.h"
#include "LWEAsset.h"
#include <cstdarg>
#include <algorithm>
#include <iostream>



//LWETextLine
char8_t LWETextLine::PasswordField[LWETEXTINPUT_MAXLINELENGTH] = "";

LWUTF8Iterator LWETextLine::GetValue(void) const {
	return LWUTF8Iterator(0, m_Value, m_Value, m_Value+m_RawLength-1);
}

LWUTF8Iterator LWETextLine::GetValueLast(void) const {
	return LWUTF8Iterator(m_Length, m_Value + m_RawLength-1, m_Value, m_Value + m_RawLength-1);
};

LWUTF8Iterator LWETextLine::GetValue(bool isPassword) const {
	const char8_t *Src = isPassword ? PasswordField : m_Value;
	return LWUTF8Iterator(0, Src, Src, Src + m_RawLength - 1);
}

LWUTF8Iterator LWETextLine::GetValueLast(bool isPassword) const {
	const char8_t *Src = isPassword ? PasswordField : m_Value;
	return LWUTF8Iterator(m_Length, Src + m_RawLength - 1, Src, Src + m_RawLength - 1); //we don't know the actual character count, but at minimum it's Length, this shouldn't be necessary for how this method is used.
}

LWUTF8GraphemeIterator LWETextLine::GetValueGrapheme(bool isPassword) const {
	const char8_t *Src = isPassword ? PasswordField : m_Value;
	return LWUTF8GraphemeIterator(0, 0, Src, Src, Src + m_RawLength - 1);
}

LWUTF8GraphemeIterator LWETextLine::GetValueGraphemeLast(bool isPassword) const {
	const char8_t *Src = isPassword ? PasswordField : m_Value;
	return LWUTF8GraphemeIterator(m_Length, m_Length, Src + m_RawLength-1, Src, Src + m_RawLength-1); //we don't know the actual character count, but at minimum it's Length, this shouldn't be necessary for how this method is used.
}

LWUTF8Iterator LWETextLine::UpdateIterator(const LWUTF8Iterator &Pos) const {
	return LWUTF8Iterator(Pos.m_Index, Pos(), m_Value, m_Value + m_RawLength-1);
}

uint32_t LWETextLine::Insert(const LWUTF8Iterator &Pos, const LWUTF8Iterator &Source) {
	uint32_t NLen, NRawLen;
	if (!LWUTF8Iterator::ValidateIterator(Source, NLen, NRawLen)) return 0;
	if (m_RawLength + --NRawLen > LWETEXTINPUT_MAXLINELENGTH) return 0; //Make sure we won't overflow.
	std::copy_backward(m_Value + Pos.RawIndex(), m_Value + m_RawLength, m_Value + NRawLen + m_RawLength);
	std::copy(Source(), Source() + NRawLen, m_Value + Pos.RawIndex());
	m_RawLength += NRawLen;
	m_Length += NLen;
	return NLen;
}

uint32_t LWETextLine::Insert(const LWUTF8Iterator &Pos, uint32_t Char) {
	uint32_t r = LWUTF8Iterator::CodePointUnitSize(Char);
	if (m_RawLength + r > LWETEXTINPUT_MAXLINELENGTH) return 0;
	std::copy_backward(m_Value + Pos.RawIndex(), m_Value + m_RawLength, m_Value + m_RawLength + r);
	LWUTF8Iterator::EncodeCodePoint(m_Value + Pos.RawIndex(), r, Char);
	m_RawLength += r;
	m_Length++;
	return 1;
}

uint32_t LWETextLine::Erase(const LWUTF8Iterator &Begin, const LWUTF8Iterator &End) {
	const char8_t *bP = Begin();
	const char8_t *eP = End();
	LWVerify(bP >= m_Value && bP <= m_Value + m_RawLength && eP >= m_Value && eP <= m_Value + m_RawLength);
	if (eP < bP) return Erase(End, Begin);
	uint32_t bIndex = Begin.RawIndex();
	uint32_t eIndex = End.RawIndex();
	uint32_t Len = Begin.Distance(End);
	std::copy(m_Value + eIndex, m_Value + m_RawLength, m_Value + bIndex);
	m_RawLength -= eIndex - bIndex;
	m_Length -= Len;
	return Len;
}

uint32_t LWETextLine::Clear(void) {
	uint32_t r = m_Length;
	m_Length = 0;
	m_RawLength = 1;
	*m_Value = '\0';
	return r;
}

LWETextLine::LWETextLine() {
	if (!*PasswordField) std::fill(PasswordField, PasswordField + sizeof(PasswordField), '*');
}


//LWETextInputCursor
LWUTF8Iterator LWETextInputCursor::AsIterator(const LWUTF8Iterator &SrcData) {
	return LWUTF8Iterator(m_Position.m_Index, SrcData() + m_Position.RawIndex(), SrcData.GetFirst(), SrcData.GetLast());
}

LWUTF8GraphemeIterator LWETextInputCursor::AsGrapheme(const LWUTF8GraphemeIterator &SrcData) {
	return LWUTF8GraphemeIterator(m_Position.m_Index, m_Position.m_Index, SrcData() + m_Position.RawIndex(), SrcData.GetFirst(), SrcData.GetLast());
}

LWETextInputCursor::LWETextInputCursor(uint32_t Line, const LWUTF8Iterator &Pos) : m_Line(Line), m_Position(Pos) {}

//LWEUITextInput
LWEUITextInput *LWEUITextInput::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator &Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEAssetManager *AM = Manager->GetAssetManager();
	LWEUITextInput *TextInput = Allocator.Create<LWEUITextInput>(LWVector4f(0.0f), LWVector4f(0.0f), FocusAble | TabAble);
	LWEUI::XMLParse(TextInput, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWEXMLAttribute *BorderMaterialAttr = FindAttribute(Node, Style, "BorderMaterial");
	LWEXMLAttribute *FontMaterialAttr = FindAttribute(Node, Style, "FontMaterial");
	LWEXMLAttribute *SelectMaterialAttr = FindAttribute(Node, Style, "SelectMaterial");
	LWEXMLAttribute *DefaultMaterialAttr = FindAttribute(Node, Style, "DefaultMaterial");
	LWEXMLAttribute *TextAreaMaterialAttr = FindAttribute(Node, Style, "TextAreaMaterial");
	LWEXMLAttribute *CursorMaterialAttr = FindAttribute(Node, Style, "CursorMaterial");
	LWEXMLAttribute *AllowedCharactersAttr = FindAttribute(Node, Style, "AllowedCharacters");
	LWEXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWEXMLAttribute *ValueAttr = FindAttribute(Node, Style, "Value");
	LWEXMLAttribute *DefaultAttr = FindAttribute(Node, Style, "Default");
	LWEXMLAttribute *BorderSizeAttr = FindAttribute(Node, Style, "BorderSize");
	LWEXMLAttribute *CursorSizeAttr = FindAttribute(Node, Style, "CursorSize");
	LWEXMLAttribute *MaxLengthAttr = FindAttribute(Node, Style, "MaxLength");
	LWEXMLAttribute *MaxLinesAttr = FindAttribute(Node, Style, "MaxLines");
	LWEXMLAttribute *FontScaleAttr = FindAttribute(Node, Style, "FontScale");
	LWEXMLAttribute *DefaultFontScaleAttr = FindAttribute(Node, Style, "DefaultFontScale");
	LWEXMLAttribute *BtnOffAttr = FindAttribute(Node, Style, "OffMaterial");
	LWEXMLAttribute *BtnOverAttr = FindAttribute(Node, Style, "OverMaterial");
	LWEXMLAttribute *BtnDownAttr = FindAttribute(Node, Style, "DownMaterial");
	LWEXMLAttribute *BtnFontScaleAttr = FindAttribute(Node, Style, "ButtonFontScale");
	LWEXMLAttribute *BtnFontMaterialAttr = FindAttribute(Node, Style, "ButtonFontMaterial");

	LWEUIMaterial *BorderMat = nullptr;
	LWEUIMaterial *FontMat = nullptr;
	LWEUIMaterial *BtnFontMat = nullptr;
	LWEUIMaterial *SelectMat = nullptr;
	LWEUIMaterial *DefaultMat = nullptr;
	LWEUIMaterial *TextAreaMat = nullptr;
	LWEUIMaterial *CursorMat = nullptr;
	LWEUIMaterial *BtnOffMat = nullptr;
	LWEUIMaterial *BtnDownMat = nullptr;
	LWEUIMaterial *BtnOverMat = nullptr;

	LWFont *Font = nullptr;
	float BorderSize = 1.0f;
	float CursorSize = 1.0f;
	uint32_t MaxLength = 0xFFFFFFFF;
	uint32_t MaxLines = 0xFFFFFFFF;
	if (BorderMaterialAttr) BorderMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BorderMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (FontMaterialAttr) FontMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), FontMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (DefaultMaterialAttr) DefaultMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BtnFontMaterialAttr) BtnFontMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (SelectMaterialAttr) SelectMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), SelectMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (TextAreaMaterialAttr) TextAreaMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), TextAreaMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (CursorMaterialAttr) CursorMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), CursorMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BtnOffAttr) BtnOffMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOffAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BtnDownAttr) BtnDownMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnDownAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BtnOverAttr) BtnOverMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOverAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (FontAttr) Font = AM->GetAsset<LWFont>(ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BorderSizeAttr) BorderSize = (float)atof((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), BorderSizeAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
	if (CursorSizeAttr) CursorSize = (float)atof((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), CursorSizeAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
	if (MaxLengthAttr) MaxLength = (uint32_t)atoi((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLengthAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
	if (MaxLinesAttr) MaxLines = (uint32_t)atoi((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLinesAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
	if (FontScaleAttr) {
		float Scale = (float)atof((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), FontScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
		TextInput->SetFontScale(Scale).SetDefaultFontScale(Scale);
	}
	if (DefaultFontScaleAttr) {
		float Scale = (float)atof((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultFontScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode)());
		TextInput->SetDefaultFontScale(Scale);
	}
	if (BtnFontScaleAttr) TextInput->SetBtnFontScale((float)atof((const char*)ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode)()));
	if (AllowedCharactersAttr) TextInput->SetAllowedCharacters(ParseComponentAttribute(Buffer, sizeof(Buffer), AllowedCharactersAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	TextInput->SetBorderMaterial(BorderMat).SetFontMaterial(FontMat).SetDefaultMaterial(DefaultMat).SetSelectMaterial(SelectMat).SetTextAreaMaterial(TextAreaMat).SetCursorMaterial(CursorMat).SetBtnOffMaterial(BtnOffMat).SetBtnOverMaterial(BtnOverMat).SetBtnDownMaterial(BtnDownMat).SetBtnFontMaterial(BtnFontMat);
	TextInput->SetFont(Font).SetBorderSize(BorderSize).SetCursorSize(CursorSize).SetMaxLength(MaxLength).SetMaxLines(MaxLines);
	if (DefaultAttr) {
		TextInput->SetDefaultText(ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultAttr->m_Value, ActiveComponent, ActiveComponentNode));
	}
	if (ValueAttr) {
		LWUTF8Iterator Val = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize && Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Val)) Val = LWUTF8Iterator(SBuffer);
		TextInput->InsertText(Val, false, false, 1.0f);
	}
	return TextInput;
}

LWEUITextInput &LWEUITextInput::UpdateTouchButtons(LWEUIManager &Manager, float Scale, uint64_t lCurrentTime) {
	const LWVector2f DefaultPos = LWVector2f(-1000.0f, -1000.0f);
	auto UpdateButton = [this](const LWTouchPoint &TP, uint32_t TPIndex, LWEUIManager &Manager, LWETextInputTouchBtn &Btn, float Scale, uint64_t lCurrentTime, LWKey lKeyPress, bool ShiftDown, bool CtrlDown)->void {
		LWVector2f DownPnt = LWVector2f(-1000.0f, -1000.0f);
		float BtnHeight = m_Font->GetLineSize()*m_FontScale;
		LWVector2f Size = LWVector2f(Btn.m_Size.x, BtnHeight)*Scale;
		bool isOver = PointInside(TP.m_Position.CastTo<float>(), TP.m_Size*Scale, Btn.m_Position, Size);
		Btn.m_Flag = isOver ? LWETextInputTouchBtn::ButtonOver : 0;
		if (TP.m_State == LWTouchPoint::UP) {
			if(isOver) ProcessKey(lKeyPress, ShiftDown, CtrlDown, Scale);
			return;
		}
		if (Manager.DispatchEvent(this, LWEUI::Event_TempOverInc | (TPIndex << LWEUI::Event_OverOffset), isOver)){
			Btn.m_Flag |= LWETextInputTouchBtn::ButtonDown;
		}
		return;
	};
	LWTouch *Touch = Manager.GetWindow()->GetTouchDevice();
	if (!Touch) return *this;
	if (!m_Font) return *this;
	if ((m_Flag&(SelectEnabled | TouchEnabled)) != (SelectEnabled | TouchEnabled)) return *this;
	CalculateTouchBtnPositions(Scale);
	uint32_t TouchCnt = Touch->GetPointCount();
	for (uint32_t i = 0; i < TouchCnt; i++) {
		const LWTouchPoint &TP = Touch->GetPoint(i);
		UpdateButton(TP, i, Manager, m_CopyBtn, Scale, lCurrentTime, LWKey::C, false, true);
		UpdateButton(TP, i, Manager, m_CutBtn, Scale, lCurrentTime, LWKey::X, false, true);
		UpdateButton(TP, i, Manager, m_PasteBtn, Scale, lCurrentTime, LWKey::P, false, true);
		UpdateButton(TP, i, Manager, m_SelectAllBtn, Scale, lCurrentTime, LWKey::A, false, true);
	}
	return *this;
}

LWEUITextInput &LWEUITextInput::DrawTouchButtons(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, uint64_t lCurrentTime) {
	auto DrawBtn = [this](LWEUIFrame &Frame, const LWETextInputTouchBtn &Btn, float Scale)->bool {
		if (Btn.m_Position.x < 0.0f) return true;
		float BtnLineSize = m_Font->GetLineSize()*m_FontScale;
		bool isOver = (Btn.m_Flag&LWETextInputTouchBtn::ButtonOver) != 0;
		bool isDown = (Btn.m_Flag&LWETextInputTouchBtn::ButtonDown) != 0;
		LWEUIMaterial *Mat = isOver ? (isDown ? m_BtnDownMaterial : m_BtnOverMaterial) : m_BtnOffMaterial;
		return Frame.WriteRect(Mat, Btn.m_Position, LWVector2f(Btn.m_Size.x, BtnLineSize)*Scale);
	};
	if (!m_BtnFontMaterial) return *this;
	if ((m_Flag&(SelectEnabled | TouchEnabled)) != (SelectEnabled | TouchEnabled)) return *this;

	if (!DrawBtn(Frame, m_CopyBtn, Scale)) return *this;
	if (!DrawBtn(Frame, m_CutBtn, Scale)) return *this;
	if (!DrawBtn(Frame, m_PasteBtn, Scale)) return *this;
	if (!DrawBtn(Frame, m_SelectAllBtn, Scale)) return *this;
	return *this;
}

LWEUI &LWEUITextInput::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWKeyboard *Keyboard = Wnd->GetKeyboardDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();

	bool wasOver = (m_Flag&MouseOver) != 0;
	bool wasLDown = (m_Flag&MouseLDown) != 0;
	uint64_t Flag = (m_Flag & ~(MouseOver | MouseLDown | MouseMDown | MouseRDown)) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (Flag&MouseOver) != 0;
	bool isFocused = Manager.GetFocusedUI() == this;
	bool isFocusable = isFocusAble();
	bool isPassword = isPasswordField();
	bool SelectPressed = false;
	bool Changed = false;
	
	LWVector2f DownPnt = LWVector2f(-1000.0f, -1000.0f);
	UpdateTouchButtons(Manager, Scale, lCurrentTime);

	if (Mouse) {
		LWVector2f MP = Mouse->GetPositionf();
		if (Mouse->ButtonDown(LWMouseKey::Left)) {
			if (isOver) {
				Flag |= MouseLDown;
				DownPnt = MP;
			} else if (isFocused) {
				m_SelectCursorLength = 0;
				Manager.SetFocused(nullptr);
			}
		}else{
			if (isOver) {
				if (Mouse->DoubleClicked()) {
					ProcessKey(LWKey::N, false, true, m_FontScale*Scale);
					SelectPressed = true;
				}
			}
		}
	}
	if (Touch) {
		const LWGesture &Gest = Touch->GetGesture();
		if (Gest.m_Type == LWGesture::Press) {
			ProcessKey(LWKey::N, false, true, m_FontScale*Scale);
			SelectPressed = true;
		} else {
			if (!(m_Flag&SelectEnabled)) {
				if (Gest.m_Type == LWGesture::Flick) {
					m_ScrollAcceleration = Gest.m_Direction;
				} else if (Gest.m_Type == LWGesture::Drag) {
					m_ScrollAcceleration = Gest.m_Direction;
				}
			}
		}
		for (uint32_t i = 0; i < Touch->GetPointCount(); i++) {
			const LWTouchPoint &Pnt = Touch->GetPoint(i);
			LWVector2f TouchPnt = Pnt.m_Position.CastTo<float>();
			float TSize = Pnt.m_Size*Scale;
			if (isOver) {
				if (Pnt.m_State == LWTouchPoint::UP) continue;
				Flag |= MouseLDown;
				if (!(m_Flag&TouchEnabled)) {
					m_Flag |= TouchEnabled;
					Manager.SetFocused(this);
				}
			} else if (Pnt.m_State == LWTouchPoint::UP && isFocused) {
				m_SelectCursorLength = 0;
				Manager.SetFocused(nullptr);
			}
		}
	}

	if (m_ScrollAcceleration.x > 0.0f || m_ScrollAcceleration.y > 0.0f) {
		SetScroll(m_HorizontalScroll + m_ScrollAcceleration.x, m_VerticalScroll + m_ScrollAcceleration.y, Scale);
		m_ScrollAcceleration *= 0.9f;
		Changed = true;
	}
	m_Flag = (m_Flag&~(MouseOver | MouseLDown)) | (Flag&(MouseOver | MouseLDown));
	bool isLDown = (Flag&MouseLDown) != 0;

	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	Manager.DispatchEvent(this, Event_Released, isOver && !isLDown && wasLDown);
	if (Manager.DispatchEvent(this, Event_Pressed, isOver && isLDown && !wasLDown)) {
		if (isFocusable) {
			Manager.SetFocused(this);
			isFocused = true;
		}
	}
	if(isFocused){
		if (Keyboard) {
			uint32_t KeyChangeCnt = Keyboard->GetKeyChangeCount();
			uint32_t CharCnt = Keyboard->GetCharPressed();
			bool Shift = Keyboard->ButtonDown(LWKey::LShift) || Keyboard->ButtonDown(LWKey::RShift);
			bool Ctrl = Keyboard->ButtonDown(LWKey::LCtrl) || Keyboard->ButtonDown(LWKey::RCtrl);
			for (uint32_t i = 0; i < KeyChangeCnt; i++) {
				if (Keyboard->GetKeyState(i)) {
					if (ProcessKey((LWKey)Keyboard->GetKeyChanged(i), Shift, Ctrl, Scale)) Changed = true;
				}
			}
			for (uint32_t i = 0; i < CharCnt; i++) {
				if (Keyboard->GetChar(i) == '\n') continue;
				if (InsertChar(Keyboard->GetChar(i), Shift, Ctrl, Scale)) Changed = true;
			}
		}

		float MaxVerticalScroll = 0.0f;
		float LineHeight = 0.0f;
		float FScale = m_FontScale * Scale;
		if (m_Font) {
			LineHeight = m_Font->GetLineSize()*FScale;
			MaxVerticalScroll = std::max<float>(0.0f, (m_LineCount*LineHeight + (LineHeight*0.25f)) - VisibleSize.y);
		}
		if(isOver && isLDown && !SelectPressed){
			LWVector2f RelPos = LWVector2f(DownPnt.x - VisiblePos.x, VisibleSize.y - (DownPnt.y - VisiblePos.y)) + LWVector2f(m_HorizontalScroll, m_VerticalScroll);
			uint32_t Line = std::min<uint32_t>((uint32_t)((float)RelPos.y / LineHeight), m_LineCount - 1);
			LWUTF8GraphemeIterator CLine = m_Lines[Line].GetValueGrapheme(isPassword);

			uint32_t Pos = m_Font->CharacterAt(CLine, RelPos.x, FScale);
			LWUTF8Iterator CPos = m_Lines[Line].GetValueGrapheme(false) + Pos;
			if (!wasLDown) m_SelectCursor = LWETextInputCursor(Line, CPos);
			m_Cursor = LWETextInputCursor(Line, CPos);
			int32_t RawDis = 0;
			m_SelectCursorLength = GetCursorDistance(m_SelectCursor, m_Cursor, RawDis);
			Wnd->SetKeyboardText(CLine);
			Wnd->SetKeyboardEditRange(Pos, m_SelectCursorLength);
			Changed = true;
		}
		if (Mouse) {
			m_VerticalScroll -= (float)Mouse->GetScroll()*0.25f;
			m_VerticalScroll = std::max<float>(std::min<float>(m_VerticalScroll, MaxVerticalScroll), 0.0f);
		}
		if(Changed){
			ScrollToCursor(Scale);
			Manager.DispatchEvent(this, Event_Changed);
		}
	} else m_Flag &= ~(TouchEnabled|SelectEnabled);
	
	
	return *this;
}

LWEUI &LWEUITextInput::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	Frame.WriteRect(m_BorderMaterial, VisiblePos - LWVector2f(m_BorderSize), VisibleSize + LWVector2f(m_BorderSize*2.0f));
	if (!Frame.WriteRect(m_TextAreaMaterial, VisiblePos, VisibleSize)) return *this;
	if (!m_Font) return *this;
	if (!m_FontMaterial) return *this;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	float BtnLineSize = m_Font->GetLineSize()*m_FontBtnScale*Scale;
	DrawTouchButtons(Manager, Frame, Scale, lCurrentTime);
	bool isPassField = isPasswordField();
	bool isFocused = Manager.GetFocusedUI() == this;

	LWETextInputCursor SBegin, SEnd;
	int32_t SelectDis = MakeSelectionRange(m_SelectCursor, SBegin, SEnd, m_SelectCursorLength);
	if (m_CurrentLength == 0 && !isFocused) {
		if (!*m_DefaultText) return *this;
		LWVector4f TextBounds = m_Font->MeasureText(m_DefaultText, Scale*m_DefaultFontScale);
		LWVector2f TextSize = LWVector2f(TextBounds.z - TextBounds.x, TextBounds.y - TextBounds.w);
		float UnderHang = TextBounds.w;
		m_Font->DrawTextm(m_DefaultText, VisiblePos + VisibleSize * 0.5f - TextSize * 0.5f - LWVector2f(0.0f, TextBounds.w), Scale*m_DefaultFontScale, m_DefaultMaterial ? m_DefaultMaterial->m_ColorA : LWVector4f(1.0f), &Frame, &LWEUIFrame::WriteFontGlyph);
	} else {
		for (uint32_t i = 0; i < m_LineCount; i++) {
			LWUTF8GraphemeIterator Line = m_Lines[i].GetValueGrapheme(isPassField);
			m_Font->DrawClippedTextm(Line, VisiblePos + LWVector2f(0.0f, VisibleSize.y - LineSize * (i + 1)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), m_FontScale*Scale, m_FontMaterial->m_ColorA, LWVector4f(VisiblePos, VisibleSize), &Frame, &LWEUIFrame::WriteFontGlyph);
		}
		if (SelectDis) {
			LWUTF8GraphemeIterator GBegin = SBegin.AsGrapheme(m_Lines[SBegin.m_Line].GetValueGrapheme(isPassField));
			LWUTF8GraphemeIterator GEnd = SEnd.AsGrapheme(m_Lines[SEnd.m_Line].GetValueGrapheme(isPassField));
			float SpaceWidth = 0.0f; //Added to end of a selection line.
			LWGlyph *WGlyph = m_Font->GetGlyph(' ');
			if (WGlyph) SpaceWidth = WGlyph->m_Advance.x * m_FontScale * Scale;
			for (uint32_t i = SBegin.m_Line; i <= SEnd.m_Line; i++) {
				LWUTF8GraphemeIterator Line = m_Lines[i].GetValueGrapheme(isPassField);
				LWUTF8GraphemeIterator LineEnd = m_Lines[i].GetValueGraphemeLast(isPassField);
				LWVector4f BeginPos;
				LWVector4f EndPos;
				if (i == SBegin.m_Line) {
					BeginPos = m_Font->MeasureText(LWUTF8GraphemeIterator(Line, GBegin), m_FontScale*Scale);
					LWUTF8GraphemeIterator e = (SBegin.m_Line == SEnd.m_Line) ? GEnd : LineEnd;
					EndPos = m_Font->MeasureText(LWUTF8GraphemeIterator(Line, e), m_FontScale*Scale);
				} else if (i == SEnd.m_Line) {
					EndPos = m_Font->MeasureText(LWUTF8GraphemeIterator(Line, GEnd), m_FontScale*Scale);
				} else {
					EndPos = m_Font->MeasureText(Line, m_FontScale*Scale);
				}
				if (i==SEnd.m_Line) SpaceWidth = 0.0f;
				Frame.WriteClippedRect(m_SelectMaterial, VisiblePos + LWVector2f(BeginPos.z, VisibleSize.y - (LineSize*(i + 1) + LineSize * 0.25f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), LWVector2f(EndPos.z - BeginPos.z + SpaceWidth, LineSize), LWVector4f(VisiblePos, VisibleSize));
			}
		}
		if (isFocused) {
			uint64_t HalfSeconds = lCurrentTime / (LWTimer::GetResolution() / 2);
			if ((HalfSeconds % 3) >= 1) {
				LWUTF8GraphemeIterator Line = m_Lines[m_Cursor.m_Line].GetValueGrapheme(isPassField);
				LWUTF8GraphemeIterator CPos = m_Cursor.AsGrapheme(Line);

				LWVector4f LineSizeToCursor = m_Font->MeasureText(LWUTF8GraphemeIterator(Line, CPos), m_FontScale*Scale);
				LWVector2f CursorCtr = VisiblePos + LWVector2f(LineSizeToCursor.z, VisibleSize.y - (LineSize*m_Cursor.m_Line + LineSize * 0.75f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll);
				Frame.WriteClippedRect(m_CursorMaterial, CursorCtr - LWVector2f(m_CursorSize, LineSize)*0.5f, LWVector2f(m_CursorSize*2.0f, LineSize), LWVector4f(VisiblePos, VisibleSize));
			}
		}
	}
	return *this;
}

void LWEUITextInput::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

bool LWEUITextInput::ProcessKey(LWKey Key, bool ShiftDown, bool CtrlDown, float Scale) {
	char8_t SelectBuffer[1024];

	if (Key == LWKey::Return) {
		if (m_SelectCursorLength) ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
		if (m_LineCount >= m_MaxLines || (m_Cursor.m_Line + 1) >= LWETEXTINPUT_MAXLINES || m_LineCount >= LWETEXTINPUT_MAXLINES) return false;
		//shift all lines down by 1.
		std::copy_backward(m_Lines + (m_Cursor.m_Line + 1), m_Lines + m_LineCount, m_Lines + (m_LineCount + 1));

		LWETextLine &CurrLine = m_Lines[m_Cursor.m_Line];
		LWETextLine &NewLine = m_Lines[m_Cursor.m_Line + 1];
		NewLine.Clear();
		m_CurrentLength += NewLine.Insert(NewLine.GetValue(), m_Cursor.m_Position);
		m_CurrentLength -= CurrLine.Erase(m_Cursor.m_Position, CurrLine.GetValueLast());
		m_Cursor = LWETextInputCursor(m_Cursor.m_Line + 1, NewLine.GetValue());
		m_LineCount++;
		m_SelectCursorLength = 0;
		return true;
	} else if (Key == LWKey::Delete) {
		if (!m_SelectCursorLength) {
			if (m_Cursor.m_Position.AtEnd() && m_Cursor.m_Line + 1 >= m_LineCount) return false;
			m_SelectCursor = m_Cursor;
			m_SelectCursorLength = 1;
		}

		LWETextInputCursor SBegin, SEnd;
		MakeSelectionRange(m_SelectCursor, SBegin, SEnd, m_SelectCursorLength);
		LWETextLine &BeginLine = m_Lines[SBegin.m_Line];
		LWETextLine &EndLine = m_Lines[SEnd.m_Line];
		if (SBegin.m_Line == SEnd.m_Line) {
			m_CurrentLength -= BeginLine.Erase(SBegin.m_Position, SEnd.m_Position);
		} else {
			m_CurrentLength -= BeginLine.Erase(SBegin.m_Position, BeginLine.GetValueLast());
			BeginLine.Insert(SBegin.m_Position, SEnd.m_Position);
			m_CurrentLength -= EndLine.Erase(EndLine.GetValue(), SEnd.m_Position);
			for (uint32_t i = SBegin.m_Line + 1; i < SEnd.m_Line; i++) m_CurrentLength -= m_Lines[i].m_Length;
			std::copy(m_Lines + SEnd.m_Line + 1, m_Lines + m_LineCount, m_Lines + SBegin.m_Line + 1);
			m_LineCount -= (SEnd.m_Line - SBegin.m_Line);
		}
		m_Cursor = LWETextInputCursor(SBegin.m_Line, BeginLine.UpdateIterator(SBegin.m_Position));
		m_SelectCursorLength = 0;
		return true;
	} else if (Key == LWKey::Back) {
		bool Res = m_SelectCursorLength!=0;
		if (!Res) Res = MoveCursorLeft(m_Cursor);
		if (Res) Res = ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
		return Res;
	} else if (Key == LWKey::Left || Key == LWKey::Right || Key == LWKey::Up || Key == LWKey::Down) {
		bool Res = false;
		LWETextInputCursor pCursor = m_Cursor;
		if (Key == LWKey::Left) Res = MoveCursorLeft(m_Cursor);
		else if (Key == LWKey::Right) Res = MoveCursorRight(m_Cursor);
		else if (Key == LWKey::Up) Res = MoveCursorUp(m_Cursor, Scale);
		else if (Key == LWKey::Down) Res = MoveCursorDown(m_Cursor, Scale);
		if (Res) {
			if (ShiftDown) {
				if (!m_SelectCursorLength) m_SelectCursor = pCursor;
				int32_t RawDis = 0;
				int32_t Dis = GetCursorDistance(m_SelectCursor, m_Cursor, RawDis);
				m_SelectCursorLength = Dis;
				if(Dis) m_Flag |= SelectEnabled;
			} else m_SelectCursorLength = 0;
		}
		return Res;
	} else if (CtrlDown) {
		if (Key == LWKey::A) {
			m_SelectCursor = LWETextInputCursor(0, m_Lines[0].GetValue());
			m_SelectCursorLength = m_CurrentLength + m_LineCount;
			m_Cursor = LWETextInputCursor(m_LineCount - 1, m_Lines[m_LineCount-1].GetValueLast());
			m_Flag |= SelectEnabled;
			return true;
		} else if (Key == LWKey::C) {
			if (m_SelectCursorLength == 0) return false;
			GetSelectedText(SelectBuffer, sizeof(SelectBuffer), isPasswordField());
			LWWindow::WriteClipboardText(SelectBuffer);
			return true;
		} else if (Key == LWKey::X) {
			if (m_SelectCursorLength == 0) return false;
			
			GetSelectedText(SelectBuffer, sizeof(SelectBuffer), isPasswordField());
			LWWindow::WriteClipboardText(SelectBuffer);
			ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
			return true;
		} else if (Key == LWKey::V) {
			bool Res = false;
			if (m_SelectCursorLength) Res = ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
			uint32_t Len = LWWindow::ReadClipboardText(SelectBuffer, sizeof(SelectBuffer));
			InsertText(SelectBuffer, false, false, Scale);
			return true;
		} else if (Key == LWKey::N) { //special key for touch presses!
			if(isPasswordField()){
				m_SelectCursor = LWETextInputCursor(m_Cursor.m_Line, m_Lines[0].GetValue());
				m_Cursor = LWETextInputCursor(m_Cursor.m_Line, m_Lines[m_Cursor.m_Line].GetValueLast());
				m_SelectCursorLength = (m_Cursor.m_Position.m_Index - m_SelectCursor.m_Position.m_Index);
				m_Flag |= SelectEnabled;
				return true;
			}
			LWUTF8Iterator F = m_Cursor.m_Position.NextWord(true);
			LWUTF8Iterator N = F.NextToken(' '); //Find end of current word.
			m_SelectCursor = LWETextInputCursor(m_Cursor.m_Line, F);
			m_Cursor = LWETextInputCursor(m_Cursor.m_Line, N);
			m_SelectCursorLength = m_Cursor.m_Position.m_Index - m_SelectCursor.m_Position.m_Index;
			m_Flag |= SelectEnabled;
			return true;
		}
	}
	return false;
}

LWEUITextInput &LWEUITextInput::CalculateTouchBtnPositions(float Scale) {
	auto UpdatePos = [](LWETextInputTouchBtn &Btn, const LWVector2f &Pos, float Scale, bool isVisible)->LWVector2f {
		if (isVisible) {
			Btn.m_Position = Pos;
			return Pos + (Btn.m_Size.x + 10.0f)*Scale;
		}
		Btn.m_Position = LWVector2f(-1.0f); //Set's off screen.
		return Pos;
	};
	char8_t Buffer = '\0';
	LWWindow::ReadClipboardText(&Buffer, sizeof(Buffer));
	LWVector2f Pos = m_VisiblePosition + LWVector2f(10.0f, m_VisibleSize.y + 10.0f);
	Pos = UpdatePos(m_CopyBtn, Pos, Scale, m_SelectCursorLength > 0);
	Pos = UpdatePos(m_CutBtn, Pos, Scale, m_SelectCursorLength > 0);
	Pos = UpdatePos(m_PasteBtn, Pos, Scale, Buffer != '\0');
	Pos = UpdatePos(m_SelectAllBtn, Pos, Scale, m_CurrentLength > 0);
	return *this;
}


int32_t LWEUITextInput::GetCursorDistance(const LWETextInputCursor &ACursor, const LWETextInputCursor &BCursor, int32_t &RawDistance) {
	int32_t Distance = 0;
	RawDistance = 0;
	LWETextInputCursor A = ACursor;
	LWETextInputCursor B = BCursor;
	while (B.m_Line > A.m_Line) {
		Distance += B.m_Position.m_Index+1; //Add newline.
		RawDistance += B.m_Position.RawIndex();
		B.m_Line--;
		B.m_Position = m_Lines[B.m_Line].GetValueLast();
	}
	while (A.m_Line > B.m_Line) {
		Distance -= A.m_Position.m_Index+1; //Add newline.
		RawDistance -= A.m_Position.RawIndex();
		A.m_Line--;
		A.m_Position = m_Lines[A.m_Line].GetValueLast();
	}
	Distance += B.m_Position.m_Index - A.m_Position.m_Index;
	RawDistance += B.m_Position.RawIndex() - A.m_Position.RawIndex();
	return Distance;
}

int32_t LWEUITextInput::MakeSelectionRange(const LWETextInputCursor &Cursor, LWETextInputCursor &RangeBegin, LWETextInputCursor &RangeEnd, int32_t Distance) {
	RangeBegin = RangeEnd = Cursor;
	if (Distance < 0) return MoveCursorBy(RangeBegin, Distance);
	return MoveCursorBy(RangeEnd, Distance);
}

int32_t LWEUITextInput::MoveCursorBy(LWETextInputCursor &Cursor, int32_t Distance) {
	int32_t D = 0;
	for (; Distance < 0 && MoveCursorLeft(Cursor); Distance++, D--) {}
	for (; Distance > 0 && MoveCursorRight(Cursor); Distance--, D++) {}
	return D;
}

bool LWEUITextInput::MoveCursorUp(LWETextInputCursor &Cursor, float UIScale) {
	if (Cursor.m_Line == 0) {
		if (!Cursor.m_Position.AtStart()) {
			Cursor = LWETextInputCursor(0, m_Lines[0].GetValue());
			return true;
		}
		return false;
	}
	if (!m_Font) Cursor.m_Position = m_Lines[Cursor.m_Line - 1].GetValue().Next(Cursor.m_Position.m_Index);
	else{
		bool isPassword = isPasswordField();
		LWUTF8GraphemeIterator C = m_Lines[Cursor.m_Line].GetValueGrapheme(isPassword);
		LWUTF8GraphemeIterator P = m_Lines[Cursor.m_Line - 1].GetValueGrapheme(isPassword);
		LWUTF8GraphemeIterator CP = Cursor.AsGrapheme(C);
		LWVector4f CSize = m_Font->MeasureText(LWUTF8GraphemeIterator(C, CP), m_FontScale * UIScale);
		uint32_t Idx = m_Font->CharacterAt(P, CSize.z, m_FontScale * UIScale);
		Cursor.m_Position = m_Lines[Cursor.m_Line - 1].GetValueGrapheme(false).Next(Idx);
	}
	Cursor.m_Line--;
	return true;
}

bool LWEUITextInput::MoveCursorDown(LWETextInputCursor &Cursor, float UIScale) {
	if (Cursor.m_Line + 1 >= m_LineCount) {
		if(!Cursor.m_Position.AtEnd()) {
			Cursor = LWETextInputCursor(Cursor.m_Line, m_Lines[Cursor.m_Line].GetValueLast());
			return true;
		}
		return false;
	}
	if (!m_Font) Cursor.m_Position = m_Lines[Cursor.m_Line + 1].GetValue().Next(Cursor.m_Position.m_Index);
	else {
		bool isPassword = isPasswordField();
		LWUTF8GraphemeIterator C = m_Lines[Cursor.m_Line].GetValueGrapheme(isPassword);
		LWUTF8GraphemeIterator P = m_Lines[Cursor.m_Line + 1].GetValueGrapheme(isPassword);
		LWUTF8GraphemeIterator CP = Cursor.AsGrapheme(C);
		LWVector4f CSize = m_Font->MeasureText(LWUTF8GraphemeIterator(C, CP), m_FontScale * UIScale);
		uint32_t Idx = m_Font->CharacterAt(P, CSize.z, m_FontScale * UIScale);
		Cursor.m_Position = m_Lines[Cursor.m_Line + 1].GetValueGrapheme(false).Next(Idx);
	}
	Cursor.m_Line++;
	return true;
}

bool LWEUITextInput::MoveCursorLeft(LWETextInputCursor &Cursor) {
	if(Cursor.m_Position.AtStart()) {
		if (Cursor.m_Line == 0) return false;
		Cursor = LWETextInputCursor(Cursor.m_Line - 1, m_Lines[Cursor.m_Line - 1].GetValueLast());
		return true;
	}
	--Cursor.m_Position;
	return true;
}

bool LWEUITextInput::MoveCursorRight(LWETextInputCursor &Cursor) {
	if(Cursor.m_Position.AtEnd()) {
		if (Cursor.m_Line + 1 >= m_LineCount) return false;
		Cursor = LWETextInputCursor(Cursor.m_Line + 1, m_Lines[Cursor.m_Line+1].GetValue());
		return true;
	}
	++Cursor.m_Position;
	return true;
}


uint32_t LWEUITextInput::InsertText(const LWUTF8Iterator &Text, bool ShiftDown, bool CtrlDown, float Scale) {
	uint32_t Cnt = 0;
	for (LWUTF8Iterator C = Text; !C.AtEnd(); ++C) {
		if (InsertChar(*C, ShiftDown, CtrlDown, Scale)) Cnt++;
	}
	return Cnt;
}

bool LWEUITextInput::InsertChar(uint32_t Char, bool ShiftDown, bool CtrlDown, float Scale) {
	if (Char == '\n') return ProcessKey(LWKey::Return, ShiftDown, CtrlDown, Scale);
	else if (Char < ' ') return false;
	bool Allowed = true;
	if (*m_AllowedCharacters) {
		Allowed = (m_Flag&InvertAllowed) != 0;
		for (LWUTF8Iterator C = m_AllowedCharacters; !C.AtEnd(); ++C) {
			if (*C == Char) {
				Allowed = (m_Flag & InvertAllowed) == 0;
				break;
			}
		}
	}
	if (!Allowed) return false;
	if (m_SelectCursorLength) ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
	if (m_CurrentLength >= m_MaxLength) return false;
	uint32_t r = m_Lines[m_Cursor.m_Line].Insert(m_Cursor.m_Position, Char);
	if (!r) return false;
	m_CurrentLength += r;
	m_Cursor.m_Position = m_Lines[m_Cursor.m_Line].UpdateIterator(m_Cursor.m_Position);
	++m_Cursor.m_Position;
	return true;
}

bool LWEUITextInput::ScrollToCursor(float Scale) {
	if (!m_Font) return false;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	LWUTF8GraphemeIterator GLine = m_Lines[m_Cursor.m_Line].GetValueGrapheme(isPasswordField());
	LWUTF8GraphemeIterator CPos = m_Cursor.AsGrapheme(GLine);

	LWVector4f LineSizeToCursor = m_Font->MeasureText(LWUTF8GraphemeIterator(GLine, CPos), m_FontScale*Scale);
	LWVector2f CursorPos = LWVector2f(LineSizeToCursor.z, m_VisibleSize.y - (LineSize*m_Cursor.m_Line + LineSize*0.75f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll);
	m_LongestLine = std::max<float>(LineSizeToCursor.z, m_LongestLine);
	if (CursorPos.x >= m_VisibleSize.x*0.9f) m_HorizontalScroll += (CursorPos.x - m_VisibleSize.x*0.9f);
	else if (CursorPos.x < m_VisibleSize.x*0.2f) m_HorizontalScroll += (CursorPos.x - m_VisibleSize.x*0.2f);
	m_HorizontalScroll = std::max<float>(0.0f, m_HorizontalScroll);

	if (CursorPos.y >= m_VisibleSize.y - LineSize) m_VerticalScroll += (CursorPos.y - m_VisibleSize.y - LineSize);
	else if (CursorPos.y < LineSize*2.0f) m_VerticalScroll -= (CursorPos.y - LineSize*2.0f);
	float MaxVerticalScroll = std::max<float>(0.0f, (m_LineCount*m_Font->GetLineSize() + (m_Font->GetLineSize()*0.25f)) - m_VisibleSize.y);
	m_VerticalScroll = std::max<float>(std::min<float>(m_VerticalScroll, MaxVerticalScroll), 0.0f);
	return true;
}

bool LWEUITextInput::SetScroll(float HoriScroll, float VertScroll, float Scale) {
	float MaxHoriScroll = GetHorizontalMaxScroll();
	float MaxVerticalScroll = GetVerticalMaxScroll(Scale);
	m_HorizontalScroll = std::max<float>(std::min<float>(HoriScroll, MaxHoriScroll - m_VisibleSize.x), 0.0f);
	m_VerticalScroll = std::max<float>(std::min<float>(VertScroll, MaxVerticalScroll - m_VisibleSize.y), 0.0f);
	return true;
}

LWETextInputCursor LWEUITextInput::MakeCursorAt(uint32_t Line, uint32_t Index) {
	Line = std::min<uint32_t>(Line, m_LineCount - 1);
	return LWETextInputCursor(Line, m_Lines[Line].GetValue().Next(Index));
}

LWEUITextInput &LWEUITextInput::Clear(void) {
	m_Lines[0].Clear();
	m_SelectCursor = LWETextInputCursor(0, m_Lines[0].GetValue());
	m_Cursor = LWETextInputCursor(0, m_Lines[0].GetValue());
	m_SelectCursorLength = 0;
	m_LineCount = 1;
	m_CurrentLength = 0;
	m_VerticalScroll = m_HorizontalScroll = 0.0f;
	m_LongestLine = 0.0f;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetFontScale(float FontScale) {
	m_FontScale = FontScale;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetAllowedCharacters(const LWUTF8Iterator &Characters) {
	Characters.Copy(m_AllowedCharacters, sizeof(m_AllowedCharacters));
	return *this;
}

LWEUITextInput &LWEUITextInput::SetDefaultText(const LWUTF8Iterator &Text) {
	Text.Copy(m_DefaultText, sizeof(m_DefaultText));
	return *this;
}

LWEUITextInput &LWEUITextInput::SetDefaultFontScale(float FontScale) {
	m_DefaultFontScale = FontScale;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBorderSize(float BorderSize) {
	m_BorderSize = BorderSize;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetCursorSize(float CursorSize) {
	m_CursorSize = CursorSize;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetMaxLength(uint32_t MaxLength) {
	m_MaxLength = MaxLength;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetMaxLines(uint32_t MaxLines) {
	m_MaxLines = MaxLines;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetCursorPosition(uint32_t CursorPosition, uint32_t CursorLine) {
	CursorLine = std::min<uint32_t>(m_Cursor.m_Line, m_LineCount - 1);
	m_Cursor = LWETextInputCursor(CursorLine, m_Lines[CursorLine].GetValue().Next(CursorPosition));
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBorderMaterial(LWEUIMaterial *BorderMaterial) {
	m_BorderMaterial = BorderMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetFontMaterial(LWEUIMaterial *FontMaterial) {
	m_FontMaterial = FontMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetDefaultMaterial(LWEUIMaterial *DefMaterial) {
	m_DefaultMaterial = DefMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetSelectMaterial(LWEUIMaterial *SelectMaterial) {
	m_SelectMaterial = SelectMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetTextAreaMaterial(LWEUIMaterial *TextAreaMaterial) {
	m_TextAreaMaterial = TextAreaMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetCursorMaterial(LWEUIMaterial *CursorMaterial) {
	m_CursorMaterial = CursorMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBtnDownMaterial(LWEUIMaterial *BtnDownMaterial) {
	m_BtnDownMaterial = BtnDownMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBtnOverMaterial(LWEUIMaterial *BtnOverMaterial) {
	m_BtnOverMaterial = BtnOverMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBtnOffMaterial(LWEUIMaterial *BtnOffMaterial) {
	m_BtnOffMaterial = BtnOffMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBtnFontMaterial(LWEUIMaterial *BtnFontMaterial) {
	m_BtnFontMaterial = BtnFontMaterial;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetBtnFontScale(float FontScale) {
	m_FontBtnScale = FontScale;
	return SetFont(m_Font);
}

LWEUITextInput &LWEUITextInput::SetFont(LWFont *Font) {
	m_Font = Font;
	if (m_Font) {
		float LineSize = Font->GetLineSize()*m_FontScale;
		LWVector4f CopyBtn = m_Font->MeasureText("Copy", m_FontBtnScale);
		LWVector4f CutBtn = m_Font->MeasureText("Cut", m_FontBtnScale);
		LWVector4f PasteBtn = m_Font->MeasureText("Paste", m_FontBtnScale);
		LWVector4f SelectAllBtn = m_Font->MeasureText("Select All", m_FontBtnScale);
		m_CopyBtn.m_Size = LWVector2f(CopyBtn.z - CopyBtn.x, -CopyBtn.w);
		m_CutBtn.m_Size = LWVector2f(CutBtn.z - CutBtn.x, -CutBtn.w);
		m_PasteBtn.m_Size = LWVector2f(PasteBtn.z - PasteBtn.x, -PasteBtn.w);
		m_SelectAllBtn.m_Size = LWVector2f(SelectAllBtn.z - SelectAllBtn.x, -SelectAllBtn.w);
	}
	return *this;
}

LWEUITextInput &LWEUITextInput::SetSelectRange(uint32_t Position, uint32_t Line, int32_t Length) {
	Line = std::min<uint32_t>(Line, m_LineCount - 1);
	m_SelectCursor = LWETextInputCursor(Line, m_Lines[Line].GetValue().Next(Position));
	m_Cursor = m_SelectCursor;
	uint32_t SelectLen = 0;
	for (; Length < 0 && MoveCursorLeft(m_SelectCursor); Length++, SelectLen--) {}
	for (; Length > 0 && MoveCursorRight(m_SelectCursor); Length--, SelectLen++) {}
	m_SelectCursorLength = SelectLen;
	m_Flag = (m_Flag&~SelectEnabled) | (m_SelectCursorLength != 0 ? SelectEnabled : 0);
	return *this;
}

uint32_t LWEUITextInput::GetSelectedText(char8_t *Buffer, uint32_t BufferLen, bool PasswordField) {
	return GetTextRange(m_SelectCursor, m_SelectCursorLength, Buffer, BufferLen, PasswordField);
}

uint32_t LWEUITextInput::GetTextRange(const LWETextInputCursor &Cursor, int32_t Count, char8_t *Buffer, uint32_t BufferLen, bool PasswordField) {
	LWETextInputCursor Begin, End;
	if (!MakeSelectionRange(Cursor, Begin, End, Count)) {
		if (BufferLen) *Buffer = '\0';
		return 1;
	}
	char8_t *B = Buffer;
	char8_t *BL = Buffer + std::min<uint32_t>(BufferLen - 1, BufferLen);
	uint32_t o = 0;
	LWUTF8Iterator E = End.AsIterator(m_Lines[End.m_Line].GetValue(PasswordField));
	LWUTF8Iterator P = Begin.AsIterator(m_Lines[Begin.m_Line].GetValue(PasswordField));
	for (; P != E;) {
		if (P.AtEnd()) {
			P = m_Lines[++Begin.m_Line].GetValue(PasswordField);
			if (B != BL) *B++ = '\n';
			o++;
		} else {
			uint32_t r = LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), *P);
			B = std::min<char8_t*>(B + r, BL);
			o += r;
			++P;
		}
	}
	if (BufferLen) *B = '\0';
	return o + 1;
}

const LWETextLine *LWEUITextInput::GetLine(uint32_t Line) const {
	return m_Lines + Line;
}

LWEUIMaterial *LWEUITextInput::GetBorderMaterial(void) const {
	return m_BorderMaterial;
}

LWEUIMaterial *LWEUITextInput::GetFontMaterial(void) const {
	return m_FontMaterial;
}

LWEUIMaterial *LWEUITextInput::GetSelectMaterial(void) const {
	return m_SelectMaterial;
}

LWEUIMaterial *LWEUITextInput::GetDefaultMaterial(void) const {
	return m_DefaultMaterial;
}

LWEUIMaterial *LWEUITextInput::GetTextAreaMaterial(void) const {
	return m_TextAreaMaterial;
}

LWEUIMaterial *LWEUITextInput::GetCursorMaterial(void) const {
	return m_CursorMaterial;
}

LWFont *LWEUITextInput::GetFont(void) const {
	return m_Font;
}

float LWEUITextInput::GetBorderSize(void) const {
	return m_BorderSize;
}

float LWEUITextInput::GetCursorSize(void) const {
	return m_CursorSize;
}

uint32_t LWEUITextInput::GetMaxLength(void) const {
	return m_MaxLength;
}

uint32_t LWEUITextInput::GetMaxLines(void) const {
	return m_MaxLines;
}

uint32_t LWEUITextInput::GetCurrentLength(void) const {
	return m_CurrentLength;
}

const LWETextInputCursor &LWEUITextInput::GetCursor(void) const {
	return m_Cursor;
}

const LWETextInputCursor &LWEUITextInput::GetSelectCursor(void) const {
	return m_SelectCursor;
}

LWETextInputCursor &LWEUITextInput::GetCursor(void) {
	return m_Cursor;
}

LWETextInputCursor &LWEUITextInput::GetSelectCursor(void) {
	return m_SelectCursor;
}

uint32_t LWEUITextInput::GetLineCount(void) const {
	return m_LineCount;
}

int32_t LWEUITextInput::GetSelectCursorLength(void) const {
	return m_SelectCursorLength;
}

float LWEUITextInput::GetVerticalScroll(void) const {
	return m_VerticalScroll;
}

float LWEUITextInput::GetHorizontalScroll(void) const {
	return m_HorizontalScroll;
}

float LWEUITextInput::GetHorizontalMaxScroll() const {
	return m_LongestLine;
}

float LWEUITextInput::GetVerticalMaxScroll(float Scale) const {
	if (!m_Font) return 0.0f;
	float LineHeight = m_Font->GetLineSize() * m_FontScale * Scale;
	return m_LineCount * LineHeight + (LineHeight * 0.25f);
}

float LWEUITextInput::GetFontScale(void) const {
	return m_FontScale;
}

float LWEUITextInput::GetDefaultFontScale(void) const {
	return m_DefaultFontScale;
}

bool LWEUITextInput::isPasswordField(void) const {
	return (m_Flag&PasswordField) != 0;
}

LWUTF8Iterator LWEUITextInput::GetAllowedCharacters(void) const {
	return m_AllowedCharacters;
}

LWUTF8Iterator LWEUITextInput::GetDefaultText(void) const {
	return m_DefaultText;
}

LWEUITextInput::LWEUITextInput(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag) {
	*m_DefaultText = '\0';
	*m_AllowedCharacters = '\0';
	Clear();
}

LWEUITextInput::~LWEUITextInput() {}
