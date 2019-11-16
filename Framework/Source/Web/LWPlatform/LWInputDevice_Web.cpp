#include "LWCore/LWText.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>
#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	LWWebEvent *Event = (LWWebEvent*)MessageData;
	LWVector2i WndSize = Window->GetSize();
	if (MessageID != LWWEBEVENT_MOUSE && MessageID != LWWEBEVENT_SCROLL) return false;
	if (MessageID == LWWEBEVENT_MOUSE) {
		m_Position = LWVector2i(Event->m_MouseEvent.m_Mousex, WndSize.y-Event->m_MouseEvent.m_Mousey);

		m_CurrState = ((Event->m_MouseEvent.m_ButtonState & 1) != 0 ? (uint32_t)LWMouseKey::Left : 0) | ((Event->m_MouseEvent.m_ButtonState & 2) != 0 ? (uint32_t)LWMouseKey::Right : 0) | ((Event->m_MouseEvent.m_ButtonState & 4) != 0 ? (uint32_t)LWMouseKey::Middle : 0);
	}
	if (MessageID == LWWEBEVENT_SCROLL) m_Scroll = (Event->m_MouseEvent.m_SubType);
	return true;
}
#pragma endregion

#pragma region LWKeyboard
bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *) {
	
	
	LWWebEvent *Event = (LWWebEvent*)MessageData;
	if (MessageID != LWWEBEVENT_KEY) return false;
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

	bool Up = Event->m_KeyEvent.m_SubType == EMSCRIPTEN_EVENT_KEYDOWN;
	if (Event->m_KeyEvent.m_CharCode != 0xFFFFFFFF && m_CharPressed<MaxKeyChanges) m_CharInputs[m_CharPressed++] = Event->m_KeyEvent.m_CharCode;
	SetKeyState(Event->m_KeyEvent.m_KeyCode, Up);
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
