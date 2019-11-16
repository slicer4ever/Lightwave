#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>


#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window){
	LWWindowContext Con = Window->GetContext();
	NSEvent *E = (__bridge NSEvent*)MessageData;
	LWVector2i WndSize = Window->GetSize();
    if(MessageID==NSMouseMoved || MessageID==NSLeftMouseDragged || MessageID==NSRightMouseDragged || MessageID==NSOtherMouseDragged || MessageID == NSLeftMouseDown || MessageID==NSLeftMouseUp || MessageID==NSRightMouseDown || MessageID==NSRightMouseUp || MessageID==NSOtherMouseDown || MessageID==NSOtherMouseUp || MessageID==NSScrollWheel){
        NSRect WndFrame = [Con.Window frame];
        NSRect ViewFrame = [[Con.Window contentView] frame];
        
        NSUInteger State = [NSEvent pressedMouseButtons];
        NSPoint Pos = [NSEvent mouseLocation];
        m_CurrState = ((State&1)!=0?(uint32_t)LWMouseKey::Left:0) | ((State&2)!=0?(uint32_t)LWMouseKey::Right:0) | ((State&4)!=0?(uint32_t)LWMouseKey::Middle:0) | ((State&8)!=0?(uint32_t)LWMouseKey::X1:0)| ((State&16)!=0?(uint32_t)LWMouseKey::X2:0);
        m_Position = LWVector2i((uint32_t)(Pos.x-WndFrame.origin.x),WndSize.y-((uint32_t)(ViewFrame.size.height-(Pos.y-WndFrame.origin.y))));
        if(MessageID==NSScrollWheel) m_Scroll = (uint32_t)[E deltaY];
        return true;
    }
	return false;
}

#pragma endregion

#pragma region LWKeyboard


bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *){
	auto TranslateSpecialKeys = [](uint32_t Key)->LWKey {
		switch(Key){
		case kVK_Tab: return LWKey::Tab;
		case kVK_CapsLock: return LWKey::CapLock;
		case kVK_Shift: return LWKey::LShift;
		case kVK_Command: return LWKey::LCtrl;
		case kVK_Control: return LWKey::LCtrl;
		case kVK_Option: return LWKey::LAlt;
		case kVK_Space: return LWKey::Space;
		case kVK_RightOption: return LWKey::RAlt;
		case kVK_RightControl: return LWKey::RCtrl;
		case kVK_RightShift: return LWKey::RShift;
		case kVK_Return: return LWKey::Return;
		case kVK_Delete: return LWKey::Back;
		case kVK_LeftArrow: return LWKey::Left;
		case kVK_RightArrow: return LWKey::Right;
		case kVK_UpArrow: return LWKey::Up;
		case kVK_DownArrow: return LWKey::Down;
		case kVK_ForwardDelete: return LWKey::Delete;
		case kVK_Home: return LWKey::Home;
		case kVK_End: return LWKey::End;
		case kVK_PageUp: return LWKey::PageUp;
		case kVK_PageDown: return LWKey::PageDown;
		case kVK_Escape: return LWKey::Esc;
		case kVK_F1: return LWKey::F1;
		case kVK_F2: return LWKey::F2;
		case kVK_F3: return LWKey::F3;
		case kVK_F4: return LWKey::F4;
		case kVK_F5: return LWKey::F5;
		case kVK_F6: return LWKey::F6;
		case kVK_F7: return LWKey::F7;
		case kVK_F8: return LWKey::F8;
		case kVK_F9: return LWKey::F9;
		case kVK_F10: return LWKey::F10;
		case kVK_F11: return LWKey::F11;
		case kVK_F12: return LWKey::F12;
		case kVK_F13: return LWKey::F13;
		case kVK_F14: return LWKey::F14;
		case kVK_F15: return LWKey::F15;
		case kVK_VolumeUp: return LWKey::VolumeUp;
		case kVK_VolumeDown: return LWKey::VolumeDown;
		}
		return LWKey::Unknown;
	};
	
	auto Translate = [](uint32_t Key) -> LWKey{

		switch (Key) {
		case kVK_ANSI_A: return LWKey::A;
		case kVK_ANSI_B: return LWKey::B;
		case kVK_ANSI_C: return LWKey::C;
		case kVK_ANSI_D: return LWKey::D;
		case kVK_ANSI_E: return LWKey::E;
		case kVK_ANSI_F: return LWKey::F;
		case kVK_ANSI_G: return LWKey::G;
		case kVK_ANSI_H: return LWKey::H;
		case kVK_ANSI_I: return LWKey::I;
		case kVK_ANSI_J: return LWKey::J;
		case kVK_ANSI_K: return LWKey::K;
		case kVK_ANSI_L: return LWKey::L;
		case kVK_ANSI_M: return LWKey::M;
		case kVK_ANSI_N: return LWKey::N;
		case kVK_ANSI_O: return LWKey::O;
		case kVK_ANSI_P: return LWKey::P;
		case kVK_ANSI_Q: return LWKey::Q;
		case kVK_ANSI_R: return LWKey::R;
		case kVK_ANSI_S: return LWKey::S;
		case kVK_ANSI_T: return LWKey::T;
		case kVK_ANSI_U: return LWKey::U;
		case kVK_ANSI_V: return LWKey::V;
		case kVK_ANSI_X: return LWKey::X;
		case kVK_ANSI_Y: return LWKey::Y;
		case kVK_ANSI_Z: return LWKey::Z;
		case kVK_ANSI_W: return LWKey::W;
		case kVK_ANSI_0: return LWKey::Key0;
		case kVK_ANSI_1: return LWKey::Key1;
		case kVK_ANSI_2: return LWKey::Key2;
		case kVK_ANSI_3: return LWKey::Key3;
		case kVK_ANSI_4: return LWKey::Key4;
		case kVK_ANSI_5: return LWKey::Key5;
		case kVK_ANSI_6: return LWKey::Key6;
		case kVK_ANSI_7: return LWKey::Key7;
		case kVK_ANSI_8: return LWKey::Key8;
		case kVK_ANSI_9: return LWKey::Key9;
		case kVK_ANSI_Comma: return LWKey::OEM_COMMA;
		case kVK_ANSI_Minus: return LWKey::OEM_MINUS;
		case kVK_ANSI_Equal: return LWKey::OEM_PLUS;
		case kVK_ANSI_Period: return LWKey::OEM_PERIOD;
		case kVK_ANSI_Grave: return LWKey::OEM_0;
		case kVK_ANSI_Slash: return LWKey::OEM_1;
		case kVK_ANSI_Semicolon: return LWKey::OEM_2;
		case kVK_ANSI_Quote: return LWKey::OEM_3;
		case kVK_ANSI_LeftBracket: return LWKey::OEM_4;
		case kVK_ANSI_RightBracket: return LWKey::OEM_5;
		case kVK_ANSI_Backslash: return LWKey::OEM_6;
		case kVK_ANSI_Keypad0: return LWKey::Num0;
		case kVK_ANSI_Keypad1: return LWKey::Num1;
		case kVK_ANSI_Keypad2: return LWKey::Num2;
		case kVK_ANSI_Keypad3: return LWKey::Num3;
		case kVK_ANSI_Keypad4: return LWKey::Num4;
		case kVK_ANSI_Keypad5: return LWKey::Num5;
		case kVK_ANSI_Keypad6: return LWKey::Num6;
		case kVK_ANSI_Keypad7: return LWKey::Num7;
		case kVK_ANSI_Keypad8: return LWKey::Num8;
		case kVK_ANSI_Keypad9: return LWKey::Num9;
		case kVK_ANSI_KeypadDecimal: return LWKey::NumDecimal;
		case kVK_ANSI_KeypadEnter: return LWKey::NumReturn;
		case kVK_ANSI_KeypadPlus: return LWKey::NumAdd;
		case kVK_ANSI_KeypadMinus: return LWKey::NumMinus;
		case kVK_ANSI_KeypadMultiply: return LWKey::NumMultiply;
		case kVK_ANSI_KeypadDivide: return LWKey::NumDivide;
		case kVK_ANSI_KeypadEquals: return LWKey::NumEqual;
		}
		return LWKey::Unknown;
	};
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
    
	NSEvent *E = (__bridge NSEvent*)MessageData;
    uint32_t Count = 0;
	if (MessageID == NSKeyUp || MessageID == NSKeyDown) {
		bool Up = MessageID == NSKeyUp;
		uint32_t Key = (uint32_t)TranslateSpecialKeys((uint32_t)[E keyCode]);
		if (Key == (uint32_t)LWKey::Unknown) {
			Key = (uint32_t)Translate((uint32_t)[E keyCode]);
			if (!Up && m_CharPressed < MaxKeyChanges && !ButtonDown(LWKey::LCtrl)) { //LCtrl is a bit of a hack to allow us to do mac+x commands without also sending an x as well(which can be probmatic for a text editor.)
				const char *S = [[E characters] UTF8String];
				if (S && *S) m_CharInputs[m_CharPressed++] = LWText::GetCharacter(S);
			}
		}
		Count += SetKeyState(Key, Up);
    }else if(MessageID==NSFlagsChanged){
        NSUInteger CurentState = [E modifierFlags];
        if(!ButtonDown(LWKey::CapLock) && (CurentState&NSAlphaShiftKeyMask)!=0) SetKeyState((uint32_t)LWKey::CapLock, false);
        else if(ButtonDown(LWKey::CapLock) && (CurentState&NSAlphaShiftKeyMask)==0) SetKeyState((uint32_t)LWKey::CapLock, true);
        
        if(!ButtonDown(LWKey::LShift) && (CurentState&NSShiftKeyMask)!=0) SetKeyState((uint32_t)LWKey::LShift, false);
        else if(ButtonDown(LWKey::LShift) && (CurentState&NSShiftKeyMask)==0) SetKeyState((uint32_t)LWKey::LShift, true);
        
        if(!ButtonDown(LWKey::LCtrl) && (CurentState&(NSControlKeyMask|NSCommandKeyMask))!=0) SetKeyState((uint32_t)LWKey::LCtrl, false);
        else if(ButtonDown(LWKey::LCtrl) && (CurentState&(NSControlKeyMask | NSCommandKeyMask))==0) SetKeyState((uint32_t)LWKey::LCtrl, true);
        
        if(!ButtonDown(LWKey::LAlt) && (CurentState&NSAlternateKeyMask)!=0) SetKeyState((uint32_t)LWKey::LAlt, false);
        else if(ButtonDown(LWKey::LAlt) && (CurentState&NSAlternateKeyMask)==0) SetKeyState((uint32_t)LWKey::LAlt, true);
        
    }
	return Count!=0;

}
#pragma endregion

#pragma region LWTouch

bool LWTouch::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

#pragma endregion

#pragma region LWGamePad

LWInputDevice &LWGamePad::Update(LWWindow *Window, uint64_t lCurrentTime) {
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

LWInputDevice &LWAccelerometer::Update(LWWindow *Window, uint64_t lCurrentTime) {
	return *this;
}
#pragma endregion

#pragma region LWGyroscope

bool LWGyroscope::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

LWInputDevice &LWGyroscope::Update(LWWindow *Window, uint64_t lCurrentTime) {
	return *this;
}
#pragma endregion