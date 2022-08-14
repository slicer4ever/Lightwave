#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWLogger.h"
#include <iostream>
#include <cstdarg>

const uint32_t LWWindow::Terminate; /*!< \brief the window has requested for a termination */
const uint32_t LWWindow::Visible; /*!< \brief the window is visible. */
const uint32_t LWWindow::Focused; /*!< \brief the window is focused or not. */
const uint32_t LWWindow::MouseVisible; /*!< \brief rather or not the mouse is to be considered visible or not. */
const uint32_t LWWindow::PosChanged; /*!< \brief the window position has been changed. */
const uint32_t LWWindow::SizeChanged; /*!< \brief the window size has been changed. */
const uint32_t LWWindow::FocusChanged; /*!< \brief the window focus has been changed. */
const uint32_t LWWindow::Error; /*!< \brief the window had an error during creation. */
const uint32_t LWWindow::Fullscreen; /*!< \brief the window is designed to be in fullscreen mode. note that for DX11 the display mode does not need to be changed if this flag is set. */
const uint32_t LWWindow::Borderless; /*!< \brief the window is designed to be a borderless window. */
const uint32_t LWWindow::OrientationChanged; /*!< \brief signals that the window orientation has changed. */
const uint32_t LWWindow::KeyboardPresent; /*!< \brief the windows softward keyboard is present, this flag is only viable for iOS and Android OS's. */
const uint32_t LWWindow::Rotation_0; /*!< \brief the current window is in default portrait mode, and is not rotated at all. */
const uint32_t LWWindow::Rotation_90; /*!< \brief the current window is orientated in landscape mode. */
const uint32_t LWWindow::Rotation_180; /*!< \brief the current window is orientated in upside down portrait mode. */
const uint32_t LWWindow::Rotation_270; /*!< \brief the current window is orientated in upside down landscape mode. */

const uint32_t LWWindow::MouseDevice; /*!< \brief flag to indicate window should create a mouse device if applicable to the platform(windows, mac, linux). */
const uint32_t LWWindow::KeyboardDevice; /*!< \brief flag to indicate window should create a keyboard device if applicable to the platform. (windows, mac, linux, android, iOS) */
const uint32_t LWWindow::GamepadDevice; /*!< \brief flag to indicate window should create the gamepad devices if applicable to the platform. (windows). */
const uint32_t LWWindow::TouchDevice; /*!< \brief flag to indicate window should create the touchscreen device if applicable to the platform. (android, iOS). */
const uint32_t LWWindow::GyroscopeDevice; /*!< \brief flag to indicate window should create and activate the gyroscope device if applicable to the platform. (android, iOS). */
const uint32_t LWWindow::AccelerometerDevice; /*!< \brief flag to indicate window should create and activate the accelerometer device if applicable to the platform. (android, IOS). */

const uint32_t LWWindow::DialogOK; /*!< \brief provides the dialog OK button when creating a dialog. */
const uint32_t LWWindow::DialogYES; /*!< \brief provides the dialog an yes and no button when creating a dialog. */
const uint32_t LWWindow::DialogNo; /*!< \brief return value only on what MakeDialog creates. */
const uint32_t LWWindow::DialogCancel; /*!< \brief provides the dialog an cancel button when creating a dialog. */

char WriteClipboardTextB[512];
char RecvClipboardTextB[512];
LWWindowContext *ClipboardContext = nullptr;
LWWindow *ClipboardWindow = nullptr;
bool ClipSet = false;

uint32_t LWWindow::MakeDialog(const LWUTF8Iterator &Text, const LWUTF8Iterator &Header, uint32_t DialogFlags){
	LWLogEvent<256>("Dialog: {}: {}\n", Header, Text);
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
	Text.Copy(WriteClipboardTextB, sizeof(WriteClipboardTextB));
	XSetSelectionOwner(ClipboardContext->m_Display, ClipboardContext->m_AtomList[X11_CLIPBOARD], ClipboardContext->m_Window, CurrentTime);
	XFlush(ClipboardContext->m_Display);
	return true;
}

uint32_t LWWindow::ReadClipboardText(char8_t *Buffer, uint32_t BufferLen) {
	Window Owner = XGetSelectionOwner(ClipboardContext->m_Display, ClipboardContext->m_AtomList[X11_CLIPBOARD]);
	if (!Owner) return 0;
	XConvertSelection(ClipboardContext->m_Display, ClipboardContext->m_AtomList[X11_CLIPBOARD], XA_STRING, ClipboardContext->m_AtomList[X11_CLIPBOARD], ClipboardContext->m_Window, CurrentTime);
	ClipSet = false;
	while (!ClipSet) ClipboardWindow->Update(LWTimer::GetCurrent());
	return LWUTF8I(RecvClipboardTextB).Copy(Buffer, BufferLen);
}

LWWindow &LWWindow::SetTitle(const LWUTF8Iterator &Title){
	m_Title = Title;
	XStoreName(m_Context.m_Display, m_Context.m_Window, m_Title.c_str());
	return *this;
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position){
	
	/*Atom ReturnType;
	int32_t ReturnFormat;
	unsigned long NbrItems;
	unsigned long NbrBytes;
	unsigned char *res = nullptr;

	if (XGetWindowProperty(m_Context.m_Display, m_Context.m_Window, m_Context.m_AtomList[X11_NET_FRAME_EXTENTS], 0, 4, False, AnyPropertyType, &ReturnType, &ReturnFormat, &NbrItems, &NbrBytes, &res) != Success) {
		std::cout << "No window props" << std::endl;
		return *this;
	}
	if (NbrItems != 4 && !NbrBytes) {
		std::cout << "Got bad size: " << NbrItems << " " << NbrBytes << std::endl;
		return *this;
	}
	uint32_t *Propertys = (uint32_t*)res;
	std::cout << "Results: " << Propertys[0] << " " << Propertys[1] << " " << Propertys[2] << " " << Propertys[3] << std::endl;*/
	
	//I HAVE SPENT TOO MUCH TIME TRYING TO MAKE THIS FUCKING WORK ON X11, FUCK X11
	LWVector2i Pos = Position;

	//std::cout << "Set pos: " << Pos.x << " " << Pos.y << std::endl;
		Window Child;
		if (!XTranslateCoordinates(m_Context.m_Display, m_Context.m_Parent, m_Context.m_Root, Position.x, Position.y, &Pos.x, &Pos.y, &Child)) {
			std::cout << "Error translating!" << std::endl;
			return *this;
		}
	XMoveWindow(m_Context.m_Display, m_Context.m_Window, Pos.x, Pos.y);
	//m_Position = Pos;
	//m_Flag |= PosChanged;
	return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size){
	XResizeWindow(m_Context.m_Display, m_Context.m_Window, (uint32_t)Size.x, (uint32_t)Size.y);
	m_Size = Size;
	m_Flag |= SizeChanged;
	return *this;
}

LWWindow &LWWindow::SetVisible(bool isVisible){
	//X11Context *Con = (X11Context*)m_Context;
	//if (m_Flag&Initiated) ShowWindow(Con->m_WND, isVisible ? SW_SHOW : SW_HIDE);
	//m_Flag = (m_Flag&~Visible) | (isVisible ? Visible : 0);
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused){
	XSetInputFocus(m_Context.m_Display, isFocused ? m_Context.m_Window : PointerRoot, RevertToParent, CurrentTime);
	XFlush(m_Context.m_Display);
	m_Flag = (m_Flag&~Focused) | (isFocused ? Focused : 0) | FocusChanged;
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool isFullscreen) {
	if (isBorderless && (m_Flag&Borderless)) return *this;
	else if (!isBorderless && (m_Flag&Borderless) == 0) return *this;
	XEvent e;
	e.xclient.type = ClientMessage;
	e.xclient.window = m_Context.m_Window;
	e.xclient.message_type = m_Context.m_AtomList[X11_NET_WM_STATE];
	e.xclient.format = 32;
	e.xclient.data.l[0] = 2;
	e.xclient.data.l[1] = m_Context.m_AtomList[X11_NET_WM_STATE_FULLSCREEN];
	e.xclient.data.l[2] = 0;
	e.xclient.data.l[3] = 1;
	e.xclient.data.l[4] = 0;

	
	if (isBorderless) {
		Atom BorderProps[] = { 2, 0, 0, 0, 0 };
		XChangeProperty(m_Context.m_Display, m_Context.m_Window, m_Context.m_AtomList[X11_MOTIF_WM_HINTS], m_Context.m_AtomList[X11_MOTIF_WM_HINTS], 32, PropModeReplace, (uint8_t*)&BorderProps, 5);
	} else {
		Atom BorderProps[] = { 2, 1, 1, 1, 1};
		XChangeProperty(m_Context.m_Display, m_Context.m_Window, m_Context.m_AtomList[X11_MOTIF_WM_HINTS], m_Context.m_AtomList[X11_MOTIF_WM_HINTS], 32, PropModeReplace, (uint8_t*)&BorderProps, 5);
	}
	if(isFullscreen) XSendEvent(m_Context.m_Display, m_Context.m_Root, False, SubstructureRedirectMask | SubstructureNotifyMask, &e);
	m_Flag = (m_Flag&~Borderless) | (isBorderless ? Borderless : 0) | SizeChanged | PosChanged;
	return *this;
}

LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position){
	XWarpPointer(m_Context.m_Display, 0, m_Context.m_Window, 0, 0, 0, 0, Position.x, m_Size.y-Position.y);
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool isVisible){
	if (isVisible && (m_Flag&MouseVisible) == 0) XUndefineCursor(m_Context.m_Display, m_Context.m_Window);
	else if (!isVisible && (m_Flag&MouseVisible) != 0) XDefineCursor(m_Context.m_Display, m_Context.m_Window, 0);

	m_Flag = (m_Flag&~MouseVisible) | (isVisible ? MouseVisible : 0);
	return *this;
}

LWWindow &LWWindow::SetActiveGamepad(LWGamePad *Gamepad) {
	m_ActiveGamepad = Gamepad;
	return *this;
}

LWInputDevice *LWWindow::AttachInputDevice(LWInputDevice *Device){
	Device->SetNext(m_FirstDevice);
	m_FirstDevice = Device;
	return Device;
}

LWWindow &LWWindow::OpenKeyboard(uint32_t){
	return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t, uint32_t){
	return *this;
}

LWWindow &LWWindow::SetKeyboardText(const LWUTF8Iterator &) {
	return *this;
}

LWWindow &LWWindow::CloseKeyboard(void){
	return *this;
}

LWWindow &LWWindow::GetKeyboardEditRange(uint32_t &, uint32_t &){
	return *this;
}

uint32_t LWWindow::GetKeyboardText(char *Buffer, uint32_t){
	if (Buffer) *Buffer = '\0';
	return 0;
}

LWVector4f LWWindow::GetKeyboardLayout(void) {
	return LWVector4f();
}

uint32_t LWWindow::GetKeyboardType(void) {
	return 0;
}

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime){
	XEvent *Evnt = (XEvent*)MessageData;
	
	if(Message==ClientMessage){
		if(Evnt->xclient.data.l[0]==(int32_t)m_Context.m_AtomList[X11_WM_DELETE_WINDOW]){
			m_Flag |= Terminate;
			return true;
		}
	}else if(Message==FocusIn){
		m_Flag = (m_Flag | FocusChanged | Focused);
		return true;
	}else if(Message==FocusOut){
		m_Flag = (m_Flag | FocusChanged)&~Focused;
		return true;
	} else if (Message == ConfigureNotify) {
		XConfigureEvent *CE = (XConfigureEvent*)Evnt;
		LWVector2i Size = LWVector2i(CE->width, CE->height);
		LWVector2i Pos = LWVector2i(CE->x, CE->y);
		m_Flag |= (Size == m_Size ? 0 : SizeChanged) | (Pos == m_Position ? 0 : PosChanged);
		m_Size = Size;
		m_Position = Pos;
		return true;
	} else if (Message == SelectionRequest) {
		XSelectionRequestEvent *Req = &Evnt->xselectionrequest;
		XEvent res;
		if (Req->target == XA_STRING) {
			uint32_t Len = strlen(WriteClipboardTextB);
			XChangeProperty(m_Context.m_Display, Req->requestor, Req->property, XA_STRING, 8, PropModeReplace, (const unsigned char*)WriteClipboardTextB, Len);
			res.xselection.property = Req->property;
		}
		res.xselection.type = SelectionNotify;
		res.xselection.display = Req->display;
		res.xselection.requestor = Req->requestor;
		res.xselection.selection = Req->selection;
		res.xselection.target = Req->target;
		res.xselection.time = Req->time;
		XSendEvent(m_Context.m_Display, Req->requestor, 0, 0, &res);
		XFlush(m_Context.m_Display);
		return true;
	} else if (Message == SelectionNotify) {
		RecvClipboardTextB[0] = '\0';
		if (Evnt->xselection.property == 0) {
			ClipSet = true;
			return false;
		}
		unsigned char *Result = nullptr;
		Atom RealType;
		int RealFormat;
		long unsigned int BytesAfter, ItemCnt;
		XGetWindowProperty(ClipboardContext->m_Display, Evnt->xselection.requestor, Evnt->xselection.property, 0, 0xFFFFFFFF, false, Evnt->xselection.target, &RealType, &RealFormat, &ItemCnt, &BytesAfter, &Result);
		if (!Result) {
			ClipSet = true;
			return false;
		}
		LWUTF8I((const char8_t*)Result).Copy(RecvClipboardTextB, sizeof(RecvClipboardTextB));
		ClipSet = true;
		return true;
	}else{
		for(LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()){
			if (Device->ProcessSystemMessage(Message, MessageData, lCurrentTime, this)) return true;
		}
	}
	return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime){
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);
	
	for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);
	XEvent Evnt;
	while((m_Flag&Terminate)==0 && XPending(m_Context.m_Display)){
		XNextEvent(m_Context.m_Display, &Evnt);
		if(!ProcessWindowMessage(Evnt.type, &Evnt, lCurrentTime)){
			std::cout << "Unknown event received: " << Evnt.type << std::endl;
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

const LWUTF8 &LWWindow::GetTitle(void) const{
	return m_Title;
}

const LWUTF8 &LWWindow::GetName(void) const{
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

uint32_t LWWindow::GetFlag(void) const{
	return m_Flag;
}

uint32_t LWWindow::GetOrientation(void) const{
	return m_Flag&(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270);
}

LWWindowContext &LWWindow::GetContext(void) {
	return m_Context;
}

LWAllocator *LWWindow::GetAllocator(void) const{
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

LWWindow::LWWindow(const LWUTF8Iterator &Title, const LWUTF8Iterator &Name, LWAllocator &Allocator, uint32_t Flag, const LWVector2i &Position, const LWVector2i &Size) : m_Title(Title, Allocator), m_Name(Name, Allocator), m_Allocator(&Allocator), m_FirstDevice(nullptr), m_Position(Position), m_Size(Size), m_Flag(Flag){
	m_MouseDevice = nullptr;
	m_KeyboardDevice = nullptr;
	m_TouchDevice = nullptr;
	m_AccelerometerDevice = nullptr;
	m_GyroscopeDevice = nullptr;
	m_ActiveGamepad = nullptr;
	memset(m_GamepadDevice, 0, sizeof(m_GamepadDevice));
	XSetWindowAttributes WindowAttrib;
	Window Child;
	Window *ChildWindows;
	uint32_t ChildWindowCnt;
	m_Context.m_Display = nullptr;
	m_Context.m_Window = 0;
	LWVector2i Pos = m_Position;
	if (!(m_Context.m_Display = XOpenDisplay(nullptr))) MakeDialog(u8"Error: 'XOpenDisplay'", u8"ERROR", DialogOK);
	else{
		m_Context.m_Root = DefaultRootWindow(m_Context.m_Display);
		WindowAttrib.colormap = CopyFromParent;
		WindowAttrib.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask;
		if ((m_Context.m_Window = XCreateWindow(m_Context.m_Display, m_Context.m_Root, m_Position.x, m_Position.y, m_Size.x, m_Size.y, 0, CopyFromParent, InputOutput, CopyFromParent, CWColormap | CWEventMask, &WindowAttrib)) == 0) MakeDialog(u8"Error: 'XCreateWindow'", u8"ERROR", DialogOK);
		else{
			const char *AtomNames[] = X11_ATOM_NAMES;
			if (XInternAtoms(m_Context.m_Display, (char**)AtomNames, X11_ATOM_COUNT, true, m_Context.m_AtomList) == 0) MakeDialog(u8"Error: 'XInternAtoms'", u8"ERROR", DialogOK);
			else{
				XSetWMProtocols(m_Context.m_Display, m_Context.m_Window, &m_Context.m_AtomList[X11_WM_DELETE_WINDOW], 1);
				if ((m_Flag&Borderless) != 0){
					Atom BorderProps[] = { 2, 0, 0, 0, 0 };
					XChangeProperty(m_Context.m_Display, m_Context.m_Window, m_Context.m_AtomList[X11_MOTIF_WM_HINTS], m_Context.m_AtomList[X11_MOTIF_WM_HINTS], 32, PropModeReplace, (uint8_t*)&BorderProps, 5);
				}
				if ((m_Flag&Fullscreen) != 0) XChangeProperty(m_Context.m_Display, m_Context.m_Window, m_Context.m_AtomList[X11_NET_WM_STATE], XA_ATOM, 32, PropModeReplace, (uint8_t*)&m_Context.m_AtomList[X11_NET_WM_STATE_FULLSCREEN], 1);
				XMapWindow(m_Context.m_Display, m_Context.m_Window);
				XStoreName(m_Context.m_Display, m_Context.m_Window, m_Title.c_str());
				XQueryTree(m_Context.m_Display, m_Context.m_Window, &m_Context.m_Root, &m_Context.m_Parent, &ChildWindows, &ChildWindowCnt);
				XFree(ChildWindows);
				if (m_Context.m_Parent != m_Context.m_Root) {
					if (!XTranslateCoordinates(m_Context.m_Display, m_Context.m_Parent, m_Context.m_Root, Position.x, Position.y, &Pos.x, &Pos.y, &Child)) LWLogWarn("Error translating window.");
				}
				XMoveWindow(m_Context.m_Display, m_Context.m_Window, Pos.x, Pos.y);
				m_Flag |= Focused;
				ClipboardContext = &m_Context;
				ClipboardWindow = this;
				m_Flag ^= Error;
			}
		}
		m_Flag ^= Error;
	}
	if ((m_Flag&Error) == 0) {
		if (Flag&MouseDevice) m_MouseDevice = AttachInputDevice(Allocator.Create<LWMouse>())->AsMouse();
		if (Flag&KeyboardDevice) m_KeyboardDevice = AttachInputDevice(Allocator.Create<LWKeyboard>())->AsKeyboard();
	}
}

LWWindow::~LWWindow(){

	if (m_Context.m_Window != 0) XDestroyWindow(m_Context.m_Display, m_Context.m_Window);
	if (m_Context.m_Display) XCloseDisplay(m_Context.m_Display);

	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
	if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
	if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);
}


