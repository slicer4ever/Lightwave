#include "LWCore/LWText.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>
#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window){

	XEvent *Evnt = (XEvent*)MessageData;
	LWVector2i WndSize = Window->GetSize();
	if (MessageID == MotionNotify || MessageID == ButtonRelease || MessageID == ButtonPress){
		XMotionEvent *MEvnt = (XMotionEvent*)Evnt;
		m_Position = LWVector2i(MEvnt->x, WndSize.y-MEvnt->y);
		if(MessageID==ButtonRelease || MessageID==ButtonPress){
			//Refine which button changed!
			XButtonEvent *BEvnt = (XButtonEvent*)Evnt;
			if (BEvnt->button == Button1) m_CurrState = (m_CurrState&~(uint32_t)LWMouseKey::Left) | (MessageID == ButtonPress ? (uint32_t)LWMouseKey::Left : 0);
			else if (BEvnt->button == Button2) m_CurrState = (m_CurrState&~(uint32_t)LWMouseKey::Middle) | (MessageID == ButtonPress ? (uint32_t)LWMouseKey::Middle : 0);
			else if (BEvnt->button == Button3) m_CurrState = (m_CurrState&~(uint32_t)LWMouseKey::Right) | (MessageID == ButtonPress ? (uint32_t)LWMouseKey::Right : 0);
			else if (BEvnt->button == X11_MOUSE_X1) m_CurrState = (m_CurrState&~(uint32_t)LWMouseKey::X1) | (MessageID == ButtonPress ? (uint32_t)LWMouseKey::X1 : 0);
			else if (BEvnt->button == X11_MOUSE_X2) m_CurrState = (m_CurrState&~(uint32_t)LWMouseKey::X2) | (MessageID == ButtonPress ? (uint32_t)LWMouseKey::X2 : 0);
			else if (BEvnt->button == Button4) m_Scroll = X11_MOUSE_SCROLL_INCREMENT;
			else if (BEvnt->button == Button5) m_Scroll = -X11_MOUSE_SCROLL_INCREMENT;
		}
		return true;
	}
	return false;
}
#pragma endregion

#pragma region LWKeyboard
bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *){
	char Buffer[32];

	auto Translate = [](uint32_t Key) -> LWKey{

		switch (Key) {
		case XK_a: return LWKey::A;
		case XK_b: return LWKey::B;
		case XK_c: return LWKey::C;
		case XK_d: return LWKey::D;
		case XK_e: return LWKey::E;
		case XK_f: return LWKey::F;
		case XK_g: return LWKey::G;
		case XK_h: return LWKey::H;
		case XK_i: return LWKey::I;
		case XK_j: return LWKey::J;
		case XK_k: return LWKey::K;
		case XK_l: return LWKey::L;
		case XK_m: return LWKey::M;
		case XK_n: return LWKey::N;
		case XK_o: return LWKey::O;
		case XK_p: return LWKey::P;
		case XK_q: return LWKey::Q;
		case XK_r: return LWKey::R;
		case XK_s: return LWKey::S;
		case XK_t: return LWKey::T;
		case XK_u: return LWKey::U;
		case XK_v: return LWKey::V;
		case XK_w: return LWKey::W;
		case XK_x: return LWKey::X;
		case XK_y: return LWKey::Y;
		case XK_Z: return LWKey::Z;
		case XK_0: return LWKey::Key0;
		case XK_1: return LWKey::Key1;
		case XK_2: return LWKey::Key2;
		case XK_3: return LWKey::Key3;
		case XK_4: return LWKey::Key4;
		case XK_5: return LWKey::Key5;
		case XK_6: return LWKey::Key6;
		case XK_7: return LWKey::Key7;
		case XK_8: return LWKey::Key8;
		case XK_9: return LWKey::Key9;
		case XK_Tab: return LWKey::Tab;
		case XK_Caps_Lock: return LWKey::CapLock;
		case XK_Shift_L: return LWKey::LShift;
		case XK_Control_L: return LWKey::LCtrl;
		case XK_Alt_L: return LWKey::LAlt;
		case XK_space: return LWKey::Space;
		case XK_Alt_R: return LWKey::RAlt;
		case XK_Menu: return LWKey::RMenu;
		case XK_Control_R: return LWKey::RCtrl;
		case XK_Shift_R: return LWKey::RShift;
		case XK_Return: return LWKey::Return;
		case XK_BackSpace: return LWKey::Back;
		case XK_comma: return LWKey::OEM_COMMA;
		case XK_minus: return LWKey::OEM_MINUS;
		case XK_plus: return LWKey::OEM_PLUS;
		case XK_period: return LWKey::OEM_PERIOD;
		case XK_grave: return LWKey::OEM_0;
		case XK_slash: return LWKey::OEM_1;
		case XK_semicolon: return LWKey::OEM_2;
		case XK_apostrophe: return LWKey::OEM_3;
		case XK_bracketleft: return LWKey::OEM_4;
		case XK_bracketright: return LWKey::OEM_5;
		case XK_backslash: return LWKey::OEM_6;
		case XK_Left: return LWKey::Left;
		case XK_Right: return LWKey::Right;
		case XK_Up: return LWKey::Up;
		case XK_Down: return LWKey::Down;
		case XK_Insert: return LWKey::Insert;
		case XK_Delete: return LWKey::Delete;
		case XK_Home: return LWKey::Home;
		case XK_End: return LWKey::End;
		case XK_Page_Up: return LWKey::PageUp;
		case XK_Page_Down: return LWKey::PageDown;
		case XK_Sys_Req: return LWKey::PrintScreen;
		case XK_Scroll_Lock: return LWKey::ScrollLock;
		case XK_Pause: return LWKey::Pause;
		case XK_Num_Lock: return LWKey::NumLock;
		case XK_KP_Insert: return LWKey::Num0;
		case XK_KP_0: return LWKey::Num0;
		case XK_KP_End: return LWKey::Num1;
		case XK_KP_1: return LWKey::Num1;
		case XK_KP_Down: return LWKey::Num2;
		case XK_KP_2: return LWKey::Num2;
		case XK_KP_Page_Down: return LWKey::Num3;
		case XK_KP_3: return LWKey::Num3;
		case XK_KP_Left: return LWKey::Num4;
		case XK_KP_4: return LWKey::Num4;
		case XK_KP_Begin: return LWKey::Num5;
		case XK_KP_5: return LWKey::Num5;
		case XK_KP_Right: return LWKey::Num6;
		case XK_KP_6: return LWKey::Num6;
		case XK_KP_Home: return LWKey::Num7;
		case XK_KP_7: return LWKey::Num7;
		case XK_KP_Up: return LWKey::Num8;
		case XK_KP_8: return LWKey::Num8;
		case XK_KP_Page_Up: return LWKey::Num9;
		case XK_KP_9: return LWKey::Num9;
		case XK_KP_Decimal: return LWKey::NumDecimal;
		case XK_KP_Enter: return LWKey::NumReturn;
		case XK_KP_Add: return LWKey::NumAdd;
		case XK_KP_Subtract: return LWKey::NumMinus;
		case XK_KP_Multiply: return LWKey::NumMultiply;
		case XK_KP_Divide: return LWKey::NumDivide;
		case XK_Escape: return LWKey::Esc;
		case XK_F1: return LWKey::F1;
		case XK_F2: return LWKey::F2;
		case XK_F3: return LWKey::F3;
		case XK_F4: return LWKey::F4;
		case XK_F5: return LWKey::F5;
		case XK_F6: return LWKey::F6;
		case XK_F7: return LWKey::F7;
		case XK_F8: return LWKey::F8;
		case XK_F9: return LWKey::F9;
		case XK_F10: return LWKey::F10;
		case XK_F11: return LWKey::F11;
		case XK_F12: return LWKey::F12;
		case XK_F13: return LWKey::F13;
		case XK_F14: return LWKey::F14;
		case XK_F15: return LWKey::F15;
		case XK_F16: return LWKey::F16;
		case XK_F17: return LWKey::F17;
		case XK_F18: return LWKey::F18;
		case XK_F19: return LWKey::F19;
	}
	return LWKey::Unknown;
	};
	
	static KeySym *KeyMapping = nullptr; //no good way to clean up this memory, so it'll be left to the os on app termination.
	static int32_t MinKeycodes = 0;
	static int32_t MaxKeycodes = 0;
	static int32_t KeycodeCount = 0;
	auto SetKeyState = [this](uint32_t Key, bool Up) {
		uint32_t Word = Key / (sizeof(uint32_t) * 8);
		if (Word >= (sizeof(m_CurrState) / sizeof(uint32_t))) return 0;
		uint32_t Bit = 1 << (Key % (sizeof(uint32_t) * 8));
		if (Up) m_CurrState[Word] &= ~Bit;
		else {
			if ((m_PrevState[Word] & Bit) != 0) m_PrevState[Word] &= ~Bit;
			m_CurrState[Word] |= Bit;
		}
		if (m_KeyChangeCount < MaxKeyChanges) {
			m_KeyChanges[m_KeyChangeCount] = Key;
			m_KeyStates[m_KeyChangeCount] = !Up;
			m_KeyChangeCount++;
		}

		return 1;
	};

	LWKey TransKey = LWKey::Unknown;
	bool Up = MessageID == KeyRelease;
	if(MessageID==KeyPress || MessageID==KeyRelease){
		XKeyEvent *KEvnt = (XKeyEvent*)MessageData;
		if (!KeyMapping) {
			XDisplayKeycodes(KEvnt->display, &MinKeycodes, &MaxKeycodes);
			KeyMapping = XGetKeyboardMapping(KEvnt->display, MinKeycodes, MaxKeycodes-MinKeycodes, &KeycodeCount);
		}
		TransKey = Translate((uint32_t)KeyMapping[(KEvnt->keycode-MinKeycodes)*KeycodeCount]);
		if (MessageID == KeyPress) {
			KeySym KeyS;
			uint32_t n = XLookupString(KEvnt, Buffer, sizeof(Buffer), &KeyS, nullptr);
			if (n && m_CharPressed < MaxKeyChanges) m_CharInputs[m_CharPressed++] = LWText::GetCharacter(Buffer);
		};
	}
	if (TransKey == LWKey::Unknown) return false;
	SetKeyState((uint32_t)TransKey, Up);
	return true;
}
#pragma endregion

#pragma region LWTouch

bool LWTouch::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

#pragma endregion

#pragma region LWGamePad

LWInputDevice &LWGamePad::Update(LWWindow *, uint64_t) {
	return *this;
}

bool LWGamePad::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}
#pragma endregion

#pragma region LWAccelerometer

bool LWAccelerometer::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

LWInputDevice &LWAccelerometer::Update(LWWindow *, uint64_t) {
	return *this;
}
#pragma endregion

#pragma region LWGyroscope

bool LWGyroscope::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

LWInputDevice &LWGyroscope::Update(LWWindow *, uint64_t) {
	return *this;
}

#pragma endregion