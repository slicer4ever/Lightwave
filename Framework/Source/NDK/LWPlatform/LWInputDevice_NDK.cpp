#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWTimer.h"
#include <android/keycodes.h>
#include <android/sensor.h>
#include <iostream>
#include <algorithm>

#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window){
	
	return false;
}

#pragma endregion

#pragma region LWKeyboard

bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *){
	if (MessageID != 0) return false;
	static jclass KeyEventClass = 0;
	static jmethodID KeyEvent_getUnicodeChar = 0;
	static jmethodID KeyEvent_Constructor = 0;
	static JNIEnv *Env = nullptr;
	static uint32_t EnvCounter = 0;
	if(EnvCounter!=LWAppContext.m_AppEnvCounter){
		Env = LWAppContext.m_AppEnv;
		EnvCounter = LWAppContext.m_AppEnvCounter;
		KeyEventClass = Env->FindClass("android/view/KeyEvent");
		KeyEvent_getUnicodeChar = Env->GetMethodID(KeyEventClass, "getUnicodeChar", "(I)I");
		KeyEvent_Constructor = Env->GetMethodID(KeyEventClass, "<init>", "(II)V");
	}
	
	auto Translate = [](uint32_t Key) -> LWKey{

		switch (Key){
		case AKEYCODE_A: return LWKey::A;
		case AKEYCODE_B: return LWKey::B;
		case AKEYCODE_C: return LWKey::C;
		case AKEYCODE_D: return LWKey::D;
		case AKEYCODE_E: return LWKey::E;
		case AKEYCODE_F: return LWKey::F;
		case AKEYCODE_G: return LWKey::G;
		case AKEYCODE_H: return LWKey::H;
		case AKEYCODE_I: return LWKey::I;
		case AKEYCODE_J: return LWKey::J;
		case AKEYCODE_K: return LWKey::K;
		case AKEYCODE_L: return LWKey::L;
		case AKEYCODE_M: return LWKey::M;
		case AKEYCODE_N: return LWKey::N;
		case AKEYCODE_O: return LWKey::O;
		case AKEYCODE_P: return LWKey::P;
		case AKEYCODE_Q: return LWKey::Q;
		case AKEYCODE_R: return LWKey::R;
		case AKEYCODE_S: return LWKey::S;
		case AKEYCODE_T: return LWKey::T;
		case AKEYCODE_U: return LWKey::U;
		case AKEYCODE_V: return LWKey::V;
		case AKEYCODE_W: return LWKey::W;
		case AKEYCODE_X: return LWKey::X;
		case AKEYCODE_Y: return LWKey::Y;
		case AKEYCODE_Z: return LWKey::Z;
		case AKEYCODE_0: return LWKey::Key0;
		case AKEYCODE_1: return LWKey::Key1;
		case AKEYCODE_2: return LWKey::Key2;
		case AKEYCODE_3: return LWKey::Key3;
		case AKEYCODE_4: return LWKey::Key4;
		case AKEYCODE_5: return LWKey::Key5;
		case AKEYCODE_6: return LWKey::Key6;
		case AKEYCODE_7: return LWKey::Key7;
		case AKEYCODE_8: return LWKey::Key8;
		case AKEYCODE_9: return LWKey::Key9;
		case AKEYCODE_TAB: return LWKey::Tab;
		case AKEYCODE_SHIFT_LEFT: return LWKey::LShift;
		case AKEYCODE_ALT_LEFT: return LWKey::LAlt;
		case AKEYCODE_SPACE: return LWKey::Space;
		case AKEYCODE_ALT_RIGHT: return LWKey::RAlt;
		case AKEYCODE_MENU: return LWKey::RMenu;
		case AKEYCODE_SHIFT_RIGHT: return LWKey::RShift;
		case AKEYCODE_ENTER: return LWKey::Return;
		case AKEYCODE_DEL: return LWKey::Back;
		case AKEYCODE_COMMA: return LWKey::OEM_COMMA;
		case AKEYCODE_MINUS: return LWKey::OEM_MINUS;
		case AKEYCODE_PLUS: return LWKey::OEM_PLUS;
		case AKEYCODE_GRAVE: return LWKey::OEM_0;
		case AKEYCODE_SLASH: return LWKey::OEM_1;
		case AKEYCODE_SEMICOLON: return LWKey::OEM_2;
		case AKEYCODE_APOSTROPHE: return LWKey::OEM_3;
		case AKEYCODE_LEFT_BRACKET: return LWKey::OEM_4;
		case AKEYCODE_RIGHT_BRACKET: return LWKey::OEM_5;
		case AKEYCODE_BACKSLASH: return LWKey::OEM_6;
		case AKEYCODE_DPAD_LEFT: return LWKey::Left;
		case AKEYCODE_DPAD_RIGHT: return LWKey::Right;
		case AKEYCODE_DPAD_UP: return LWKey::Up;
		case AKEYCODE_DPAD_DOWN: return LWKey::Down;
		case AKEYCODE_HOME: return LWKey::Home;
		case AKEYCODE_PAGE_UP: return LWKey::PageUp;
		case AKEYCODE_PAGE_DOWN: return LWKey::PageDown;
		case AKEYCODE_BACK: return LWKey::Esc;
		case AKEYCODE_MEDIA_PLAY_PAUSE: return LWKey::MediaPlayPause;
		case AKEYCODE_MEDIA_NEXT: return LWKey::MediaNext;
		case AKEYCODE_MEDIA_PREVIOUS: return LWKey::MediaPrev;
		case AKEYCODE_VOLUME_UP: return LWKey::VolumeUp;
		case AKEYCODE_VOLUME_DOWN: return LWKey::VolumeDown;

#if __ANDROID_API__ > 9
		case AKEYCODE_SCROLL_LOCK: return LWKey::ScrollLock;
		case AKEYCODE_BREAK: return LWKey::Pause;
		case AKEYCODE_SYSRQ: return LWKey::PrintScreen;
		case AKEYCODE_CTRL_LEFT: return LWKey::LCtrl;
		case AKEYCODE_CTRL_RIGHT: return LWKey::RCtrl;
		case AKEYCODE_CAPS_LOCK: return LWKey::CapLock;
		case AKEYCODE_INSERT: return LWKey::Insert;
		case AKEYCODE_FORWARD_DEL: return LWKey::Delete;
		case AKEYCODE_F1: return LWKey::F1;
		case AKEYCODE_F2: return LWKey::F2;
		case AKEYCODE_F3: return LWKey::F3;
		case AKEYCODE_F4: return LWKey::F4;
		case AKEYCODE_F5: return LWKey::F5;
		case AKEYCODE_F6: return LWKey::F6;
		case AKEYCODE_F7: return LWKey::F7;
		case AKEYCODE_F8: return LWKey::F8;
		case AKEYCODE_F9: return LWKey::F9;
		case AKEYCODE_F10: return LWKey::F10;
		case AKEYCODE_F11: return LWKey::F11;
		case AKEYCODE_F12: return LWKey::F12;
		case AKEYCODE_NUM_LOCK: return LWKey::NumLock;
		case AKEYCODE_NUMPAD_0: return LWKey::Num0;
		case AKEYCODE_NUMPAD_1: return LWKey::Num1;
		case AKEYCODE_NUMPAD_2: return LWKey::Num2;
		case AKEYCODE_NUMPAD_3: return LWKey::Num3;
		case AKEYCODE_NUMPAD_4: return LWKey::Num4;
		case AKEYCODE_NUMPAD_5: return LWKey::Num5;
		case AKEYCODE_NUMPAD_6: return LWKey::Num6;
		case AKEYCODE_NUMPAD_7: return LWKey::Num7;
		case AKEYCODE_NUMPAD_8: return LWKey::Num8;
		case AKEYCODE_NUMPAD_9: return LWKey::Num9;
		case AKEYCODE_NUMPAD_DOT: return LWKey::NumDecimal;
		case AKEYCODE_NUMPAD_ENTER: return LWKey::NumReturn;
		case AKEYCODE_NUMPAD_ADD: return LWKey::NumAdd;
		case AKEYCODE_NUMPAD_SUBTRACT: return LWKey::NumMinus;
		case AKEYCODE_NUMPAD_MULTIPLY: return LWKey::NumMultiply;
		case AKEYCODE_NUMPAD_DIVIDE: return LWKey::NumDivide;
		case AKEYCODE_NUMPAD_EQUALS: return LWKey::NumEqual;
		case AKEYCODE_VOLUME_MUTE: return LWKey::VolumeMute;
#endif

		}
		return LWKey::Unknown;
	};
	auto SetKeyState = [this](uint32_t Key, bool Up){
		uint32_t Word = Key / (sizeof(uint32_t)* 8);
		if (Word >= (sizeof(m_CurrState) / sizeof(uint32_t))) return 0;
		uint32_t Bit = 1 << (Key % (sizeof(uint32_t)* 8));
		if (Up) m_CurrState[Word] &= ~Bit;
		else{
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
	AInputEvent *Event = (AInputEvent*)MessageData;
	if (AInputEvent_getType(Event) != AINPUT_EVENT_TYPE_KEY) return false;
	
	int32_t Action = AKeyEvent_getAction(Event);
	int32_t KeyCode = AKeyEvent_getKeyCode(Event);
	int32_t Meta = AKeyEvent_getMetaState(Event);
	LWKey TranslatedKey = Translate(KeyCode);
	//std::cout << "Got key: " << (uint32_t)TranslatedKey << " " << KeyCode << std::endl;

	bool Up = Action == AKEY_EVENT_ACTION_UP;
	if (TranslatedKey != LWKey::Unknown) SetKeyState((uint32_t)TranslatedKey, Up);
	if (m_CharPressed < MaxKeyChanges && !Up) {
		jobject EventObj = Env->NewObject(KeyEventClass, KeyEvent_Constructor, Action, KeyCode);
		m_CharInputs[m_CharPressed++] = Env->CallIntMethod(EventObj, KeyEvent_getUnicodeChar, Meta);
		Env->DeleteLocalRef(EventObj);
	}
	return TranslatedKey != LWKey::VolumeUp && TranslatedKey != LWKey::VolumeDown && TranslatedKey != LWKey::VolumeMute;
}
#pragma endregion

#pragma region LWTouch

bool LWTouch::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	if (MessageID != 0) return false;
	AInputEvent *Event = (AInputEvent*)MessageData;
	LWVector2i WndSize = Window->GetSize();
	if (AInputEvent_getType(Event) == AINPUT_EVENT_TYPE_MOTION) {
		int32_t Action = AMotionEvent_getAction(Event);
		int32_t Pntr = (Action&AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t PntrCnt = AMotionEvent_getPointerCount(Event);
		if (PntrCnt >= MaxTouchPoints) return true;
		m_PointCount = PntrCnt;
		for (uint32_t i = 0; i < PntrCnt; i++) {
			m_Points[i].m_Position = LWVector2i((int32_t)AMotionEvent_getX(Event, i), WndSize.y - (int32_t)AMotionEvent_getY(Event, i));
			m_Points[i].m_Size = std::min<float>(AMotionEvent_getSize(Event, i)*1000.0f, 10.0f);
			m_Points[i].m_State = LWTouchPoint::MOVED;
		}
		int32_t MEvent = Action&AMOTION_EVENT_ACTION_MASK;
		if (MEvent != AMOTION_EVENT_ACTION_DOWN && MEvent != AMOTION_EVENT_ACTION_UP && MEvent!=AMOTION_EVENT_ACTION_POINTER_UP && MEvent!=AMOTION_EVENT_ACTION_POINTER_DOWN) return true;
		m_Points[Pntr].m_State = (MEvent == AMOTION_EVENT_ACTION_DOWN || MEvent==AMOTION_EVENT_ACTION_POINTER_DOWN) ? LWTouchPoint::DOWN : LWTouchPoint::UP;
		if(m_Points[Pntr].m_State==LWTouchPoint::DOWN){
			m_Points[Pntr].m_DownTime = lCurrentTime;
			m_Points[Pntr].m_InitPosition = m_Points[Pntr].m_Position;
			m_Points[Pntr].m_PrevPosition = m_Points[Pntr].m_Position;
		}
		return true;
	}
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
	if (MessageID != 1) return false;
	ASensorEvent *Event = (ASensorEvent*)MessageData;
	if (Event->type != ASENSOR_TYPE_ACCELEROMETER) return false;
	ASensorVector Vec = Event->vector;
	m_Direction.x = Vec.x;
	m_Direction.y = Vec.y;
	m_Direction.z = Vec.z;
	return true;
}

LWInputDevice &LWAccelerometer::Update(LWWindow *Window, uint64_t lCurrentTime) {
	LWWindowContext &WndCtx = Window->GetContext();
	if ((m_Flag&(RequestEnabled | Enabled)) == RequestEnabled) {
		const ASensor *Accel = ASensorManager_getDefaultSensor(WndCtx.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
		ASensorEventQueue_enableSensor(WndCtx.m_SensorQueue, Accel);
		m_Flag |= Enabled;
	} else if ((m_Flag&(RequestDisabled | Enabled)) == (RequestDisabled | Enabled)) {
		const ASensor *Accel = ASensorManager_getDefaultSensor(WndCtx.m_SensorManager, ASENSOR_TYPE_ACCELEROMETER);
		ASensorEventQueue_disableSensor(WndCtx.m_SensorQueue, Accel);
		m_Flag &= ~Enabled;
	}
	m_Flag &= ~(RequestEnabled | RequestDisabled);
	return *this;
}
#pragma endregion

#pragma region LWGyroscope

bool LWGyroscope::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	if (MessageID != 1) return false;
	ASensorEvent *Event = (ASensorEvent*)MessageData;
	if (Event->type != ASENSOR_TYPE_GYROSCOPE) return false;
	ASensorVector Vec = Event->vector;
	m_Rotation.x += Vec.x;
	m_Rotation.y += Vec.y;
	m_Rotation.z += Vec.z;
	return true;
}

LWInputDevice &LWGyroscope::Update(LWWindow *Window, uint64_t lCurrentTime) {
	LWWindowContext &WndCtx = Window->GetContext();
	if ((m_Flag&(RequestEnabled | Enabled)) == RequestEnabled) {
		const ASensor *Gyro = ASensorManager_getDefaultSensor(WndCtx.m_SensorManager, ASENSOR_TYPE_GYROSCOPE);
		ASensorEventQueue_enableSensor(WndCtx.m_SensorQueue, Gyro);
		m_Flag |= Enabled;
	} else if ((m_Flag&(RequestDisabled | Enabled)) == (RequestDisabled | Enabled)) {
		const ASensor *Gyro = ASensorManager_getDefaultSensor(WndCtx.m_SensorManager, ASENSOR_TYPE_GYROSCOPE);
		ASensorEventQueue_disableSensor(WndCtx.m_SensorQueue, Gyro);
		m_Flag &= ~Enabled;
	}
	m_Flag &= ~(RequestEnabled | RequestDisabled);
	return *this;
}

#pragma endregion