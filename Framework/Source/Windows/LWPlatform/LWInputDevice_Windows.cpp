#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWLogger.h"
#include <Xinput.h>
#include <algorithm>
#include <iostream>

#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t , LWWindow *Window){
	MSG *M = (MSG*)MessageData;
	LWVector2i WndSize = Window->GetSize();
	if(MessageID==WM_MOUSEMOVE || MessageID==WM_LBUTTONUP || MessageID==WM_LBUTTONDOWN || MessageID==WM_RBUTTONDOWN || MessageID==WM_RBUTTONUP || MessageID==WM_MBUTTONDOWN || MessageID==WM_MBUTTONUP || MessageID==WM_MOUSEWHEEL || MessageID==WM_XBUTTONDOWN || MessageID==WM_XBUTTONUP){
		m_CurrState = (((M->wParam & 0x1) != 0) ? (uint32_t)LWMouseKey::Left : 0) | (((M->wParam & 0x2) != 0) ? (uint32_t)LWMouseKey::Right : 0) | (((M->wParam & 0x10) != 0) ? (uint32_t)LWMouseKey::Middle : 0) | (((M->wParam & 0x20) != 0) ? (uint32_t)LWMouseKey::X1 : 0) | (((M->wParam & 0x40) != 0) ? (uint32_t)LWMouseKey::X2 : 0);
		if (MessageID == WM_MOUSEWHEEL) m_Scroll = GET_WHEEL_DELTA_WPARAM(M->wParam);
		else m_Position = LWVector2i(M->lParam & 0xFFFF, WndSize.y - ((M->lParam & 0xFFFF0000) >> 16));
		return true;
	}
	return false;
}

#pragma endregion

#pragma region LWKeyboard

bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t , LWWindow *){
	auto Translate = [](uint32_t Key) -> LWKey{
		switch (Key) {
		case 'A': return LWKey::A;
		case 'B': return LWKey::B;
		case 'C': return LWKey::C;
		case 'D': return LWKey::D;
		case 'E': return LWKey::E;
		case 'F': return LWKey::F;
		case 'G': return LWKey::G;
		case 'H': return LWKey::H;
		case 'I': return LWKey::I;
		case 'J': return LWKey::J;
		case 'K': return LWKey::K;
		case 'L': return LWKey::L;
		case 'M': return LWKey::M;
		case 'N': return LWKey::N;
		case 'O': return LWKey::O;
		case 'P': return LWKey::P;
		case 'Q': return LWKey::Q;
		case 'R': return LWKey::R;
		case 'S': return LWKey::S;
		case 'T': return LWKey::T;
		case 'U': return LWKey::U;
		case 'V': return LWKey::V;
		case 'W': return LWKey::W;
		case 'X': return LWKey::X;
		case 'Y': return LWKey::Y;
		case 'Z': return LWKey::Z;
		case '0': return LWKey::Key0;
		case '1': return LWKey::Key1;
		case '2': return LWKey::Key2;
		case '3': return LWKey::Key3;
		case '4': return LWKey::Key4;
		case '5': return LWKey::Key5;
		case '6': return LWKey::Key6;
		case '7': return LWKey::Key7;
		case '8': return LWKey::Key8;
		case '9': return LWKey::Key9;
		case VK_TAB: return LWKey::Tab;
		case VK_CAPITAL: return LWKey::CapLock;
		case VK_SHIFT: return LWKey::LShift;
		case VK_MENU: return LWKey::LAlt;
		case VK_CONTROL: return LWKey::LCtrl;
		case VK_LSHIFT: return LWKey::LShift;
		case VK_LCONTROL: return LWKey::LCtrl;
		case VK_LWIN: return LWKey::LMenu;
		case VK_LMENU: return LWKey::LAlt;
		case VK_SPACE: return LWKey::Space;
		case VK_RMENU: return LWKey::RAlt;
		case VK_APPS: return LWKey::RMenu;
		case VK_RCONTROL: return LWKey::RCtrl;
		case VK_RSHIFT: return LWKey::RShift;
		case VK_RETURN: return LWKey::Return;
		case VK_BACK: return LWKey::Back;
		case VK_OEM_COMMA: return LWKey::OEM_COMMA;
		case VK_OEM_MINUS: return LWKey::OEM_MINUS;
		case VK_OEM_PLUS: return LWKey::OEM_PLUS;
		case VK_OEM_PERIOD: return LWKey::OEM_PERIOD;
		case VK_OEM_3: return LWKey::OEM_0;
		case VK_OEM_2: return LWKey::OEM_1;
		case VK_OEM_1: return LWKey::OEM_2;
		case VK_OEM_7: return LWKey::OEM_3;
		case VK_OEM_4: return LWKey::OEM_4;
		case VK_OEM_6: return LWKey::OEM_5;
		case VK_OEM_5: return LWKey::OEM_6;
		case VK_LEFT: return LWKey::Left;
		case VK_RIGHT: return LWKey::Right;
		case VK_UP: return LWKey::Up;
		case VK_DOWN: return LWKey::Down;
		case VK_INSERT: return LWKey::Insert;
		case VK_DELETE: return LWKey::Delete;
		case VK_HOME: return LWKey::Home;
		case VK_END: return LWKey::End;
		case VK_PRIOR: return LWKey::PageUp;
		case VK_NEXT: return LWKey::PageDown;
		case VK_SNAPSHOT: return LWKey::PrintScreen;
		case VK_SCROLL: return LWKey::ScrollLock;
		case VK_PAUSE: return LWKey::Pause;
		case VK_NUMLOCK: return LWKey::NumLock;
		case VK_NUMPAD0: return LWKey::Num0;
		case VK_NUMPAD1: return LWKey::Num1;
		case VK_NUMPAD2: return LWKey::Num2;
		case VK_NUMPAD3: return LWKey::Num3;
		case VK_NUMPAD4: return LWKey::Num4;
		case VK_NUMPAD5: return LWKey::Num5;
		case VK_NUMPAD6: return LWKey::Num6;
		case VK_NUMPAD7: return LWKey::Num7;
		case VK_NUMPAD8: return LWKey::Num8;
		case VK_NUMPAD9: return LWKey::Num9;
		case VK_DECIMAL: return LWKey::NumDecimal;
		case VK_ADD: return LWKey::NumAdd;
		case VK_SUBTRACT: return LWKey::NumMinus;
		case VK_MULTIPLY: return LWKey::NumMultiply;
		case VK_DIVIDE: return LWKey::NumDivide;
		case VK_ESCAPE: return LWKey::Esc;
		case VK_F1: return LWKey::F1;
		case VK_F2: return LWKey::F2;
		case VK_F3: return LWKey::F3;
		case VK_F4: return LWKey::F4;
		case VK_F5: return LWKey::F5;
		case VK_F6: return LWKey::F6;
		case VK_F7: return LWKey::F7;
		case VK_F8: return LWKey::F8;
		case VK_F9: return LWKey::F9;
		case VK_F10: return LWKey::F10;
		case VK_F11: return LWKey::F11;
		case VK_F12: return LWKey::F12;
		case VK_F13: return LWKey::F13;
		case VK_F14: return LWKey::F14;
		case VK_F15: return LWKey::F15;
		case VK_F16: return LWKey::F16;
		case VK_F17: return LWKey::F17;
		case VK_F18: return LWKey::F18;
		case VK_F19: return LWKey::F19;
		case VK_MEDIA_PLAY_PAUSE: return LWKey::MediaPlayPause;
		case VK_MEDIA_NEXT_TRACK: return LWKey::MediaNext;
		case VK_MEDIA_PREV_TRACK: return LWKey::MediaPrev;
		case VK_VOLUME_MUTE: return LWKey::VolumeMute;
		case VK_VOLUME_DOWN: return LWKey::VolumeDown;
		case VK_VOLUME_UP: return LWKey::VolumeUp;
		case VK_LAUNCH_MAIL: return LWKey::Mail;
		}
		return LWKey::Unknown;
	};
	MSG *M = (MSG*)MessageData;
	LWKey Key = LWKey::Unknown;
	bool Up = MessageID == WM_KEYUP || MessageID==WM_SYSKEYUP;
	
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

	if (MessageID==WM_CHAR || MessageID==WM_SYSCHAR) {
		if (m_CharPressed >= MaxKeyChanges) return false;
		m_CharInputs[m_CharPressed++] = (uint32_t)M->wParam;
		return true;
	}

	if (MessageID == WM_KEYUP || MessageID == WM_KEYDOWN || MessageID == WM_SYSKEYUP || MessageID == WM_SYSKEYDOWN) {
		Key = Translate((uint32_t)M->wParam);
		LWLogWarnIf<64>(Key!=LWKey::Unknown, "Unknown Keycode: {}", Key);
	}
	if (Key == LWKey::Unknown) return false;
	SetKeyState((uint32_t)Key, Up);
	TranslateMessage(M);
	return true;
}
#pragma endregion

#pragma region LWTouch

bool LWTouch::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	MSG *M = (MSG*)MessageData;
	LWVector2i WndSize = Window->GetSize();

	if (MessageID == WM_MOUSEMOVE || MessageID == WM_LBUTTONUP || MessageID == WM_LBUTTONDOWN) {
		//m_CurrState = ((M->wParam & 0x1) != 0) ? (uint32_t)LWMouseKey::Left : 0 | ((M->wParam & 0x2) != 0) ? (uint32_t)LWMouseKey::Right : 0 | ((M->wParam & 0x10) != 0) ? (uint32_t)LWMouseKey::Middle : 0 | ((M->wParam & 0x20) != 0) ? (uint32_t)LWMouseKey::X1 : 0 | ((M->wParam & 0x40) != 0) ? (uint32_t)LWMouseKey::X2 : 0;
		LWVector2i Pos = LWVector2i(M->lParam & 0xFFFF, WndSize.y - ((M->lParam & 0xFFFF0000) >> 16));
		if (m_PointCount == 1) {
			if (m_Points[0].m_State != LWTouchPoint::UP) {
				m_Points[0].m_State = (MessageID == WM_MOUSEMOVE ? LWTouchPoint::MOVED : (MessageID == WM_LBUTTONUP ? LWTouchPoint::UP : m_Points[0].m_State));
			}
		}
		if (MessageID == WM_LBUTTONDOWN) {
			m_Points[0].m_State = LWTouchPoint::DOWN;
			m_Points[0].m_DownTime = lCurrentTime;
			m_Points[0].m_InitPosition = Pos;
			m_Points[0].m_PrevPosition = Pos;
			m_Points[0].m_Size = 1.0f;
			m_PointCount = 1;
		}
		m_Points[0].m_Position = Pos;
		return true;
	}
	return false; 
}

#pragma endregion

#pragma region LWGamePad
LWInputDevice &LWGamePad::Update(LWWindow *, uint64_t ) {
	m_PrevButtons = m_Buttons;
	XINPUT_STATE s;
	memset(&s, 0, sizeof(XINPUT_STATE));
	if (XInputGetState(0, &s) != ERROR_SUCCESS) return *this;
	uint32_t B = 0;

	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_A ? LWGamePad::A : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_B ? LWGamePad::B : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_X ? LWGamePad::X : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_Y ? LWGamePad::Y : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_LEFT ? LWGamePad::Left : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_RIGHT ? LWGamePad::Right : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_UP ? LWGamePad::Up : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_DPAD_DOWN ? LWGamePad::Down : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_LEFT_SHOULDER ? LWGamePad::L1 : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_RIGHT_SHOULDER ? LWGamePad::R1 : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_LEFT_THUMB ? LWGamePad::L3 : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_RIGHT_THUMB ? LWGamePad::R3 : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_BACK ? LWGamePad::Select : 0;
	B |= s.Gamepad.wButtons&XINPUT_GAMEPAD_START ? LWGamePad::Start : 0;
	B |= s.Gamepad.bLeftTrigger > 200 ? LWGamePad::L2 : 0;
	B |= s.Gamepad.bRightTrigger > 200 ? LWGamePad::R2 : 0;

	m_Buttons = B;
	float normLX = std::max<float>(-1.0f, (float)s.Gamepad.sThumbLX / 32767.0f);
	float normLY = std::max<float>(-1.0f, (float)s.Gamepad.sThumbLY / 32767.0f);
	float normRX = std::max<float>(-1.0f, (float)s.Gamepad.sThumbRX / 32767.0f);
	float normRY = std::max<float>(-1.0f, (float)s.Gamepad.sThumbRY / 32767.0f);
	
	m_LeftAxis = LWVector2f(normLX, normLY);
	m_RightAxis = LWVector2f(normRX, normRY);
	m_BumperStrength = LWVector2f((float)s.Gamepad.bLeftTrigger / 255.0f, (float)s.Gamepad.bRightTrigger / 255.0f);

	return *this;
}

bool LWGamePad::ProcessSystemMessage(uint32_t , void *, uint64_t , LWWindow *) {
	return false;
}
#pragma endregion

#pragma region LWAccelerometer

bool LWAccelerometer::ProcessSystemMessage(uint32_t, void *, uint64_t , LWWindow *) {
	return false;
}

LWInputDevice &LWAccelerometer::Update(LWWindow *, uint64_t) {
	return *this;
}
#pragma endregion

#pragma region LWGyroscope

bool LWGyroscope::ProcessSystemMessage(uint32_t , void *, uint64_t, LWWindow *) {
	return false;
}

LWInputDevice &LWGyroscope::Update(LWWindow *, uint64_t) {
	return *this;
}

#pragma endregion