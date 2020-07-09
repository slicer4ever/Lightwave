#include <LWPlatform/LWWindow.h>
#include <LWVideo/LWFont.h>
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWWindow.h>
#include "LWEUI/LWEUITextInput.h"
#include "LWELocalization.h"
#include "LWEAsset.h"
#include <cstdarg>
#include <algorithm>
#include <iostream>

LWETextInputCursor::LWETextInputCursor(uint32_t Line, uint32_t Position, uint32_t RawPosition) : m_Line(Line), m_Position(Position), m_RawPosition(RawPosition) {}

LWETextInputCursor::LWETextInputCursor(uint32_t Line, const char *LineBegin, const char *LinePosition) : m_Position(0), m_RawPosition(0) {
	const char *L = LineBegin;
	while (L && L < LinePosition) {
		m_Position++;
		L = LWText::Next(L);
	}
	if (!L) L = LinePosition;
	m_RawPosition = (uint32_t)(uintptr_t)(L - LineBegin);
}

LWEUITextInput *LWEUITextInput::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUITextInput *TextInput = Allocator->Allocate<LWEUITextInput>(LWVector4f(0.0f), LWVector4f(0.0f), FocusAble | TabAble);
	LWEUI::XMLParse(TextInput, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWXMLAttribute *BorderMaterialAttr = FindAttribute(Node, Style, "BorderMaterial");
	LWXMLAttribute *FontMaterialAttr = FindAttribute(Node, Style, "FontMaterial");
	LWXMLAttribute *SelectMaterialAttr = FindAttribute(Node, Style, "SelectMaterial");
	LWXMLAttribute *DefaultMaterialAttr = FindAttribute(Node, Style, "DefaultMaterial");
	LWXMLAttribute *TextAreaMaterialAttr = FindAttribute(Node, Style, "TextAreaMaterial");
	LWXMLAttribute *CursorMaterialAttr = FindAttribute(Node, Style, "CursorMaterial");
	LWXMLAttribute *AllowedCharactersAttr = FindAttribute(Node, Style, "AllowedCharacters");
	LWXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWXMLAttribute *ValueAttr = FindAttribute(Node, Style, "Value");
	LWXMLAttribute *DefaultAttr = FindAttribute(Node, Style, "Default");
	LWXMLAttribute *BorderSizeAttr = FindAttribute(Node, Style, "BorderSize");
	LWXMLAttribute *CursorSizeAttr = FindAttribute(Node, Style, "CursorSize");
	LWXMLAttribute *MaxLengthAttr = FindAttribute(Node, Style, "MaxLength");
	LWXMLAttribute *MaxLinesAttr = FindAttribute(Node, Style, "MaxLines");
	LWXMLAttribute *FontScaleAttr = FindAttribute(Node, Style, "FontScale");
	LWXMLAttribute *DefaultFontScaleAttr = FindAttribute(Node, Style, "DefaultFontScale");
	LWXMLAttribute *BtnOffAttr = FindAttribute(Node, Style, "OffMaterial");
	LWXMLAttribute *BtnOverAttr = FindAttribute(Node, Style, "OverMaterial");
	LWXMLAttribute *BtnDownAttr = FindAttribute(Node, Style, "DownMaterial");
	LWXMLAttribute *BtnFontScaleAttr = FindAttribute(Node, Style, "ButtonFontScale");
	LWXMLAttribute *BtnFontMaterialAttr = FindAttribute(Node, Style, "ButtonFontMaterial");

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
	if (BorderMaterialAttr) BorderMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BorderMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (FontMaterialAttr) FontMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), FontMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (DefaultMaterialAttr) DefaultMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnFontMaterialAttr) BtnFontMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (SelectMaterialAttr) SelectMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), SelectMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (TextAreaMaterialAttr) TextAreaMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), TextAreaMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (CursorMaterialAttr) CursorMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), CursorMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnOffAttr) BtnOffMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOffAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnDownAttr) BtnDownMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnDownAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnOverAttr) BtnOverMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOverAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (FontAttr) Font = Manager->GetAssetManager()->GetAsset<LWFont>(ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BorderSizeAttr) BorderSize = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), BorderSizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (CursorSizeAttr) CursorSize = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), CursorSizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (MaxLengthAttr) MaxLength = (uint32_t)atoi(ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLengthAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (MaxLinesAttr) MaxLines = (uint32_t)atoi(ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLinesAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (FontScaleAttr) {
		float Scale = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), FontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode));
		TextInput->SetFontScale(Scale).SetDefaultFontScale(Scale);
	}
	if (DefaultFontScaleAttr) {
		float Scale = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultFontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode));
		TextInput->SetDefaultFontScale(Scale);
	}
	if (BtnFontScaleAttr) TextInput->SetBtnFontScale((float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
	if (AllowedCharactersAttr) TextInput->SetAllowedCharacters(ParseComponentAttribute(Buffer, sizeof(Buffer), AllowedCharactersAttr->m_Value, ActiveComponent, ActiveComponentNode));
	TextInput->SetBorderMaterial(BorderMat).SetFontMaterial(FontMat).SetDefaultMaterial(DefaultMat).SetSelectMaterial(SelectMat).SetTextAreaMaterial(TextAreaMat).SetCursorMaterial(CursorMat).SetBtnOffMaterial(BtnOffMat).SetBtnOverMaterial(BtnOverMat).SetBtnDownMaterial(BtnDownMat).SetBtnFontMaterial(BtnFontMat);
	TextInput->SetFont(Font).SetBorderSize(BorderSize).SetCursorSize(CursorSize).SetMaxLength(MaxLength).SetMaxLines(MaxLines);
	if (DefaultAttr) {
		TextInput->SetDefaultText(ParseComponentAttribute(Buffer, sizeof(Buffer), DefaultAttr->m_Value, ActiveComponent, ActiveComponentNode));
	}
	if (ValueAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		TextInput->InsertText(Res, false, false, 1.0f);
	}
	return TextInput;
}

LWEUITextInput &LWEUITextInput::UpdateTouchButtons(LWEUIManager &Manager, float Scale, uint64_t lCurrentTime) {
	const LWVector2f DefaultPos = LWVector2f(-1000.0f, -1000.0f);
	auto UpdateButton = [this](const LWTouchPoint &TP, uint32_t TPIndex, LWEUIManager &Manager, LWETextInputTouchBtn &Btn, float Scale, uint64_t lCurrentTime, LWKey KeyPress, bool ShiftDown, bool CtrlDown)->void {
		LWVector2f DownPnt = LWVector2f(-1000.0f, -1000.0f);
		float BtnHeight = m_Font->GetLineSize()*m_FontScale;
		LWVector2f Size = LWVector2f(Btn.m_Size.x, BtnHeight)*Scale;
		bool isOver = PointInside(TP.m_Position.CastTo<float>(), TP.m_Size*Scale, Btn.m_Position, Size);
		Btn.m_Flag = isOver ? LWETextInputTouchBtn::ButtonOver : 0;
		if (TP.m_State == LWTouchPoint::UP) {
			if(isOver) ProcessKey(KeyPress, ShiftDown, CtrlDown, Scale);
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
	static char PasswordLine[LWETEXTINPUT_MAXLINELENGTH];
	static bool Initiated = false;
	if (!Initiated) {
		std::fill(PasswordLine, PasswordLine + LWETEXTINPUT_MAXLINELENGTH, '*');
		Initiated = true;
	}

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
			char *CLine = isPassword ? PasswordLine : m_Lines[Line].m_Value;

			uint32_t Pos = m_Font->CharacterAt(CLine, RelPos.x, m_Lines[Line].m_CharLength, FScale);
			char *RawPos = LWText::At(m_Lines[Line].m_Value, Pos);
			if (!RawPos) RawPos = m_Lines[Line].m_Value + m_Lines[Line].m_RawLength;
			uint32_t CRawPos = (uint32_t)(uintptr_t)(RawPos - m_Lines[Line].m_Value);
			if (!wasLDown) m_SelectCursor = LWETextInputCursor(Line, Pos, CRawPos);
			m_Cursor = LWETextInputCursor(Line, Pos, CRawPos);
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
	static char PasswordLine[LWETEXTINPUT_MAXLINELENGTH];
	static bool Initiated = false;
	if (!Initiated) {
		std::fill(PasswordLine, PasswordLine + LWETEXTINPUT_MAXLINELENGTH, '*');
		Initiated = true;
	}
	Frame.WriteRect(m_BorderMaterial, VisiblePos - LWVector2f(m_BorderSize), VisibleSize + LWVector2f(m_BorderSize*2.0f));
	if (!Frame.WriteRect(m_TextAreaMaterial, VisiblePos, VisibleSize)) return *this;
	if (!m_Font) return *this;
	if (!m_FontMaterial) return *this;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	float BtnLineSize = m_Font->GetLineSize()*m_FontBtnScale*Scale;
	DrawTouchButtons(Manager, Frame, Scale, lCurrentTime);
	bool isPassField = isPasswordField();
	bool isFocused = Manager.GetFocusedUI() == this;

	LWETextInputCursor SBegin = m_SelectCursor;
	LWETextInputCursor SEnd = m_SelectCursor;
	int32_t SelectDis = 0;
	if (m_SelectCursorLength < 0) SelectDis = MoveCursorBy(SBegin, m_SelectCursorLength);
	else SelectDis = MoveCursorBy(SEnd, m_SelectCursorLength);
	if (m_CurrentLength == 0 && !isFocused) {
		if (!*m_DefaultText) return *this;
		LWVector4f TextBounds = m_Font->MeasureText(m_DefaultText, Scale*m_DefaultFontScale);
		LWVector2f TextSize = LWVector2f(TextBounds.z - TextBounds.x, TextBounds.y - TextBounds.w);
		float UnderHang = TextBounds.w;
		m_Font->DrawTextm(m_DefaultText, VisiblePos + VisibleSize * 0.5f - TextSize * 0.5f - LWVector2f(0.0f, TextBounds.w), Scale*m_DefaultFontScale, m_DefaultMaterial ? m_DefaultMaterial->m_ColorA : LWVector4f(1.0f), &Frame, &LWEUIFrame::WriteFontGlyph);
	} else {
		for (uint32_t i = 0; i < m_LineCount; i++) {
			char *Line = isPassField ? PasswordLine : m_Lines[i].m_Value;
			m_Font->DrawClippedTextm(Line, m_Lines[i].m_CharLength, VisiblePos + LWVector2f(0.0f, VisibleSize.y - LineSize * (i + 1)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), m_FontScale*Scale, m_FontMaterial->m_ColorA, LWVector4f(VisiblePos, VisibleSize), &Frame, &LWEUIFrame::WriteFontGlyph);
		}
		if (SelectDis) {
			for (uint32_t i = SBegin.m_Line; i <= SEnd.m_Line; i++) {
				char *Line = isPassField ? PasswordLine : m_Lines[i].m_Value;
				uint32_t LineLen = m_Lines[i].m_CharLength;
				LWVector4f BeginPos;
				LWVector4f EndPos;
				if (i == SBegin.m_Line) {
					BeginPos = m_Font->MeasureText(Line, SBegin.m_Position, m_FontScale*Scale);
					uint32_t e = (SBegin.m_Line == SEnd.m_Line) ? SEnd.m_Position : m_Lines[i].m_CharLength;
					EndPos = m_Font->MeasureText(Line, e, m_FontScale*Scale);
				} else if (i == SEnd.m_Line) {
					EndPos = m_Font->MeasureText(Line, SEnd.m_Position, m_FontScale*Scale);
				} else {
					EndPos = m_Font->MeasureText(Line, LineLen, m_FontScale*Scale);
				}
				Frame.WriteClippedRect(m_SelectMaterial, VisiblePos + LWVector2f(BeginPos.z, VisibleSize.y - (LineSize*(i + 1) + LineSize * 0.25f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), LWVector2f(EndPos.z - BeginPos.z, LineSize), LWVector4f(VisiblePos, VisibleSize));
			}
		}
		if (isFocused) {
			uint64_t HalfSeconds = lCurrentTime / (LWTimer::GetResolution() / 2);
			if ((HalfSeconds % 3) >= 1) {
				char *Line = isPassField ? PasswordLine : m_Lines[m_Cursor.m_Line].m_Value;
				LWVector4f LineSizeToCursor = m_Font->MeasureText(Line, m_Cursor.m_Position, m_FontScale*Scale);
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
	
	char SelectBuffer[512];

	if (Key == LWKey::Return) {
		if (m_SelectCursorLength) ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
		if (m_LineCount >= m_MaxLines || (m_Cursor.m_Line + 1) >= LWETEXTINPUT_MAXLINES || m_LineCount >= LWETEXTINPUT_MAXLINES) return false;
		//shift all lines down by 1.
		std::copy_backward(m_Lines + (m_Cursor.m_Line + 1), m_Lines + m_LineCount, m_Lines + (m_LineCount + 1));

		uint32_t RawLen = m_Lines[m_Cursor.m_Line].m_RawLength;
		uint32_t RemLen = RawLen - m_Cursor.m_RawPosition;
		char *CurrentLine = m_Lines[m_Cursor.m_Line].m_Value;
		char *NextLine = m_Lines[m_Cursor.m_Line + 1].m_Value;
		std::copy(CurrentLine + m_Cursor.m_RawPosition, CurrentLine + RawLen+1, NextLine);
		CurrentLine[m_Cursor.m_RawPosition] = '\0';
		m_Lines[m_Cursor.m_Line + 1].m_RawLength = RemLen;
		m_Lines[m_Cursor.m_Line + 1].m_CharLength = m_Lines[m_Cursor.m_Line].m_CharLength - m_Cursor.m_Position;
		m_Lines[m_Cursor.m_Line].m_RawLength = m_Cursor.m_RawPosition;
		m_Lines[m_Cursor.m_Line].m_CharLength = m_Cursor.m_Position;
		m_Cursor = LWETextInputCursor(m_Cursor.m_Line + 1, 0u, 0u);
		m_LineCount++;
		return true;
	} else if (Key == LWKey::Delete) {
		if (!m_SelectCursorLength) {
			if (m_Cursor.m_Position == m_Lines[m_Cursor.m_Line].m_CharLength && m_Cursor.m_Line + 1 >= m_LineCount) return false;
			m_SelectCursor = m_Cursor;
			m_SelectCursorLength = 1;
		}
		LWETextInputCursor SBegin = m_SelectCursor;
		LWETextInputCursor SEnd = m_SelectCursor;
		if (m_SelectCursorLength < 0) MoveCursorBy(SBegin, m_SelectCursorLength);
		else MoveCursorBy(SEnd, m_SelectCursorLength);
		char *BeginLine = m_Lines[SBegin.m_Line].m_Value;
		char *EndLine = m_Lines[SEnd.m_Line].m_Value;
		uint32_t EndLineRemLen = m_Lines[SEnd.m_Line].m_RawLength - SEnd.m_RawPosition + 1;
		if (SBegin.m_RawPosition + EndLineRemLen >= LWETEXTINPUT_MAXLINELENGTH) {
			SetSelectRange(0, 0, 0);
			return false; //we do not have enough space to do this operation, so we abort!
		}
		uint32_t PrevLineCount = m_Lines[SBegin.m_Line].m_CharLength;
		std::copy(EndLine + SEnd.m_RawPosition, EndLine + m_Lines[SEnd.m_Line].m_RawLength+1, BeginLine + SBegin.m_RawPosition);
		m_Lines[SBegin.m_Line].m_CharLength = SBegin.m_Position + (m_Lines[SEnd.m_Line].m_CharLength - SEnd.m_Position);
		m_Lines[SBegin.m_Line].m_RawLength = SBegin.m_RawPosition + (m_Lines[SEnd.m_Line].m_RawLength - SEnd.m_RawPosition);
		uint32_t TotalLength = m_CurrentLength;
		if (SEnd.m_Line != SBegin.m_Line) {
			TotalLength -= (PrevLineCount - SBegin.m_Position) + SEnd.m_Position;
			for (uint32_t n = SBegin.m_Line + 1; n < SEnd.m_Line; n++) {
				TotalLength -= m_Lines[n].m_CharLength;
			}
			std::copy(m_Lines + (SEnd.m_Line + 1), m_Lines + m_LineCount, m_Lines + (SBegin.m_Line + 1));
			m_LineCount -= (SEnd.m_Line - SBegin.m_Line);
		} else TotalLength = TotalLength - (SEnd.m_Position - SBegin.m_Position);
		m_Cursor = SBegin;
		m_CurrentLength = TotalLength;
		SetSelectRange(0, 0, 0);
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
			m_SelectCursor = LWETextInputCursor(0, 0u, 0u);
			m_SelectCursorLength = m_CurrentLength + m_LineCount;
			m_Cursor = LWETextInputCursor(m_LineCount - 1, m_Lines[m_LineCount - 1].m_CharLength, m_Lines[m_LineCount - 1].m_RawLength);
			m_Flag |= SelectEnabled;
			return true;
		} else if (Key == LWKey::C) {
			if (m_SelectCursorLength == 0) return false;
			GetSelectedText(SelectBuffer, sizeof(SelectBuffer));
			LWWindow::WriteClipboardText(SelectBuffer);
			return true;
		} else if (Key == LWKey::X) {
			if (m_SelectCursorLength == 0) return false;
			
			GetSelectedText(SelectBuffer, sizeof(SelectBuffer));
			LWWindow::WriteClipboardText(SelectBuffer);
			ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
			return true;
		} else if (Key == LWKey::V) {
			bool Res = false;
			if (m_SelectCursorLength) Res = ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
			uint32_t Len = LWWindow::ReadClipboardText(SelectBuffer, sizeof(SelectBuffer));
			SelectBuffer[Len] = '\0';
			InsertText(SelectBuffer, false, false, Scale);
			return true;
		} else if (Key == LWKey::N) { //special key for touch presses!
			if(isPasswordField()){
				m_SelectCursor = LWETextInputCursor(m_Cursor.m_Line, 0u, 0u);
				m_Cursor = LWETextInputCursor(m_Cursor.m_Line, m_Lines[m_Cursor.m_Line].m_CharLength, m_Lines[m_Cursor.m_Line].m_RawLength);
				m_SelectCursorLength = (m_Cursor.m_Position - m_SelectCursor.m_Position);
				m_Flag |= SelectEnabled;
				return true;
			}
			char *F = LWText::NextWord(m_Lines[m_Cursor.m_Line].m_Value + m_Cursor.m_RawPosition, true);
			char *N = LWText::NextWord(F);
			if (!N) N = m_Lines[m_Cursor.m_Line].m_Value + m_Lines[m_Cursor.m_Line].m_RawLength;
			m_SelectCursor = LWETextInputCursor(m_Cursor.m_Line, m_Lines[m_Cursor.m_Line].m_Value, F);
			m_Cursor = LWETextInputCursor(m_Cursor.m_Line, m_Lines[m_Cursor.m_Line].m_Value, N);
			m_SelectCursorLength = m_Cursor.m_Position - m_SelectCursor.m_Position;
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
	char Buffer = '\0';
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
		Distance += B.m_Position;
		RawDistance += B.m_RawPosition;
		B.m_Line--;
		B.m_Position = m_Lines[B.m_Line].m_CharLength + 1;
		B.m_RawPosition = m_Lines[B.m_Line].m_RawLength + 1;
	}
	while (A.m_Line > B.m_Line) {
		Distance -= A.m_Position;
		RawDistance -= A.m_RawPosition;
		A.m_Line--;
		A.m_Position = m_Lines[A.m_Line].m_CharLength + 1;
		A.m_RawPosition = m_Lines[A.m_Line].m_RawLength + 1;
	}
	Distance += B.m_Position - A.m_Position;
	RawDistance += B.m_RawPosition - A.m_RawPosition;
	return Distance;
}

int32_t LWEUITextInput::MoveCursorBy(LWETextInputCursor &Cursor, int32_t Distance) {
	int32_t D = 0;
	for (; Distance < 0 && MoveCursorLeft(Cursor); Distance++, D--) {}
	for (; Distance > 0 && MoveCursorRight(Cursor); Distance--, D++) {}
	return D;
}

bool LWEUITextInput::MoveCursorUp(LWETextInputCursor &Cursor, float UIScale) {
	if (Cursor.m_Line == 0) {
		if (Cursor.m_Position != 0) {
			Cursor = LWETextInputCursor();
			return true;
		}
		return false;
	}
	if(!m_Font) Cursor.m_Position = std::min<uint32_t>(Cursor.m_Position, m_Lines[Cursor.m_Line-1].m_CharLength);
	else{
		char *C = m_Lines[Cursor.m_Line].m_Value;
		char *P = m_Lines[Cursor.m_Line - 1].m_Value;
		LWVector4f CSize = m_Font->MeasureText(C, Cursor.m_Position, m_FontScale*UIScale);
		Cursor.m_Position = m_Font->CharacterAt(P, CSize.z, m_Lines[Cursor.m_Line-1].m_CharLength, m_FontScale*UIScale);
	}
	Cursor.m_Line--;
	char *N = LWText::At(m_Lines[Cursor.m_Line].m_Value, Cursor.m_Position);
	if (!N) N = m_Lines[Cursor.m_Line].m_Value + m_Lines[Cursor.m_Line].m_RawLength;
	Cursor.m_RawPosition = (uint32_t)(uintptr_t)(N - m_Lines[Cursor.m_Line].m_Value);
	return true;
}

bool LWEUITextInput::MoveCursorDown(LWETextInputCursor &Cursor, float UIScale) {
	if (Cursor.m_Line + 1 >= m_LineCount) {
		if (Cursor.m_Position < m_Lines[Cursor.m_Line].m_CharLength) {
			Cursor = LWETextInputCursor(Cursor.m_Line, m_Lines[Cursor.m_Line].m_CharLength, m_Lines[Cursor.m_Line].m_RawLength);
			return true;
		}
		return false;
	}
	if (!m_Font) Cursor.m_Position = std::min<uint32_t>(Cursor.m_Position, m_Lines[Cursor.m_Line + 1].m_CharLength);
	else {
		char *C = m_Lines[Cursor.m_Line].m_Value;
		char *P = m_Lines[Cursor.m_Line + 1].m_Value;
		LWVector4f CSize = m_Font->MeasureText(C, Cursor.m_Position, m_FontScale*UIScale);
		Cursor.m_Position = m_Font->CharacterAt(P, CSize.z, m_Lines[Cursor.m_Line + 1].m_CharLength, m_FontScale*UIScale);
	}
	Cursor.m_Line++;
	char *N = LWText::At(m_Lines[Cursor.m_Line].m_Value, Cursor.m_Position);
	if (!N) N = m_Lines[Cursor.m_Line].m_Value + m_Lines[Cursor.m_Line].m_RawLength;
	Cursor.m_RawPosition = (uint32_t)(uintptr_t)(N - m_Lines[Cursor.m_Line].m_Value);
	return true;
}

bool LWEUITextInput::MoveCursorLeft(LWETextInputCursor &Cursor) {
	if (Cursor.m_Position == 0) {
		if (Cursor.m_Line == 0) return false;
		Cursor = LWETextInputCursor(Cursor.m_Line - 1, m_Lines[Cursor.m_Line - 1].m_CharLength, m_Lines[Cursor.m_Line - 1].m_RawLength);
		return true;
	}
	char *C = m_Lines[Cursor.m_Line].m_Value + Cursor.m_RawPosition;
	char *P = LWText::Prev(C, m_Lines[Cursor.m_Line].m_Value);
	if (!P) P = m_Lines[Cursor.m_Line].m_Value;
	Cursor.m_Position--;
	Cursor.m_RawPosition -= (uint32_t)(uintptr_t)(C - P);
	return true;
}

bool LWEUITextInput::MoveCursorRight(LWETextInputCursor &Cursor) {
	if (Cursor.m_Position == m_Lines[Cursor.m_Line].m_CharLength) {
		if (Cursor.m_Line + 1 >= m_LineCount) return false;
		Cursor = LWETextInputCursor(Cursor.m_Line + 1, 0u, 0u);
		return true;
	}
	char *C = m_Lines[Cursor.m_Line].m_Value + Cursor.m_RawPosition;
	char *N = LWText::Next(C);
	if (!N) N = m_Lines[Cursor.m_Line].m_Value + m_Lines[Cursor.m_Line].m_RawLength;
	Cursor.m_Position++;
	Cursor.m_RawPosition += (uint32_t)(uintptr_t)(N - C);
	return true;
}


uint32_t LWEUITextInput::InsertText(const LWText &Text, bool ShiftDown, bool CtrlDown, float Scale) {
	uint32_t Cnt = 0;
	for (const char *C = LWText::FirstCharacter((const char*)Text.GetCharacters()); C; C = LWText::Next(C)) {
		if (InsertChar(LWText::GetCharacter(C), ShiftDown, CtrlDown, Scale)) Cnt++;
	}
	return Cnt;
}

uint32_t LWEUITextInput::InsertTextf(const char *Fmt, bool ShiftDown, bool CtrlDown, float Scale, ...) {
	char Buffer[LWETEXTINPUT_MAXLINELENGTH];
	va_list lst;
	va_start(lst, Scale);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return InsertText(Buffer, ShiftDown, CtrlDown, Scale);
}

bool LWEUITextInput::InsertChar(uint32_t Char, bool ShiftDown, bool CtrlDown, float Scale) {
	if (Char == '\n') return ProcessKey(LWKey::Return, ShiftDown, CtrlDown, Scale);
	else if (Char < ' ') return false;
	bool Allowed = true;
	if (*m_AllowedCharacters) {
		Allowed = (m_Flag&InvertAllowed) != 0;
		for (const char *C = LWText::FirstCharacter(m_AllowedCharacters); C; C = LWText::Next(C)) {
			uint32_t n = LWText::GetCharacter(C);
			if (n == Char) {
				Allowed = (m_Flag&InvertAllowed) == 0;
				break;
			}
		}
	}
	if (!Allowed) return false;
	if (m_SelectCursorLength) ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
	if (m_CurrentLength >= m_MaxLength) return false;

	uint32_t CharLen = LWText::GetUTF8Size(Char);
	char *Line = m_Lines[m_Cursor.m_Line].m_Value;
	uint32_t RawLen = m_Lines[m_Cursor.m_Line].m_RawLength;
	if (RawLen + CharLen + 1 > sizeof(m_Lines[m_Cursor.m_Line].m_Value)) return false;
	std::copy_backward(Line + m_Cursor.m_RawPosition, Line + RawLen+1, Line + RawLen+CharLen+1);
	LWText::MakeUTF32To8(&Char, (uint8_t*)Line + m_Cursor.m_RawPosition, CharLen);
	m_Cursor.m_Position++;
	m_Cursor.m_RawPosition += CharLen;
	m_Lines[m_Cursor.m_Line].m_CharLength++;
	m_Lines[m_Cursor.m_Line].m_RawLength += CharLen;
	m_CurrentLength++;
	return true;
}

bool LWEUITextInput::ScrollToCursor(float Scale) {
	if (!m_Font) return false;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	LWVector4f LineSizeToCursor = m_Font->MeasureText(m_Lines[m_Cursor.m_Line].m_Value, m_Cursor.m_Position, m_FontScale*Scale);
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
	if (!m_Font) return false;
	float MaxHoriScroll = std::max<float>(0.0f, m_LongestLine - m_VisibleSize.x);
	float MaxVerticalScroll = std::max<float>(0.0f, (m_LineCount*m_Font->GetLineSize()*m_FontScale*Scale + (m_Font->GetLineSize()*m_FontScale*Scale*0.25f)) - m_VisibleSize.y);
	m_HorizontalScroll = std::max<float>(std::min<float>(HoriScroll, MaxHoriScroll), 0.0f);
	m_VerticalScroll = std::max<float>(std::min<float>(VertScroll, MaxVerticalScroll), 0.0f);
	return true;
}


LWEUITextInput &LWEUITextInput::Clear(void) {
	m_Lines[0].m_Value[0] = '\0';
	m_Lines[0].m_CharLength = m_Lines[0].m_RawLength = 0;
	m_SelectCursor = LWETextInputCursor(0, 0u, 0u);
	m_Cursor = LWETextInputCursor(0, 0u, 0u);
	m_SelectCursorLength = 0;
	m_LineCount = 1;
	m_CurrentLength = 0;
	m_VerticalScroll = m_HorizontalScroll = 0.0f;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetFontScale(float FontScale) {
	m_FontScale = FontScale;
	return *this;
}

LWEUITextInput &LWEUITextInput::SetAllowedCharacters(const char *Characters) {
	uint32_t Len = (uint32_t)strlen(Characters) + 1;
	Len = std::min<uint32_t>(sizeof(m_AllowedCharacters) - 1, Len);
	memcpy(m_AllowedCharacters, Characters, sizeof(char)*Len);
	m_AllowedCharacters[Len] = '\0';
	return *this;
}

LWEUITextInput &LWEUITextInput::SetDefaultText(const char *Text) {
	*m_DefaultText = '\0';
	strncat(m_DefaultText, Text, sizeof(m_DefaultText));
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
	CursorPosition = std::min<uint32_t>(m_Cursor.m_Position, m_Lines[CursorLine].m_CharLength);
	char *N = LWText::At(m_Lines[CursorLine].m_Value, CursorPosition);
	if (!N) N = m_Lines[CursorLine].m_Value + m_Lines[CursorLine].m_RawLength;
	m_Cursor = LWETextInputCursor(CursorLine, CursorPosition, (uint32_t)(uintptr_t)(N - m_Lines[CursorLine].m_Value));
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
	Position = std::min<uint32_t>(Position, m_Lines[Line].m_CharLength);
	char *N = LWText::At(m_Lines[Line].m_Value, Position);
	if (!N) N = m_Lines[Line].m_Value + m_Lines[Line].m_RawLength;
	m_SelectCursor = LWETextInputCursor(Line, Position, (uint32_t)(uintptr_t)(N - m_Lines[Line].m_Value));
	uint32_t SelectLen = 0;
	for (; Length < 0 && MoveCursorLeft(m_SelectCursor); Length++, SelectLen--) {}
	for (; Length > 0 && MoveCursorRight(m_SelectCursor); Length--, SelectLen++) {}
	m_SelectCursorLength = SelectLen;
	m_Flag = (m_Flag&~SelectEnabled) | (m_SelectCursorLength != 0 ? SelectEnabled : 0);
	return *this;
}

uint32_t LWEUITextInput::GetSelectedText(char *Buffer, uint32_t BufferLen) {
	if (isPasswordField()) {
		Buffer[0] = '\0';
		return 0;
	}
	uint32_t o = 0;
	char *BLast = Buffer + BufferLen;
	LWETextInputCursor S = m_SelectCursor;
	int32_t SLen = m_SelectCursorLength;
	if (SLen < 0) {
		S = m_Cursor;
		SLen = -SLen;
	}
	for (int32_t i = 0; i < SLen && (Buffer + o) != BLast;) {
		uint32_t n = std::min<uint32_t>(SLen - i, m_Lines[S.m_Line].m_CharLength - S.m_Position);
		o += LWText::Copy(m_Lines[S.m_Line].m_Value + S.m_RawPosition, n, Buffer + o, BufferLen - o);
		if ((i + n != SLen)) {
			if (Buffer + o != BLast) Buffer[o++] = '\n';
			if (Buffer + o != BLast) Buffer[o] = '\0';
			n++;
		}
		S = LWETextInputCursor(S.m_Line + 1, 0u, 0u);
		if (S.m_Line >= m_LineCount) break;
		i += n;
	}
	return o;
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

float LWEUITextInput::GetFontScale(void) const {
	return m_FontScale;
}

float LWEUITextInput::GetDefaultFontScale(void) const {
	return m_DefaultFontScale;
}

bool LWEUITextInput::isPasswordField(void) const {
	return (m_Flag&PasswordField) != 0;
}

const char *LWEUITextInput::GetAllowedCharacters(void) const {
	return m_AllowedCharacters;
}

const char *LWEUITextInput::GetDefaultText(void) const {
	return m_DefaultText;
}

LWEUITextInput::LWEUITextInput(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag) {
	*m_DefaultText = '\0';
	m_Lines[0].m_Value[0] = '\0';
	m_Lines[0].m_RawLength = m_Lines[0].m_CharLength = 0;
	m_AllowedCharacters[0] = '\0';
}

LWEUITextInput::~LWEUITextInput() {}
