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

LWEUITextInput *LWEUITextInput::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUITextInput *TextInput = Allocator->Allocate<LWEUITextInput>(LWVector4f(0.0f), LWVector4f(0.0f), FocusAble | TabAble);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
	LWEUI::XMLParse(TextInput, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWXMLAttribute *BorderMaterialAttr = FindAttribute(Node, Style, "BorderMaterial");
	LWXMLAttribute *FontMaterialAttr = FindAttribute(Node, Style, "FontMaterial");
	LWXMLAttribute *SelectMaterialAttr = FindAttribute(Node, Style, "SelectMaterial");
	LWXMLAttribute *TextAreaMaterialAttr = FindAttribute(Node, Style, "TextAreaMaterial");
	LWXMLAttribute *CursorMaterialAttr = FindAttribute(Node, Style, "CursorMaterial");
	LWXMLAttribute *AllowedCharactersAttr = FindAttribute(Node, Style, "AllowedCharacters");
	LWXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWXMLAttribute *ValueAttr = FindAttribute(Node, Style, "Value");
	LWXMLAttribute *BorderSizeAttr = FindAttribute(Node, Style, "BorderSize");
	LWXMLAttribute *CursorSizeAttr = FindAttribute(Node, Style, "CursorSize");
	LWXMLAttribute *MaxLengthAttr = FindAttribute(Node, Style, "MaxLength");
	LWXMLAttribute *MaxLinesAttr = FindAttribute(Node, Style, "MaxLines");
	LWXMLAttribute *FontScaleAttr = FindAttribute(Node, Style, "FontScale");
	LWXMLAttribute *BtnOffAttr = FindAttribute(Node, Style, "ButtonOffMaterial");
	LWXMLAttribute *BtnOverAttr = FindAttribute(Node, Style, "ButtonOverMaterial");
	LWXMLAttribute *BtnDownAttr = FindAttribute(Node, Style, "ButtonDownMaterial");
	LWXMLAttribute *BtnFontScaleAttr = FindAttribute(Node, Style, "ButtonFontScale");
	LWXMLAttribute *BtnFontMaterialAttr = FindAttribute(Node, Style, "ButtonFontMaterial");

	LWEUIMaterial *BorderMat = nullptr;
	LWEUIMaterial *FontMat = nullptr;
	LWEUIMaterial *BtnFontMat = nullptr;
	LWEUIMaterial *SelectMat = nullptr;
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
	if (BtnFontMaterialAttr) BtnFontMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (SelectMaterialAttr) SelectMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), SelectMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (TextAreaMaterialAttr) TextAreaMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), TextAreaMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (CursorMaterialAttr) CursorMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), CursorMaterialAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnOffAttr) BtnOffMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOffAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnDownAttr) BtnDownMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnDownAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (BtnOverAttr) BtnOverMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnOverAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (FontAttr) Font = (LWFont*)FindAsset(Manager->GetAssetManager(), FontAttr->m_Value, LWEAsset::Font);
	if (BorderSizeAttr) BorderSize = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), BorderSizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (CursorSizeAttr) CursorSize = (float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), CursorSizeAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (MaxLengthAttr) MaxLength = (uint32_t)atoi(ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLengthAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (MaxLinesAttr) MaxLines = (uint32_t)atoi(ParseComponentAttribute(Buffer, sizeof(Buffer), MaxLinesAttr->m_Value, ActiveComponent, ActiveComponentNode));
	if (FontScaleAttr) TextInput->SetFontScale((float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), FontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
	if (BtnFontScaleAttr) TextInput->SetBtnFontScale((float)atof(ParseComponentAttribute(Buffer, sizeof(Buffer), BtnFontScaleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
	if (AllowedCharactersAttr) TextInput->SetAllowedCharacters(ParseComponentAttribute(Buffer, sizeof(Buffer), AllowedCharactersAttr->m_Value, ActiveComponent, ActiveComponentNode));

	TextInput->SetBorderMaterial(BorderMat).SetFontMaterial(FontMat).SetSelectMaterial(SelectMat).SetTextAreaMaterial(TextAreaMat).SetCursorMaterial(CursorMat).SetBtnOffMaterial(BtnOffMat).SetBtnOverMaterial(BtnOverMat).SetBtnDownMaterial(BtnDownMat).SetBtnFontMaterial(BtnFontMat);
	TextInput->SetFont(Font).SetBorderSize(BorderSize).SetCursorSize(CursorSize).SetMaxLength(MaxLength).SetMaxLines(MaxLines);
	if (ValueAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), ValueAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		TextInput->InsertText(Res, false, false, 1.0f);
	}
	return TextInput;
}

LWEUI &LWEUITextInput::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {

	static char PasswordLine[LWETEXTINPUT_MAXLINELENGTH];
	static bool Initiated = false;
	if (!Initiated) {
		memset(PasswordLine, '*', sizeof(PasswordLine));
		Initiated = true;
	}


	auto NextChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == Lines[CurrentLine].m_CharLength) {
			if (CurrentLine + 1 >= TotalLines) return false;
			CurrentLine++;
			CurrentPos = 0;
			CurrentRawPos = 0;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *N = LWText::Next(C);
		if (!N) N = Lines[CurrentLine].m_Value + Lines[CurrentLine].m_RawLength;
		CurrentPos++;
		CurrentRawPos += (uint32_t)(uintptr_t)(N - C);
		return true;
	};

	auto PrevChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == 0) {
			if (CurrentLine == 0) return false;
			CurrentLine--;
			CurrentPos = Lines[CurrentLine].m_CharLength;
			CurrentRawPos = Lines[CurrentLine].m_RawLength;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *P = LWText::Prev(C, Lines[CurrentLine].m_Value);
		if (!P) P = Lines[CurrentLine].m_Value;

		CurrentPos--;
		CurrentRawPos -= (uint32_t)(uintptr_t)(C - P);
		return true;
	};

	auto MoveCursorBy = [&PrevChar, &NextChar](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, int32_t Distance, LWTextLine *Lines, uint32_t TotalLines)->int32_t {
		int32_t D = 0;
		for (; Distance < 0 && PrevChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance++, D--) {}
		for (; Distance > 0 && NextChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance--, D++) {}
		return D;
	};

	auto CursorDistance = [](uint32_t APos, uint32_t ARawPos, uint32_t ALine, uint32_t BPos, uint32_t BRawPos, uint32_t BLine, int32_t &CharDistance, int32_t &RawDistance, LWTextLine *Lines, uint32_t TotalLines)->bool {

		while (BLine > ALine) {
			CharDistance += BPos;
			RawDistance += BRawPos;
			BLine--;
			BPos = Lines[BLine].m_CharLength + 1;
			BRawPos = Lines[BLine].m_RawLength + 1;
		}
		while (ALine > BLine) {
			CharDistance -= APos;
			RawDistance -= ARawPos;
			ALine--;
			APos = Lines[ALine].m_CharLength + 1;
			ARawPos = Lines[ALine].m_RawLength + 1;
		}
		CharDistance += BPos - APos;
		RawDistance += BRawPos - ARawPos;
		return true;
	};

	LWWindow *Wnd = Manager->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWKeyboard *Keyboard = Wnd->GetKeyboardDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();

	uint32_t Flag = m_Flag;
	m_Flag &= ~(MouseOver | MouseDown);

	LWVector2f DownPnt = LWVector2f(-1000.0f, -1000.0f);
	LWVector2f CopyBtnPos = DownPnt;
	LWVector2f CutBtnPos = DownPnt;
	LWVector2f PasteBtnPos = DownPnt;
	LWVector2f SelectAllBtnPos = DownPnt;
	bool OverMain = false;
	bool OverCopy = false;
	bool OverCut = false;
	bool OverPaste = false;
	bool OverSelect = false;
	bool SelectPressed = false;
	m_CopyMaterial = m_BtnOffMaterial;
	m_CutMaterial = m_BtnOffMaterial;
	m_PasteMaterial = m_BtnOffMaterial;
	m_SelectAllMaterial = m_BtnOffMaterial;

	if ((m_Flag&(TouchEnabled | SelectEnabled)) == (TouchEnabled | SelectEnabled)) CalculateBtnPositions(CopyBtnPos, CutBtnPos, PasteBtnPos, SelectAllBtnPos, Scale);
	m_CopyBtn = LWVector4f(m_CopyBtn.x, m_CopyBtn.y, CopyBtnPos.x, CopyBtnPos.y);
	m_CutBtn = LWVector4f(m_CutBtn.x, m_CutBtn.y, CutBtnPos.x, CutBtnPos.y);
	m_PasteBtn = LWVector4f(m_PasteBtn.x, m_PasteBtn.y, PasteBtnPos.x, PasteBtnPos.y);
	m_SelectAllBtn = LWVector4f(m_SelectAllBtn.x, m_SelectAllBtn.y, SelectAllBtnPos.x, SelectAllBtnPos.y);

	if (Mouse) {
		LWVector2i MP = Mouse->GetPosition();
		LWVector2f Pnt = LWVector2f((float)MP.x, (float)MP.y);
		OverMain = PointInside(Pnt, 0.0f, m_VisiblePosition, m_VisibleSize);
		if (OverMain) {
			Manager->DispatchEvent(this, Event_TempOverInc); 
			m_Flag |= MouseOver;
		}
		if (Mouse->ButtonDown(LWMouseKey::Left)) {
			if (m_Flag&MouseOver) {
				m_Flag |= MouseDown;
				DownPnt = Pnt;
			} else if (Manager->GetFocusedUI() == this) {
				m_SelectCursorLength = 0;
				Manager->SetFocused(nullptr);
			}
		} else {
			if (m_Flag&MouseOver) {
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
			auto Pnt = Touch->GetPoint(i);
			LWVector2f TouchPnt = LWVector2f((float)Pnt->m_Position.x, (float)Pnt->m_Position.y);
			OverMain = PointInside(TouchPnt, Pnt->m_Size*Scale, m_VisiblePosition, m_VisibleSize);
			OverCopy = PointInside(TouchPnt, Pnt->m_Size*Scale, CopyBtnPos, LWVector2f(m_CopyBtn.x, m_Font->GetLineSize()*m_FontBtnScale)*Scale);
			OverCut = PointInside(TouchPnt, Pnt->m_Size*Scale, CutBtnPos, LWVector2f(m_CutBtn.x, m_Font->GetLineSize()*m_FontBtnScale)*Scale);
			OverPaste = PointInside(TouchPnt, Pnt->m_Size*Scale, PasteBtnPos, LWVector2f(m_PasteBtn.x, m_Font->GetLineSize()*m_FontBtnScale)*Scale);
			OverSelect = PointInside(TouchPnt, Pnt->m_Size*Scale, SelectAllBtnPos, LWVector2f(m_SelectAllBtn.x, m_Font->GetLineSize()*m_FontBtnScale)*Scale);
			if(OverCopy) m_CopyMaterial = m_BtnOverMaterial;
			if(OverCut) m_CutMaterial = m_BtnOverMaterial;
			if(OverPaste) m_PasteMaterial = m_BtnOverMaterial;
			if(OverSelect) m_SelectAllMaterial = m_BtnOverMaterial;
			bool Over = OverMain || OverCopy || OverCut || OverPaste || OverSelect;
			if (Over) {
				Manager->DispatchEvent(this, Event_TempOverInc|(i<<Event_OverOffset));
				m_Flag |= MouseOver;
				if (Pnt->m_State != LWTouchPoint::UP) {
					if (Pnt->m_State == LWTouchPoint::DOWN) Flag = (Flag&~MouseDown) | MouseOver;
					if (OverCopy) m_CopyMaterial = m_BtnDownMaterial;
					if (OverCut) m_CutMaterial = m_BtnDownMaterial;
					if (OverPaste) m_PasteMaterial = m_BtnDownMaterial;
					if (OverSelect) m_SelectAllMaterial = m_BtnDownMaterial;

					m_Flag |= MouseDown;
					DownPnt = TouchPnt;
				} else{
					if (!(m_Flag&TouchEnabled)) {
						m_Flag |= TouchEnabled;
						Manager->SetFocused(this);
					}
					if (OverCopy) ProcessKey(LWKey::C, false, true, Scale);
					if (OverCut) ProcessKey(LWKey::X, false, true, Scale);
					if (OverPaste) ProcessKey(LWKey::V, false, true, Scale);
					if (OverSelect) ProcessKey(LWKey::A, false, true, Scale);

				}
			} else if (Pnt->m_State == LWTouchPoint::UP && Manager->GetFocusedUI() == this) {
				m_SelectCursorLength = 0;
				Manager->SetFocused(nullptr);
			}
		}
	}
	SetScroll(m_HorizontalScroll + m_ScrollAcceleration.x, m_VerticalScroll + m_ScrollAcceleration.y, Scale);
	m_ScrollAcceleration *= 0.9f;
	if (m_Flag&MouseOver && (Flag&MouseOver) == 0) Manager->DispatchEvent(this, Event_MouseOver);
	if ((m_Flag&MouseOver) == 0 && Flag&MouseOver) Manager->DispatchEvent(this, Event_MouseOff);
	if ((Flag&(MouseDown | MouseOver)) == MouseOver && (m_Flag&MouseDown)) Manager->DispatchEvent(this, Event_Released);
	if ((m_Flag&(MouseDown | MouseOver)) == (MouseDown | MouseOver) && (Flag&MouseDown) == 0) Manager->DispatchEvent(this, Event_Pressed);
	
	if ((m_Flag&(MouseDown | FocusAble)) == (MouseDown | FocusAble) && Manager->GetFocusedUI() != this) Manager->SetFocused(this);
	if (Manager->GetFocusedUI() == this) {
		bool Changed = false;
		if (Keyboard) {
			uint32_t KeyChangeCnt = Keyboard->GetKeyChangeCount();
			uint32_t CharCnt = Keyboard->GetCharPressed();
			bool Shift = Keyboard->ButtonDown(LWKey::LShift) || Keyboard->ButtonDown(LWKey::RShift);
			bool Ctrl = Keyboard->ButtonDown(LWKey::LCtrl) || Keyboard->ButtonDown(LWKey::RCtrl);
			for (uint32_t i = 0; i < KeyChangeCnt; i++) {
				if (Keyboard->GetKeyState(i)) {
					if (ProcessKey((LWKey)Keyboard->GetKeyChanged(i), Shift, Ctrl, Scale)) Changed = true;;
				}
			}
			for (uint32_t i = 0; i < CharCnt; i++) {
				if (Keyboard->GetChar(i) == '\n') continue;
				if (InsertChar(Keyboard->GetChar(i), Shift, Ctrl, Scale)) Changed = true;
			}
		}

		float MaxVerticalScroll = 0.0f;
		if (m_Font) MaxVerticalScroll = std::max<float>(0.0f, (m_LineCount*m_Font->GetLineSize()*m_FontScale*Scale + (m_Font->GetLineSize()*m_FontScale*Scale*0.25f)) - m_VisibleSize.y);
		if ((m_Flag&(MouseOver | MouseDown)) == (MouseOver | MouseDown) && OverMain && !SelectPressed) {
			LWVector2f RelPos = LWVector2f(DownPnt.x - m_VisiblePosition.x, m_VisibleSize.y - (DownPnt.y - m_VisiblePosition.y)) + LWVector2f(m_HorizontalScroll, m_VerticalScroll);
			uint32_t Line = std::min<uint32_t>((uint32_t)((float)RelPos.y / (m_Font->GetLineSize()*m_FontScale*Scale)), m_LineCount - 1);
			char *CLine = (m_Flag&PasswordField) ? PasswordLine : m_Lines[Line].m_Value;

			uint32_t Pos = m_Font->CharacterAt(CLine, RelPos.x, m_Lines[Line].m_CharLength, m_FontScale*Scale);
			char *RawPos = LWText::At(m_Lines[Line].m_Value, Pos);
			if (!RawPos) RawPos = m_Lines[Line].m_Value + m_Lines[Line].m_RawLength;
			uint32_t CRawPos = (uint32_t)(uintptr_t)(RawPos - m_Lines[Line].m_Value);

			if ((Flag&(MouseDown | MouseOver)) == MouseOver) {
				m_SelectCursorPosition = Pos;
				m_SelectCursorRawPosition = CRawPos;
				m_SelectCursorLine = Line;
			}
			m_CursorLine = Line;
			m_CursorPosition = Pos;
			m_CursorRawPosition = CRawPos;
			int32_t Dis = 0;
			int32_t RawDis = 0;
			CursorDistance(m_SelectCursorPosition, m_SelectCursorRawPosition, m_SelectCursorLine, m_CursorPosition, m_CursorRawPosition, m_CursorLine, Dis, RawDis, m_Lines, m_LineCount);
			m_SelectCursorLength = Dis;
			Wnd->SetKeyboardText(CLine);
			Wnd->SetKeyboardEditRange(Pos, Dis);
			Changed = true;
		}
		if (Mouse) {
			m_VerticalScroll -= (float)Mouse->GetScroll()*0.25f;
			m_VerticalScroll = std::max<float>(std::min<float>(m_VerticalScroll, MaxVerticalScroll), 0.0f);
		}
		if (Changed) {
			ScrollToCursor(Scale);
			Manager->DispatchEvent(this, Event_Changed);
		}
	} else m_Flag &= ~(TouchEnabled|SelectEnabled);
	
	
	return *this;
}

LWEUI &LWEUITextInput::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	static char PasswordLine[LWETEXTINPUT_MAXLINELENGTH];
	static bool Initiated = false;
	if (!Initiated) {
		memset(PasswordLine, '*', sizeof(PasswordLine));
		Initiated = true;
	}

	auto NextChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == Lines[CurrentLine].m_CharLength) {
			if (CurrentLine + 1 >= TotalLines) return false;
			CurrentLine++;
			CurrentPos = 0;
			CurrentRawPos = 0;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *N = LWText::Next(C);
		if (!N) N = Lines[CurrentLine].m_Value + Lines[CurrentLine].m_RawLength;
		CurrentPos++;
		CurrentRawPos += (uint32_t)(uintptr_t)(N - C);
		return true;
	};

	auto PrevChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == 0) {
			if (CurrentLine == 0) return false;
			CurrentLine--;
			CurrentPos = Lines[CurrentLine].m_CharLength;
			CurrentRawPos = Lines[CurrentLine].m_RawLength;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *P = LWText::Prev(C, Lines[CurrentLine].m_Value);
		if (!P) P = Lines[CurrentLine].m_Value;

		CurrentPos--;
		CurrentRawPos -= (uint32_t)(uintptr_t)(C - P);
		return true;
	};

	auto MoveCursorBy = [&PrevChar, &NextChar](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, int32_t Distance, LWTextLine *Lines, uint32_t TotalLines)->int32_t {
		int32_t D = 0;
		for (; Distance < 0 && PrevChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance++, D--) {}
		for (; Distance > 0 && NextChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance--, D++) {}
		return D;
	};

	auto DrawRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, LWEUIFrame *F)->bool {
		if (!Mat) return true;
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

	DrawRect(m_BorderMaterial, m_VisiblePosition - LWVector2f(m_BorderSize), m_VisibleSize + LWVector2f(m_BorderSize*2.0f), Frame);
	if (!DrawRect(m_TextAreaMaterial, m_VisiblePosition, m_VisibleSize, Frame)) return *this;
	if (!m_Font) return *this;
	if (!m_FontMaterial) return *this;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	float BtnLineSize = m_Font->GetLineSize()*m_FontBtnScale*Scale;
	
	if (m_BtnFontMaterial) {
		if (m_CopyBtn.z >= 0.0f) {
			if (!DrawRect(m_CopyMaterial, LWVector2f(m_CopyBtn.z, m_CopyBtn.w), LWVector2f(m_CopyBtn.x*Scale, BtnLineSize), Frame)) return *this;
		}
		if (m_CutBtn.z >= 0.0f) {
			if (!DrawRect(m_CutMaterial, LWVector2f(m_CutBtn.z, m_CutBtn.w), LWVector2f(m_CutBtn.x*Scale, BtnLineSize), Frame)) return *this;
		}
		if (m_PasteBtn.z >= 0.0f) {
			if (!DrawRect(m_PasteMaterial, LWVector2f(m_PasteBtn.z, m_PasteBtn.w), LWVector2f(m_PasteBtn.x*Scale, BtnLineSize), Frame)) return *this;
		}
		if (m_SelectAllBtn.z >= 0.0f) {
			if (!DrawRect(m_SelectAllMaterial, LWVector2f(m_SelectAllBtn.z, m_SelectAllBtn.w), LWVector2f(m_SelectAllBtn.x*Scale, BtnLineSize), Frame)) return *this;
		}
	}
	//if (!Frame->SetActiveTexture(m_Font->GetTexture(), true)) return *this;

	//uint32_t p = Frame->m_Mesh->GetActiveCount();

	
	if (m_BtnFontMaterial) {
		if (m_CopyBtn.z >= 0.0f) {
			m_Font->DrawTextm("Copy", LWVector2f(m_CopyBtn.z, m_CopyBtn.w +m_CopyBtn.y*Scale), m_FontBtnScale*Scale, m_BtnFontMaterial->m_Color, Frame, &LWEUIFrame::WriteFontGlyph);
		}
		if (m_CutBtn.z >= 0.0f) {
			m_Font->DrawTextm("Cut", LWVector2f(m_CutBtn.z, m_CutBtn.w +m_CutBtn.y*Scale), m_FontBtnScale*Scale, m_BtnFontMaterial->m_Color, Frame, &LWEUIFrame::WriteFontGlyph);
		}
		if (m_PasteBtn.z >= 0.0f) {
			m_Font->DrawTextm("Paste", LWVector2f(m_PasteBtn.z, m_PasteBtn.w +m_PasteBtn.y*Scale), m_FontBtnScale*Scale, m_BtnFontMaterial->m_Color, Frame, &LWEUIFrame::WriteFontGlyph);
		}
		if (m_SelectAllBtn.z >= 0.0f) {
			m_Font->DrawTextm("Select All", LWVector2f(m_SelectAllBtn.z, m_SelectAllBtn.w + m_SelectAllBtn.y*Scale), m_FontBtnScale*Scale, m_BtnFontMaterial->m_Color, Frame, &LWEUIFrame::WriteFontGlyph);
		}
	}
	uint32_t SCursorBeginPosition = m_SelectCursorPosition;
	uint32_t SCursorBeginRawPosition = m_SelectCursorRawPosition;
	uint32_t SCursorBeginLine = m_SelectCursorLine;
	uint32_t SCursorEndPosition = m_SelectCursorPosition;
	uint32_t SCursorEndRawPosition = m_SelectCursorRawPosition;
	uint32_t SCursorEndLine = m_SelectCursorLine;
	if (m_SelectCursorLength < 0) MoveCursorBy(SCursorBeginPosition, SCursorBeginRawPosition, SCursorBeginLine, m_SelectCursorLength, m_Lines, m_LineCount);
	else MoveCursorBy(SCursorEndPosition, SCursorEndRawPosition, SCursorEndLine, m_SelectCursorLength, m_Lines, m_LineCount);

	for (uint32_t i = 0; i < m_LineCount; i++) {
		char *Line = (m_Flag&PasswordField) ? PasswordLine : m_Lines[i].m_Value;
		m_Font->DrawClippedTextm(Line, m_Lines[i].m_CharLength, m_VisiblePosition + LWVector2f(0.0f, m_VisibleSize.y - LineSize*(i + 1)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), m_FontScale*Scale, m_FontMaterial->m_Color, LWVector4f(m_VisiblePosition, m_VisibleSize), Frame, &LWEUIFrame::WriteFontGlyph);
	}
	//uint32_t c = Frame->m_Mesh->GetActiveCount();
	//Frame->m_VertexCount[Frame->m_TextureCount - 1] += (c - p);

	if (SCursorBeginPosition != SCursorEndPosition || SCursorBeginLine != SCursorEndLine) {
		for (uint32_t i = SCursorBeginLine; i <= SCursorEndLine; i++) {
			char *Line = (m_Flag&PasswordField) ? PasswordLine : m_Lines[i].m_Value;
			LWVector4f BeginPos;
			LWVector4f EndPos;
			if (i == SCursorBeginLine) {
				BeginPos = m_Font->MeasureText(Line, SCursorBeginPosition, m_FontScale*Scale);
				uint32_t e = (SCursorBeginLine == SCursorEndLine) ? SCursorEndPosition : m_Lines[i].m_CharLength;
				EndPos = m_Font->MeasureText(Line, e, m_FontScale*Scale);
			} else if (i == SCursorEndLine) EndPos = m_Font->MeasureText(Line, SCursorEndPosition, m_FontScale*Scale);
			else if (i > SCursorBeginLine && i < SCursorEndLine) EndPos = m_Font->MeasureText(Line, m_FontScale*Scale);
			DrawClippedRect(m_SelectMaterial, m_VisiblePosition + LWVector2f(BeginPos.z, m_VisibleSize.y - (LineSize*(i + 1) + LineSize*0.25f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll), LWVector2f(EndPos.z - BeginPos.z, LineSize), Frame, LWVector4f(m_VisiblePosition, m_VisibleSize));
		}
	}
	if (Manager->GetFocusedUI() == this) {
		uint64_t HalfSeconds = lCurrentTime / (LWTimer::GetResolution() / 2);
		if ((HalfSeconds % 3) >= 1) {
			char *Line = (m_Flag&PasswordField) ? PasswordLine : m_Lines[m_CursorLine].m_Value;
			LWVector4f LineSizeToCursor = m_Font->MeasureText(Line, m_CursorPosition, m_FontScale*Scale);
			LWVector2f CursorCtr = m_VisiblePosition + LWVector2f(LineSizeToCursor.z, m_VisibleSize.y - (LineSize*m_CursorLine + LineSize*0.75f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll);
			DrawClippedRect(m_CursorMaterial, CursorCtr - LWVector2f(m_CursorSize, LineSize)*0.5f, LWVector2f(m_CursorSize*2.0f, LineSize), Frame, LWVector4f(m_VisiblePosition, m_VisibleSize));
		}
	}
	return *this;
}

bool LWEUITextInput::ProcessKey(LWKey Key, bool ShiftDown, bool CtrlDown, float Scale) {
	
	auto NextChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == Lines[CurrentLine].m_CharLength) {
			if (CurrentLine + 1 >= TotalLines) return false;
			CurrentLine++;
			CurrentPos = 0;
			CurrentRawPos = 0;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *N = LWText::Next(C);
		if (!N) N = Lines[CurrentLine].m_Value + Lines[CurrentLine].m_RawLength;
		CurrentPos++;
		CurrentRawPos += (uint32_t)(uintptr_t)(N - C);
		return true;
	};

	auto PrevChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == 0) {
			if (CurrentLine == 0) return false;
			CurrentLine--;
			CurrentPos = Lines[CurrentLine].m_CharLength;
			CurrentRawPos = Lines[CurrentLine].m_RawLength;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *P = LWText::Prev(C, Lines[CurrentLine].m_Value);
		if (!P) P = Lines[CurrentLine].m_Value;

		CurrentPos--;
		CurrentRawPos -= (uint32_t)(uintptr_t)(C - P);
		return true;
	};

	auto UpChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWFont *Font, LWTextLine *Lines, float Scale, uint32_t TotalLines)->bool {
		if (CurrentLine == 0) {
			if (CurrentPos != 0) {
				CurrentPos = 0;
				CurrentRawPos = 0;
				return true;
			}
			return false;
		}
		if (!Font) CurrentPos = std::min<uint32_t>(CurrentPos, Lines[CurrentLine].m_CharLength);
		else {
			char *C = Lines[CurrentLine].m_Value;
			char *P = Lines[CurrentLine - 1].m_Value;
			LWVector4f CSize = Font->MeasureText(C, CurrentPos, Scale);
			CurrentPos = Font->CharacterAt(P, CSize.z, Scale);
		}
		CurrentLine--;
		char *N = LWText::At(Lines[CurrentLine].m_Value, CurrentPos);
		if (!N) N = Lines[CurrentLine].m_Value + CurrentPos;
		CurrentRawPos = (uint32_t)(N - Lines[CurrentLine].m_Value);
		return true;
	};

	auto DownChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWFont *Font, LWTextLine *Lines, float Scale, uint32_t TotalLines)->bool {
		if (CurrentLine + 1 == TotalLines) {
			if (CurrentPos != Lines[CurrentLine].m_CharLength) {
				CurrentPos = Lines[CurrentLine].m_CharLength;
				CurrentRawPos = Lines[CurrentLine].m_RawLength;
				return true;
			}
			return false;
		}
		if (!Font) CurrentPos = std::min<uint32_t>(CurrentPos, Lines[CurrentLine].m_CharLength);
		else {
			char *C = Lines[CurrentLine].m_Value;
			char *P = Lines[CurrentLine + 1].m_Value;
			LWVector4f CSize = Font->MeasureText(C, CurrentPos, Scale);
			CurrentPos = Font->CharacterAt(P, CSize.z, Scale);
		}
		CurrentLine++;
		char *N = LWText::At(Lines[CurrentLine].m_Value, CurrentPos);
		if (!N) N = Lines[CurrentLine].m_Value + CurrentPos;
		CurrentRawPos = (uint32_t)(N - Lines[CurrentLine].m_Value);
		return true;
	};

	auto MoveCursorBy = [&PrevChar, &NextChar](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, int32_t Distance, LWTextLine *Lines, uint32_t TotalLines)->int32_t {
		int32_t D = 0;
		for (; Distance < 0 && PrevChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance++, D--) {}
		for (; Distance > 0 && NextChar(CurrentPos, CurrentRawPos, CurrentLine, Lines, TotalLines); Distance--, D++) {}
		return D;
	};

	auto CursorDistance = [](uint32_t APos, uint32_t ARawPos, uint32_t ALine, uint32_t BPos, uint32_t BRawPos, uint32_t BLine, int32_t &CharDistance, int32_t &RawDistance, LWTextLine *Lines, uint32_t TotalLines)->bool {

		while (BLine > ALine) {
			CharDistance += BPos;
			RawDistance += BRawPos;
			BLine--;
			BPos = Lines[BLine].m_CharLength + 1;
			BRawPos = Lines[BLine].m_RawLength + 1;
		}
		while (ALine > BLine) {
			CharDistance -= APos;
			RawDistance -= ARawPos;
			ALine--;
			APos = Lines[ALine].m_CharLength + 1;
			ARawPos = Lines[ALine].m_RawLength + 1;
		}
		CharDistance += BPos - APos;
		RawDistance += BRawPos - ARawPos;
		return true;
	};
	char SelectBuffer[512];

	if (Key == LWKey::Return) {
		if (m_SelectCursorLength) ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
		if (m_LineCount >= m_MaxLines || (m_CursorLine + 1) >= LWETEXTINPUT_MAXLINES || m_LineCount >= LWETEXTINPUT_MAXLINES) return false;
		//shift all lines down by 1.
		std::copy_backward(m_Lines + (m_CursorLine + 1), m_Lines + (m_CursorLine + 1) + (m_LineCount - (m_CursorLine + 1)), m_Lines + (m_LineCount + 1));
		uint32_t RawLen = m_Lines[m_CursorLine].m_RawLength;
		uint32_t RemLen = RawLen - m_CursorRawPosition;
		char *CurrentLine = m_Lines[m_CursorLine].m_Value;
		char *NextLine = m_Lines[m_CursorLine + 1].m_Value;
		memcpy(NextLine, CurrentLine + m_CursorRawPosition, sizeof(char)*RemLen + 1);
		CurrentLine[m_CursorRawPosition] = '\0';
		m_Lines[m_CursorLine].m_RawLength = m_CursorRawPosition;
		m_Lines[m_CursorLine].m_CharLength = LWText::TextLength(CurrentLine);
		m_Lines[m_CursorLine + 1].m_RawLength = RemLen;
		m_Lines[m_CursorLine + 1].m_CharLength = LWText::TextLength(NextLine);

		m_CursorLine++;
		m_CursorPosition = 0;
		m_CursorRawPosition = 0;
		m_LineCount++;

		return true;
	} else if (Key == LWKey::Delete) {
		if (!m_SelectCursorLength) {
			if (m_CursorPosition == m_Lines[m_CursorLine].m_CharLength && m_CursorLine + 1 >= m_LineCount) return false;
			m_SelectCursorPosition = m_CursorPosition;
			m_SelectCursorRawPosition = m_CursorRawPosition;
			m_SelectCursorLine = m_CursorLine;
			m_SelectCursorLength = 1;
		}
		uint32_t SCursorBeginPosition = m_SelectCursorPosition;
		uint32_t SCursorBeginRawPosition = m_SelectCursorRawPosition;
		uint32_t SCursorBeginLine = m_SelectCursorLine;
		uint32_t SCursorEndPosition = m_SelectCursorPosition;
		uint32_t SCursorEndRawPosition = m_SelectCursorRawPosition;
		uint32_t SCursorEndLine = m_SelectCursorLine;
		if (m_SelectCursorLength < 0) MoveCursorBy(SCursorBeginPosition, SCursorBeginRawPosition, SCursorBeginLine, m_SelectCursorLength, m_Lines, m_LineCount);
		else MoveCursorBy(SCursorEndPosition, SCursorEndRawPosition, SCursorEndLine, m_SelectCursorLength, m_Lines, m_LineCount);


		char *BeginLine = m_Lines[SCursorBeginLine].m_Value;
		char *EndLine = m_Lines[SCursorEndLine].m_Value;
		uint32_t EndLineRemLen = m_Lines[SCursorEndLine].m_RawLength - SCursorEndRawPosition + 1;
		if (SCursorBeginRawPosition + EndLineRemLen >= LWETEXTINPUT_MAXLINELENGTH) {
			SetSelectRange(0, 0, 0);
			return false; //we do not have enough space to do this operation, so we abort!
		}
		memcpy(BeginLine + SCursorBeginRawPosition, EndLine + SCursorEndRawPosition, sizeof(char)*EndLineRemLen);
		m_Lines[SCursorBeginLine].m_CharLength = SCursorBeginPosition + (m_Lines[SCursorEndLine].m_CharLength - SCursorEndPosition);
		m_Lines[SCursorBeginLine].m_RawLength = SCursorBeginRawPosition + (m_Lines[SCursorEndLine].m_RawLength - SCursorEndRawPosition);
		uint32_t TotalLength = m_CurrentLength;

		if (EndLine != BeginLine) {
			TotalLength = TotalLength - (m_Lines[SCursorBeginLine].m_CharLength - SCursorBeginPosition) - SCursorEndPosition;
			for (uint32_t n = SCursorBeginLine + 1; n < SCursorEndLine - 1; n++) TotalLength -= m_Lines[n].m_CharLength;
			memcpy(&m_Lines[SCursorBeginLine + 1], &m_Lines[SCursorEndLine + 1], sizeof(LWTextLine)*(m_LineCount - SCursorEndLine));
			m_LineCount -= (SCursorEndLine - SCursorBeginLine);
		} else TotalLength = TotalLength - (SCursorEndPosition - SCursorBeginPosition);

		m_CursorPosition = SCursorBeginPosition;
		m_CursorRawPosition = SCursorBeginRawPosition;
		m_CursorLine = SCursorBeginLine;
		m_CurrentLength = TotalLength;
		SetSelectRange(0, 0, 0);
		return true;
	} else if (Key == LWKey::Back) {
		bool Res = false;
		if (!m_SelectCursorLength) Res = PrevChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount);
		bool ResB = ProcessKey(LWKey::Delete, ShiftDown, CtrlDown, Scale);
		return Res || ResB;
	} else if (Key == LWKey::Left || Key == LWKey::Right || Key == LWKey::Up || Key == LWKey::Down) {
		bool Res = false;
		uint32_t pCursorPos = m_CursorPosition;
		uint32_t pCursorRawPos = m_CursorRawPosition;
		uint32_t pCursorLine = m_CursorLine;

		if (Key == LWKey::Left) Res = PrevChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount);
		else if (Key == LWKey::Right) Res = NextChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount);
		else if (Key == LWKey::Up) Res = UpChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Font, m_Lines, m_FontScale*Scale, m_LineCount);
		else if (Key == LWKey::Down) Res = DownChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Font, m_Lines, m_FontScale*Scale, m_LineCount);

		if (Res) {
			if (ShiftDown) {
				if (!m_SelectCursorLength) {
					m_SelectCursorPosition = pCursorPos;
					m_SelectCursorRawPosition = pCursorRawPos;
					m_SelectCursorLine = pCursorLine;
				}
				int32_t Dis = 0;
				int32_t RawDis = 0;
				CursorDistance(m_SelectCursorPosition, m_SelectCursorRawPosition, m_SelectCursorLine, m_CursorPosition, m_CursorRawPosition, m_CursorLine, Dis, RawDis, m_Lines, m_LineCount);
				m_SelectCursorLength = Dis;
			} else m_SelectCursorLength = 0;
		}
		return Res;
	} else if (CtrlDown) {
		if (Key == LWKey::A) {
			m_SelectCursorLine = 0;
			m_SelectCursorPosition = 0;
			m_SelectCursorRawPosition = 0;
			m_SelectCursorLength = m_CurrentLength;
			m_CursorLine = m_LineCount - 1;
			m_CursorPosition = m_Lines[m_CursorLine].m_CharLength;
			m_CursorRawPosition = m_Lines[m_CursorLine].m_RawLength;
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
			if (m_Flag&PasswordField) {
				m_SelectCursorPosition = 0;
				m_SelectCursorRawPosition = 0;
				m_SelectCursorLine = m_CursorLine;
				m_CursorPosition = m_Lines[m_CursorLine].m_CharLength;
				m_CursorRawPosition = m_Lines[m_CursorLine].m_RawLength;
				m_SelectCursorLength = (m_CursorPosition - m_SelectCursorPosition);
				m_Flag |= SelectEnabled;
				return true;
			}
			bool InWord = LWText::GetCharacter(m_Lines[m_CursorLine].m_Value+m_CursorRawPosition)!=' '; //space
			for (;; PrevChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount)) {
				uint32_t c = LWText::GetCharacter(m_Lines[m_CursorLine].m_Value + m_CursorRawPosition);
				if ((c == ' ' && InWord) || m_CursorPosition==0) {
					if(m_CursorPosition!=0) NextChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount);
					m_SelectCursorPosition = m_CursorPosition;
					m_SelectCursorRawPosition = m_CursorRawPosition;
					m_SelectCursorLine = m_CursorLine;
					for (NextChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount); m_CursorPosition != m_Lines[m_CursorLine].m_CharLength; NextChar(m_CursorPosition, m_CursorRawPosition, m_CursorLine, m_Lines, m_LineCount)) {
						uint32_t c = LWText::GetCharacter(m_Lines[m_CursorLine].m_Value + m_CursorRawPosition);
						if (c == ' ') {
							break;
						}
					}
					m_SelectCursorLength = m_CursorPosition - m_SelectCursorPosition;
					break;
				} else if(c!=' ') InWord = true;
			}
			m_Flag |= SelectEnabled;
			return true;
		}
	}
	return false;
}

LWEUITextInput &LWEUITextInput::CalculateBtnPositions(LWVector2f &CopyBtn, LWVector2f &CutBtn, LWVector2f &PasteBtn, LWVector2f &SelectAllBtn, float Scale) {
	char Buffer;
	LWVector2f Pos = m_VisiblePosition + LWVector2f(10.0f, m_VisibleSize.y + 10.0f);
	if (m_SelectCursorLength > 0) {
		CopyBtn = Pos;
		Pos.x += (m_CopyBtn.x + 10.0f)*Scale;
		CutBtn = Pos;
		Pos.x += (m_CutBtn.x + 10.0f)*Scale;
	}
	LWWindow::ReadClipboardText(&Buffer, sizeof(Buffer));
	if (Buffer) {
		PasteBtn = Pos;
		Pos.x += (m_PasteBtn.x + 10.0f)*Scale;
	}
	if (m_CurrentLength > 0) {
		SelectAllBtn = Pos;
		Pos.x += (m_SelectAllBtn.x + 10.0f)*Scale;
	}
	return *this;
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
	char *Line = m_Lines[m_CursorLine].m_Value;
	uint32_t RawLen = m_Lines[m_CursorLine].m_RawLength + 1;
	if (RawLen + CharLen + 1 > sizeof(m_Lines[m_CursorLine].m_Value)) return false;
	std::copy_backward(Line + m_CursorRawPosition, Line + m_CursorRawPosition + (RawLen - m_CursorRawPosition), Line + m_CursorRawPosition + CharLen + (RawLen - m_CursorRawPosition));
	LWText::MakeUTF32To8(&Char, (uint8_t*)Line + m_CursorRawPosition, CharLen);
	
		
	m_CursorPosition++;
	m_CursorRawPosition += CharLen;
	m_Lines[m_CursorLine].m_RawLength += CharLen;
	m_Lines[m_CursorLine].m_CharLength++;
	m_CurrentLength++;

	return true;
}

bool LWEUITextInput::ScrollToCursor(float Scale) {
	if (!m_Font) return false;
	float LineSize = m_Font->GetLineSize()*m_FontScale*Scale;
	LWVector4f LineSizeToCursor = m_Font->MeasureText(m_Lines[m_CursorLine].m_Value, m_CursorPosition, m_FontScale*Scale);
	LWVector2f CursorPos = LWVector2f(LineSizeToCursor.z, m_VisibleSize.y - (LineSize*m_CursorLine + LineSize*0.75f)) + LWVector2f(-m_HorizontalScroll, m_VerticalScroll);
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
	m_SelectCursorLine = m_SelectCursorPosition = m_SelectCursorRawPosition = m_SelectCursorLength = 0;
	m_CursorLine = m_CursorPosition = m_CursorRawPosition = 0;
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
	if (CursorLine >= m_LineCount) CursorLine = m_LineCount - 1;
	if (CursorPosition > m_Lines[m_CursorLine].m_RawLength) CursorPosition = m_Lines[m_CursorLine].m_RawLength;
	m_CursorPosition = CursorPosition;
	m_CursorLine = CursorLine;
	char *Line = m_Lines[m_CursorLine].m_Value;
	char *RawPos = LWText::At(Line, m_CursorPosition);
	m_CursorRawPosition = RawPos ? (uint32_t)(RawPos - Line) : m_CursorPosition;

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
		LWVector4f PasteBtn = m_Font->MeasureText("Paste", m_FontBtnScale);
		LWVector4f CutBtn = m_Font->MeasureText("Cut", m_FontBtnScale);
		LWVector4f SelectAllBtn = m_Font->MeasureText("Select All", m_FontBtnScale);
		m_CopyBtn = LWVector4f(CopyBtn.z - CopyBtn.x, -CopyBtn.w, m_CopyBtn.z, m_CopyBtn.w);
		m_PasteBtn = LWVector4f(PasteBtn.z - PasteBtn.x, -PasteBtn.w, m_PasteBtn.z, m_PasteBtn.w);
		m_CutBtn = LWVector4f(CutBtn.z - CutBtn.x, -CutBtn.w, m_CutBtn.z, m_CutBtn.w);
		m_SelectAllBtn = LWVector4f(SelectAllBtn.z - SelectAllBtn.x, -SelectAllBtn.w, m_SelectAllBtn.z, m_SelectAllBtn.w);
	}
	return *this;
}

LWEUITextInput &LWEUITextInput::SetSelectRange(uint32_t Position, uint32_t Line, int32_t Length) {

	auto NextChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == Lines[CurrentLine].m_CharLength) {
			if (CurrentLine + 1 >= TotalLines) return false;
			CurrentLine++;
			CurrentPos = 0;
			CurrentRawPos = 0;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *N = LWText::Next(C);
		if (!N) N = Lines[CurrentLine].m_Value + Lines[CurrentLine].m_RawLength;
		CurrentPos++;
		CurrentRawPos += (uint32_t)(uintptr_t)(N - C);
		return true;
	};

	auto PrevChar = [](uint32_t &CurrentPos, uint32_t &CurrentRawPos, uint32_t &CurrentLine, LWTextLine *Lines, uint32_t TotalLines)->bool {
		if (CurrentPos == 0) {
			if (CurrentLine == 0) return false;
			CurrentLine--;
			CurrentPos = Lines[CurrentLine].m_CharLength;
			CurrentRawPos = Lines[CurrentLine].m_RawLength;
			return true;
		}
		char *C = Lines[CurrentLine].m_Value + CurrentRawPos;
		char *P = LWText::Prev(C, Lines[CurrentLine].m_Value);
		if (!P) P = Lines[CurrentLine].m_Value;

		CurrentPos--;
		CurrentRawPos -= (uint32_t)(uintptr_t)(C - P);
		return true;
	};

	if (Line >= m_LineCount) Line = m_LineCount - 1;
	if (Position > m_Lines[Line].m_CharLength) Position = m_Lines[Line].m_CharLength;
	m_SelectCursorPosition = Position;
	m_SelectCursorLine = Line;
	char *SelectLine = m_Lines[m_SelectCursorLine].m_Value;
	char *SelectRawPos = LWText::At(SelectLine, m_SelectCursorPosition);
	uint32_t RawPos = SelectRawPos ? (uint32_t)(SelectRawPos - SelectLine) : m_SelectCursorPosition;
	m_SelectCursorRawPosition = RawPos;

	int32_t SelectLen = 0;
	for (; Length < 0 && PrevChar(Position, RawPos, Line, m_Lines, m_LineCount); Length++, SelectLen--) {}
	for (; Length > 0 && NextChar(Position, RawPos, Line, m_Lines, m_LineCount); Length--, SelectLen++) {}
	m_SelectCursorLength = SelectLen;
	if (!m_SelectCursorLength) m_Flag &= ~SelectEnabled;
	else m_Flag |= SelectEnabled;
	return *this;
}

uint32_t LWEUITextInput::GetSelectedText(char *Buffer, uint32_t BufferLen) {
	if (m_Flag&PasswordField) {
		Buffer[0] = '\0';
		return 0;
	}
	uint32_t o = 0;
	char *BLast = Buffer + BufferLen;
	uint32_t sp = m_SelectCursorPosition;
	uint32_t srp = m_SelectCursorRawPosition;
	uint32_t sl = m_SelectCursorLine;
	int32_t len = m_SelectCursorLength;
	if (m_SelectCursorLength<0) {
		sp = m_CursorPosition;
		srp = m_CursorRawPosition;
		sl = m_CursorLine;
		len = -m_SelectCursorLength;
	}
	Buffer[o] = '\0';
	for (int32_t i = 0; i < len && (Buffer+o)!=BLast;) {
		uint32_t n = std::min<uint32_t>(len - i, m_Lines[sl].m_CharLength - sp);
		o += LWText::Copy(m_Lines[sl].m_Value + srp, n, Buffer + o, BufferLen - o);
		if ((i + n != len)) {
			if(Buffer+o!=BLast) Buffer[o++] = '\n';
			if(Buffer+o!=BLast) Buffer[o] = '\0';
			n++;
		}
		sp = 0;
		srp = 0;
		sl++;
		if (sl >= m_LineCount) break;
		i += n;
	}
	return o;
}

const LWTextLine *LWEUITextInput::GetLine(uint32_t Line) const {
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

uint32_t LWEUITextInput::GetCursorPosition(void) const {
	return m_CursorPosition;
}

uint32_t LWEUITextInput::GetCursorRawPosition(void) const {
	return m_CursorRawPosition;
}

uint32_t LWEUITextInput::GetCursorLine(void) const {
	return m_CursorLine;
}

uint32_t LWEUITextInput::GetLineCount(void) const {
	return m_LineCount;
}

uint32_t LWEUITextInput::GetSelectCusorPosition(void) const {
	return m_SelectCursorPosition;
}

uint32_t LWEUITextInput::GetSelectCursorRawPosition(void) const {
	return m_SelectCursorRawPosition;
}

uint32_t LWEUITextInput::GetSelectCusorLine(void) const {
	return m_SelectCursorLine;
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

const char *LWEUITextInput::GetAllowedCharacters(void) const {
	return m_AllowedCharacters;
}

LWEUITextInput::LWEUITextInput(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag) {
	m_Lines[0].m_Value[0] = '\0';
	m_Lines[0].m_RawLength = m_Lines[0].m_CharLength = 0;
	m_AllowedCharacters[0] = '\0';
}

LWEUITextInput::~LWEUITextInput() {}
