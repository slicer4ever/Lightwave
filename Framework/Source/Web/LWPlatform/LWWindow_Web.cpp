#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWCore/LWTimer.h"
#include <iostream>
#include <cstdarg>
#include <unordered_map>

uint32_t LWWindow::MakeDialog(const LWUTF8Iterator &Text, const LWUTF8Iterator &Header, uint32_t DialogFlags) {
	LWLogEvent<256>("Dialog: {}: {}", Header, Text);
	return 0;
}

bool LWWindow::MakeSaveFileDialog(const LWUTF8Iterator &Filter, char8_t *Buffer, uint32_t BufferLen) {
	return false;
}

bool LWWindow::MakeLoadFileDialog(const LWUTF8Iterator &Filter, char8_t *Buffer, uint32_t BufferLen) {
	return false;
}

uint32_t LWWindow::MakeLoadFileMultipleDialog(const LWUTF8Iterator &Filter, char8_t **Bufer, uint32_t BufferLen, uint32_t BufferCount) {
	return 0;
}

bool LWWindow::WriteClipboardText(const LWUTF8Iterator &Text) {
	return false;
}

uint32_t LWWindow::ReadClipboardText(char *Buffer, uint32_t BufferLen) {
	return false;
}

LWWindow &LWWindow::SetTitle(const LWUTF8Iterator &Title) {
	m_Title = Title;
	return *this;
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position) {
	return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size) {
	emscripten_set_canvas_element_size(nullptr, m_Size.x, m_Size.y);
	m_Size = Size;
	m_Flag |= SizeChanged;
	return *this;
}

LWWindow &LWWindow::SetVisible(bool isVisible) {
	//X11Context *Con = (X11Context*)m_Context;
	//if (m_Flag&Initiated) ShowWindow(Con->m_WND, isVisible ? SW_SHOW : SW_HIDE);
	//m_Flag = (m_Flag&~Visible) | (isVisible ? Visible : 0);
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused) {
	
	//m_Flag = (m_Flag&~Focused) | (isFocused ? Focused : 0) | FocusChanged;
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool isFullscreen) {
	return *this;
}

LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position) {
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool isVisible) {
	//if (isVisible && (m_Flag&MouseVisible) == 0) emscripten_hide_mouse();
	//else if (!isVisible && (m_Flag&MouseVisible) != 0) emscripten_show_mouse();

	//m_Flag = (m_Flag&~MouseVisible) | (isVisible ? MouseVisible : 0);
	return *this;
}

LWWindow &LWWindow::SetActiveGamepad(LWGamePad *Gamepad) {
	m_ActiveGamepad = Gamepad;
	return *this;
}

LWInputDevice *LWWindow::AttachInputDevice(LWInputDevice *Device) {
	Device->SetNext(m_FirstDevice);
	m_FirstDevice = Device;
	return Device;
}

LWWindow &LWWindow::OpenKeyboard(uint32_t) {
	return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t, uint32_t) {
	return *this;
}

LWWindow &LWWindow::SetKeyboardText(const LWUTF8Iterator &) {
	return *this;
}

LWWindow &LWWindow::CloseKeyboard(void) {
	return *this;
}

LWWindow &LWWindow::GetKeyboardEditRange(uint32_t &, uint32_t &) {
	return *this;
}

uint32_t LWWindow::GetKeyboardText(char8_t *Buffer, uint32_t) {
	if (Buffer) *Buffer = '\0';
	return 0;
}

LWVector4f LWWindow::GetKeyboardLayout(void) {
	return LWVector4f();
}

uint32_t LWWindow::GetKeyboardType(void) {
	return 0;
}

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime) {
	LWWebEvent *Event = (LWWebEvent*)MessageData;
	if (Message == LWWEBEVENT_FOCUS) {
		m_Flag = (m_Flag&~Focused) | FocusChanged | ((Event->m_FocusEvent.m_SubType == EMSCRIPTEN_EVENT_BLUR || Event->m_FocusEvent.m_SubType == EMSCRIPTEN_EVENT_FOCUSOUT) ? 0 : Focused);
		return true;
	}else if(Message==LWWEBEVENT_RESIZE){
		m_Flag |= SizeChanged;
		m_Size = LWVector2i(Event->m_SizeEvent.m_Width, Event->m_SizeEvent.m_Height);
		return true;
	} else {
		for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) {
			if (Device->ProcessSystemMessage(Message, MessageData, lCurrentTime, this)) return true;
		}
	}
	return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime) {
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);

	LWVector2i NewSize;
	int32_t FullScreenFlag;
	emscripten_get_canvas_size(&NewSize.x, &NewSize.y, &FullScreenFlag);
	if(NewSize!=m_Size){
		m_Flag |= SizeChanged;
		m_Size = NewSize;
	}

	LWWebEvent Event;
	for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);
	while ((m_Flag&Terminate) == 0 && m_Context.PopEvent(Event)) {
		if (!ProcessWindowMessage(Event.m_Type, &Event, lCurrentTime)) {
			LWLogWarn<256>("Unknown event received: {}\n", Event.m_Type);
		}
	}
	return *this;
}

LWInputDevice *LWWindow::GetInputDevices(void) {
	return m_FirstDevice;
}

LWMouse *LWWindow::GetMouseDevice(void) {
	return m_MouseDevice;
}

LWKeyboard *LWWindow::GetKeyboardDevice(void) {
	return m_KeyboardDevice;
}

LWTouch *LWWindow::GetTouchDevice(void) {
	return m_TouchDevice;
}

LWAccelerometer *LWWindow::GetAccelerometerDevice(void) {
	return m_AccelerometerDevice;
}

LWGyroscope *LWWindow::GetGyroscopeDevice(void) {
	return m_GyroscopeDevice;
}

LWGamePad *LWWindow::GetGamepadDevice(uint32_t i) {
	return m_GamepadDevice[i];
}

LWGamePad *LWWindow::GetActiveGamepadDevice(void) {
	return m_ActiveGamepad;
}

const LWUTF8 &LWWindow::GetTitle(void) const {
	return m_Title;
}

const LWUTF8 &LWWindow::GetName(void) const {
	return m_Name;
}

LWVector2i LWWindow::GetPosition(void) const {
	return m_Position;
}

LWVector2f LWWindow::GetPositionf(void) const {
	return LWVector2f((float)m_Position.x, (float)m_Position.y);
}

LWVector2i LWWindow::GetSize(void) const {
	return m_Size;
}

LWVector2f LWWindow::GetSizef(void) const {
	return LWVector2f((float)m_Size.x, (float)m_Size.y);
}

float LWWindow::GetAspect(void) const {
	return (float)m_Size.x / (float)m_Size.y;
}

uint32_t LWWindow::GetFlag(void) const {
	return m_Flag;
}

uint32_t LWWindow::GetOrientation(void) const {
	return m_Flag&(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270);
}

LWWindowContext &LWWindow::GetContext(void) {
	return m_Context;
}

LWAllocator *LWWindow::GetAllocator(void) const {
	return m_Allocator;
}

bool LWWindow::isFinished(void) const {
	return (m_Flag&Terminate) != 0;
}

bool LWWindow::isFocused(void) const {
	return (m_Flag&Focused) != 0;
}

bool LWWindow::SizeUpdated(void) const {
	return (m_Flag&SizeChanged) != 0;
}

bool LWWindow::PositionUpdated(void) const {
	return (m_Flag&PosChanged) != 0;
}

bool LWWindow::FocusUpdated(void) const {
	return (m_Flag&FocusChanged) != 0;
}

bool LWWindow::isVisible(void) const {
	return (m_Flag&Visible) != 0;
}

bool LWWindow::DidError(void) const {
	return (m_Flag & Error) != 0;
}

bool LWWindow::isVirtualKeyboardPresent(void) const {
	return (m_Flag&KeyboardPresent) != 0;
}


LWWindow::LWWindow(const LWUTF8Iterator &Title, const LWUTF8Iterator &Name, LWAllocator &Allocator, uint32_t Flag, const LWVector2i &Position, const LWVector2i &Size) : m_Title(Title, Allocator), m_Name(Name, Allocator), m_Allocator(&Allocator), m_FirstDevice(nullptr), m_Position(Position), m_Size(Size), m_Flag(Flag) {
	m_MouseDevice = nullptr;
	m_KeyboardDevice = nullptr;
	m_TouchDevice = nullptr;
	m_AccelerometerDevice = nullptr;
	m_GyroscopeDevice = nullptr;
	m_ActiveGamepad = nullptr;
	memset(m_GamepadDevice, 0, sizeof(m_GamepadDevice));

	auto SizeEvent = [](int ResizeType, const EmscriptenUiEvent *ResizeEvent, void *UserData)->int {
		LWWindow *Wnd = (LWWindow *)UserData;
		LWWindowContext &Ctx = Wnd->GetContext();
		LWWebEvent Event;
		Event.m_Type = LWWEBEVENT_RESIZE;
		Event.m_SizeEvent.m_SubType = ResizeType;
		Event.m_SizeEvent.m_Width = ResizeEvent->windowInnerWidth;
		Event.m_SizeEvent.m_Height = ResizeEvent->windowInnerHeight;
		Ctx.PushEvent(Event);
		return true;
	};

	auto FocusEvent = [](int FocusType, const EmscriptenFocusEvent *focusEvent, void *UserData)->int {
		LWWindow *Wnd = (LWWindow*)UserData;
		LWWindowContext &Ctx = Wnd->GetContext();
		LWWebEvent Event;
		Event.m_Type = LWWEBEVENT_FOCUS;
		Event.m_FocusEvent.m_SubType = FocusType;
		Ctx.PushEvent(Event);
		return true;
	};

	auto MouseEvent = [](int EventType, const EmscriptenMouseEvent *MouseEvent, void *UserData)->int {
		LWWindow *Wnd = (LWWindow*)UserData;
		LWWindowContext &Ctx = Wnd->GetContext();
		LWWebEvent Event;
		Event.m_Type = LWWEBEVENT_MOUSE;
		Event.m_MouseEvent.m_SubType = EventType;
		Event.m_MouseEvent.m_Mousex = MouseEvent->canvasX;
		Event.m_MouseEvent.m_Mousey = MouseEvent->canvasY;
		Event.m_MouseEvent.m_ButtonState = MouseEvent->buttons;
		Ctx.PushEvent(Event);
		return true;
	};

	auto MouseWheelEvent = [](int EventType, const EmscriptenWheelEvent *WheelEvent, void *UserData)->int {
		LWWindow *Wnd = (LWWindow*)UserData;
		LWWindowContext &Ctx = Wnd->GetContext();
		LWWebEvent Event;
		Event.m_Type = LWWEBEVENT_SCROLL;
		Event.m_MouseEvent.m_SubType = (int32_t)(WheelEvent->deltaY > 0 ? -120 : 120);
		Ctx.PushEvent(Event);
		return true;
	};

	auto KeyEvent = [](int EventType, const EmscriptenKeyboardEvent *KeyEvent, void *UserData)->int {
		LWWindow *Wnd = (LWWindow*)UserData;
		LWWindowContext &Ctx = Wnd->GetContext();
		LWWebEvent Event;

		auto TranslateKey = [](const char *Key) {
			static bool FirstRun = true;
			static std::unordered_map<uint32_t, uint32_t> KeyMap;
			if (FirstRun) {
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyA"), (uint32_t)LWKey::A));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyB"), (uint32_t)LWKey::B));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyC"), (uint32_t)LWKey::C));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyD"), (uint32_t)LWKey::D));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyE"), (uint32_t)LWKey::E));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyF"), (uint32_t)LWKey::F));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyG"), (uint32_t)LWKey::G));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyH"), (uint32_t)LWKey::H));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyI"), (uint32_t)LWKey::I));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyJ"), (uint32_t)LWKey::J));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyK"), (uint32_t)LWKey::K));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyL"), (uint32_t)LWKey::L));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyM"), (uint32_t)LWKey::M));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyN"), (uint32_t)LWKey::N));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyO"), (uint32_t)LWKey::O));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyP"), (uint32_t)LWKey::P));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyQ"), (uint32_t)LWKey::Q));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyR"), (uint32_t)LWKey::R));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyS"), (uint32_t)LWKey::S));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyT"), (uint32_t)LWKey::T));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyU"), (uint32_t)LWKey::U));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyV"), (uint32_t)LWKey::V));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyW"), (uint32_t)LWKey::W));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyX"), (uint32_t)LWKey::X));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyY"), (uint32_t)LWKey::Y));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("KeyZ"), (uint32_t)LWKey::Z));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit0"), (uint32_t)LWKey::Num0));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit1"), (uint32_t)LWKey::Num1));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit2"), (uint32_t)LWKey::Num2));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit3"), (uint32_t)LWKey::Num3));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit4"), (uint32_t)LWKey::Num4));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit5"), (uint32_t)LWKey::Num5));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit6"), (uint32_t)LWKey::Num6));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit7"), (uint32_t)LWKey::Num7));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit8"), (uint32_t)LWKey::Num8));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Digit9"), (uint32_t)LWKey::Num9));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F1"), (uint32_t)LWKey::F1));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F2"), (uint32_t)LWKey::F2));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F3"), (uint32_t)LWKey::F3));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F4"), (uint32_t)LWKey::F4));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F5"), (uint32_t)LWKey::F5));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F6"), (uint32_t)LWKey::F6));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F7"), (uint32_t)LWKey::F7));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F8"), (uint32_t)LWKey::F8));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F9"), (uint32_t)LWKey::F9));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F10"), (uint32_t)LWKey::F10));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F11"), (uint32_t)LWKey::F11));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F12"), (uint32_t)LWKey::F12));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F13"), (uint32_t)LWKey::F13));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F14"), (uint32_t)LWKey::F14));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F15"), (uint32_t)LWKey::F15));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F16"), (uint32_t)LWKey::F16));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F17"), (uint32_t)LWKey::F17));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F18"), (uint32_t)LWKey::F18));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("F19"), (uint32_t)LWKey::F19));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Escape"), (uint32_t)LWKey::Esc));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("PrintScreen"), (uint32_t)LWKey::PrintScreen));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ScrollLock"), (uint32_t)LWKey::ScrollLock));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("PageUp"), (uint32_t)LWKey::PageUp));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("PageDown"), (uint32_t)LWKey::PageDown));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Home"), (uint32_t)LWKey::Home));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("End"), (uint32_t)LWKey::End));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Insert"), (uint32_t)LWKey::Insert));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Delete"), (uint32_t)LWKey::Delete));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Backspace"), (uint32_t)LWKey::Back));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Equal"), (uint32_t)LWKey::OEM_PLUS));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Minus"), (uint32_t)LWKey::OEM_MINUS));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Equal"), (uint32_t)LWKey::OEM_0));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Tab"), (uint32_t)LWKey::Tab));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("BracketLeft"), (uint32_t)LWKey::OEM_4));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("BracketRight"), (uint32_t)LWKey::OEM_5));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Backslash"), (uint32_t)LWKey::OEM_6));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("CapsLock"), (uint32_t)LWKey::CapLock));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Semicolon"), (uint32_t)LWKey::OEM_2));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Quote"), (uint32_t)LWKey::OEM_3));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Enter"), (uint32_t)LWKey::Return));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ShiftLeft"), (uint32_t)LWKey::LShift));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Comma"), (uint32_t)LWKey::OEM_COMMA));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Period"), (uint32_t)LWKey::OEM_PERIOD));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Slash"), (uint32_t)LWKey::OEM_1));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ShiftRight"), (uint32_t)LWKey::RShift));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ControlLeft"), (uint32_t)LWKey::LCtrl));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("OSLeft"), (uint32_t)LWKey::LMenu));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("AltLeft"), (uint32_t)LWKey::LAlt));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Space"), (uint32_t)LWKey::Space));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("AltRight"), (uint32_t)LWKey::RAlt));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ContextMenu"), (uint32_t)LWKey::RMenu));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ControlRight"), (uint32_t)LWKey::RCtrl));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ControlLeft"), (uint32_t)LWKey::LCtrl));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ArrowLeft"), (uint32_t)LWKey::Left));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ArrowRight"), (uint32_t)LWKey::Right));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ArrowUp"), (uint32_t)LWKey::Up));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("ArrowDown"), (uint32_t)LWKey::Down));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad0"), (uint32_t)LWKey::Num0));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad1"), (uint32_t)LWKey::Num1));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad2"), (uint32_t)LWKey::Num2));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad3"), (uint32_t)LWKey::Num3));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad4"), (uint32_t)LWKey::Num4));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad5"), (uint32_t)LWKey::Num5));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad6"), (uint32_t)LWKey::Num6));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad7"), (uint32_t)LWKey::Num7));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad8"), (uint32_t)LWKey::Num8));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("Numpad9"), (uint32_t)LWKey::Num9));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumLock"), (uint32_t)LWKey::NumLock));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadDivide"), (uint32_t)LWKey::NumDivide));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadMultiply"), (uint32_t)LWKey::NumMultiply));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadSubtract"), (uint32_t)LWKey::NumMinus));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadAdd"), (uint32_t)LWKey::NumAdd));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadEnter"), (uint32_t)LWKey::NumReturn));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("NumpadDecimal"), (uint32_t)LWKey::NumDecimal));

				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("MediaPlayPause"), (uint32_t)LWKey::MediaPlayPause));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("VolumeMute"), (uint32_t)LWKey::VolumeMute));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("VolumeDown"), (uint32_t)LWKey::VolumeDown));
				KeyMap.emplace(std::make_pair<uint32_t, uint32_t>(LWCrypto::HashFNV1A("VolumeUp"), (uint32_t)LWKey::VolumeUp));
				FirstRun = false;
			}

			auto Iter = KeyMap.find(LWCrypto::HashFNV1A(Key));
			if (Iter == KeyMap.end()) return (uint32_t)LWKey::Unknown;
			return Iter->second;
		};

		Event.m_Type = LWWEBEVENT_KEY;
		Event.m_KeyEvent.m_SubType = EventType;

		Event.m_KeyEvent.m_KeyCode = TranslateKey(KeyEvent->code);
		Event.m_KeyEvent.m_CharCode = 0xFFFFFFFF;
		LWUTF8Iterator KeyIter;
		uint32_t Len, RawLen = 0;
		if (LWUTF8Iterator::Create(KeyIter, KeyEvent->key, Len, RawLen)) {
			if (Len == 1 && EventType == EMSCRIPTEN_EVENT_KEYDOWN) Event.m_KeyEvent.m_CharCode = *KeyIter;
		}
		Ctx.PushEvent(Event);
		return true;
	};


	emscripten_set_blur_callback("#document", this, true, FocusEvent);
	emscripten_set_focus_callback("#document", this, true, FocusEvent);
	emscripten_set_focusin_callback("#document", this, true, FocusEvent);
	emscripten_set_focusout_callback("#document", this, true, FocusEvent);

	emscripten_set_mousedown_callback("#canvas", this, true, MouseEvent);
	emscripten_set_mouseup_callback("#canvas", this, true, MouseEvent);
	emscripten_set_mousemove_callback("#canvas", this, true, MouseEvent);

	emscripten_set_wheel_callback("#canvas", this, true, MouseWheelEvent);

	emscripten_set_keydown_callback("#canvas", this, true, KeyEvent);
	emscripten_set_keyup_callback("#canvas", this, true, KeyEvent);

	emscripten_set_resize_callback(nullptr, this, true, SizeEvent);

	EmscriptenOrientationChangeEvent Orientation;
	emscripten_set_canvas_element_size(nullptr, m_Size.x, m_Size.y);
	emscripten_get_orientation_status(&Orientation);
	switch (Orientation.orientationIndex) {
	case EMSCRIPTEN_ORIENTATION_PORTRAIT_PRIMARY:
		m_Flag |= Rotation_0;
		break;
	case EMSCRIPTEN_ORIENTATION_LANDSCAPE_PRIMARY:
		m_Flag |= Rotation_90;
		break;
	case EMSCRIPTEN_ORIENTATION_PORTRAIT_SECONDARY:
		m_Flag |= Rotation_180;
		break;
	case EMSCRIPTEN_ORIENTATION_LANDSCAPE_SECONDARY:
		m_Flag |= Rotation_270;
		break;
	default:
		break;
	}
	if ((m_Flag & Error) == 0) {
		if (Flag & MouseDevice) m_MouseDevice = AttachInputDevice(Allocator.Create<LWMouse>())->AsMouse();
		if (Flag & KeyboardDevice) m_KeyboardDevice = AttachInputDevice(Allocator.Create<LWKeyboard>())->AsKeyboard();
	}
}


LWWindow::~LWWindow() {

	//if (m_Context.m_Window != 0) XDestroyWindow(m_Context.m_Display, m_Context.m_Window);
	//if (m_Context.m_Display) XCloseDisplay(m_Context.m_Display);

	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
	if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
	if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);
}


