#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWCore/LWAllocator.h"
#include <ShellScalingApi.h>
#include <LWCore/LWTimer.h>
#include <iostream>

uint32_t LWWindow::MakeDialog(const LWUTF8Iterator &Text, const LWUTF8Iterator &Header, uint32_t DialogFlags){
	uint32_t DFlag = MB_ICONINFORMATION;
	if (DialogFlags&DialogCancel){
		if (DialogFlags&DialogOK) DFlag |= MB_OKCANCEL;
		if (DFlag&(DialogYES | DialogNo)) DFlag |= MB_YESNOCANCEL;
	}else if (DialogFlags&DialogOK) DFlag |= MB_OK;
	else if (DialogFlags&(DialogYES | DialogNo)) DFlag |= MB_YESNO;
	int32_t Result = MessageBox(nullptr, *Text.c_str<256>(), *Header.c_str<256>(), DFlag);
	if      (Result == IDCANCEL) return DialogCancel;
	else if (Result == IDOK)     return DialogOK;
	else if (Result == IDYES)    return DialogYES;
	else if (Result == IDNO)     return DialogNo;
	return 0;
}

bool LWWindow::MakeSaveFileDialog(const LWUTF8Iterator &FilterText, char8_t *Buffer, uint32_t BufferLen) {
	const uint32_t MaxFilters = 32;
	const uint32_t WBufferLen = 512;
	COMDLG_FILTERSPEC Filters[MaxFilters];
	LWUTF8Iterator FilterList[MaxFilters*2];
	uint32_t FilterCnt = 0;
	char16_t WBuffer[WBufferLen];
	uint32_t o = 0;
	FILEOPENDIALOGOPTIONS Options = 0;
	IShellItem *Result;
	FilterCnt = std::min<uint32_t>(FilterText.SplitToken(FilterList, MaxFilters * 2, ':')/2, MaxFilters);
	for (uint32_t i = 0; i < FilterCnt; i++) {
		Filters[i].pszSpec = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 0].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
		Filters[i].pszName = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 1].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
	}
	IFileSaveDialog *Dialog;
	if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&Dialog)))) {
		return false;
	}
	Dialog->GetOptions(&Options);
	Dialog->SetOptions(FOS_NOCHANGEDIR | FOS_OVERWRITEPROMPT | Options);
	Dialog->SetFileTypes(FilterCnt, Filters);
	HRESULT Res = Dialog->Show(nullptr);
	if (FAILED(Res)) return false;
	Dialog->GetResult(&Result);
	if (!Result) return false;
	PWSTR ResultStr;
	Result->GetDisplayName(SIGDN_FILESYSPATH, &ResultStr);
	if (ResultStr) LWUTF16Iterator((char16_t*)ResultStr).MakeUTF<char8_t>(Buffer, BufferLen);
	Result->Release();
	return ResultStr != nullptr;
}

bool LWWindow::MakeLoadFileDialog(const LWUTF8Iterator &FilterText, char8_t *Buffer, uint32_t BufferLen) {
	const uint32_t MaxFilters = 32;
	const uint32_t WBufferLen = 512;
	COMDLG_FILTERSPEC Filters[MaxFilters];
	LWUTF8Iterator FilterList[MaxFilters * 2];
	uint32_t FilterCnt = 0;
	char16_t WBuffer[WBufferLen];
	uint32_t o = 0;
	FILEOPENDIALOGOPTIONS Options = 0;
	IShellItem *Result;
	FilterCnt = std::min<uint32_t>(FilterText.SplitToken(FilterList, MaxFilters * 2, ':') / 2, MaxFilters);
	for (uint32_t i = 0; i < FilterCnt; i++) {
		Filters[i].pszSpec = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 0].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
		Filters[i].pszName = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 1].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
	}
	IFileOpenDialog *Dialog;
	CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&Dialog);
	Dialog->GetOptions(&Options);
	Dialog->SetOptions(FOS_NOCHANGEDIR|Options);
	Dialog->SetFileTypes(FilterCnt, Filters);
	HRESULT Res = Dialog->Show(nullptr); //Calling Dialog->Release() crashs when the program switch's context and back.
	if (FAILED(Res)) {
		return false;
	}
	Dialog->GetResult(&Result);
	PWSTR ResultStr;
	Result->GetDisplayName(SIGDN_FILESYSPATH, &ResultStr);
	if (ResultStr) LWUTF16Iterator((char16_t*)ResultStr).MakeUTF<char8_t>(Buffer, BufferLen);
	Result->Release();
	return ResultStr!=nullptr;
}


uint32_t LWWindow::MakeLoadFileMultipleDialog(const LWUTF8Iterator &FilterText, char8_t **Buffer, uint32_t BufferLen, uint32_t BufferCount) {
	const uint32_t MaxFilters = 32;
	const uint32_t WBufferLen = 512;
	COMDLG_FILTERSPEC Filters[MaxFilters];
	LWUTF8Iterator FilterList[MaxFilters * 2];
	uint32_t FilterCnt = 0;
	char16_t WBuffer[WBufferLen];
	uint32_t o = 0;
	FILEOPENDIALOGOPTIONS Options = 0;
	IShellItemArray *Result;
	FilterCnt = std::min<uint32_t>(FilterText.SplitToken(FilterList, MaxFilters * 2, ':') / 2, MaxFilters);
	for (uint32_t i = 0; i < FilterCnt; i++) {
		Filters[i].pszSpec = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 0].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
		Filters[i].pszName = (const wchar_t*)WBuffer + o;
		o += FilterList[i * 2 + 1].MakeUTF<char16_t>(WBuffer + o, WBufferLen - o);
	}
	IFileOpenDialog *Dialog;
	CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&Dialog);
	Dialog->GetOptions(&Options);
	Dialog->SetOptions(FOS_NOCHANGEDIR | Options | FOS_ALLOWMULTISELECT);
	Dialog->SetFileTypes(FilterCnt, Filters);
	HRESULT Res = Dialog->Show(nullptr); //Calling Dialog->Release() crashs when the program switch's context and back.
	if (FAILED(Res)) {
		return 0;
	}
	Dialog->GetResults(&Result);
	DWORD Cnt = 0;
	Result->GetCount(&Cnt);
	Cnt = std::min<uint32_t>((uint32_t)Cnt, BufferCount);
	for (uint32_t i = 0; i < Cnt; i++) {
		IShellItem *pItem = nullptr;
		Result->GetItemAt(i, &pItem);
		PWSTR ResultStr = nullptr;
		pItem->GetDisplayName(SIGDN_FILESYSPATH, &ResultStr);
		if (ResultStr) LWUTF16Iterator((char16_t*)ResultStr).MakeUTF<char8_t>(Buffer[i], BufferLen);
		pItem->Release();
	}
	Result->Release();
	return Cnt;
}

bool LWWindow::WriteClipboardText(const LWUTF8Iterator &Text) {
	if (!OpenClipboard(nullptr)) return false;
	if (!EmptyClipboard()) return false;
	
	uint32_t Len16 = Text.MakeUTF<char16_t>(nullptr, 0);
	HGLOBAL cMem = GlobalAlloc(GMEM_MOVEABLE, Len16 * sizeof(char16_t));
	char16_t *Mem = (char16_t*)GlobalLock(cMem);
	if (Text.MakeUTF<char16_t>(Mem, Len16) != Len16) return false;
	GlobalUnlock(cMem);
	if (!SetClipboardData(CF_UNICODETEXT, cMem)) return false;
	CloseClipboard();
	return true;
}

uint32_t LWWindow::ReadClipboardText(char8_t *Buffer, uint32_t BufferLen) {
	if (!OpenClipboard(nullptr)) return 0;
	HANDLE Res = GetClipboardData(CF_UNICODETEXT);
	if (!Res) return 0;
	const char16_t *Data = (const char16_t*)GlobalLock(Res);
	uint32_t Len = LWUTF16Iterator(Data).MakeUTF<char8_t>(Buffer, BufferLen);
	GlobalUnlock(Res);
	CloseClipboard();
	return Len;
}

LWWindow &LWWindow::SetTitle(const LWUTF8Iterator &Title){
	m_Title = Title;
	SetWindowText(m_Context.m_WND, *Title.c_str<256>());
	return *this;
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position){
	
	RECT cRect; //Client rect.
	RECT wRect; //Window rect.
	GetClientRect(m_Context.m_WND, &cRect);
	GetWindowRect(m_Context.m_WND, &wRect);
	MoveWindow(m_Context.m_WND, wRect.left + (Position.x - m_Position.x), wRect.top + (Position.y - m_Position.y) , m_Size.x + (wRect.right - wRect.left) - cRect.right, m_Size.y + (wRect.bottom - wRect.top) - cRect.bottom, false);
	
	m_Position = Position;
	m_Flag |= PosChanged;
	return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size){
	RECT cRect; //Client rect.
	RECT wRect; //Window rect.
	GetClientRect(m_Context.m_WND, &cRect);
	GetWindowRect(m_Context.m_WND, &wRect);
	MoveWindow(m_Context.m_WND, wRect.left, wRect.top, Size.x + (wRect.right - wRect.left) - cRect.right, Size.y + (wRect.bottom - wRect.top) - cRect.bottom, false);
	GetClientRect(m_Context.m_WND, &cRect);
	m_Size = LWVector2i((cRect.right - cRect.left), (cRect.bottom - cRect.top));
	m_Flag |= SizeChanged;
	return *this;
}

LWWindow &LWWindow::SetVisible(bool isVisible){
	ShowWindow(m_Context.m_WND, isVisible ? SW_SHOW : SW_HIDE);
	m_Flag = (m_Flag&~Visible) | (isVisible ? Visible : 0);
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused){
	SetFocus(isFocused ? m_Context.m_WND : nullptr);
	SetForegroundWindow(isFocused ? m_Context.m_WND : nullptr);
	m_Flag = (m_Flag&~Focused) | (isFocused ? Focused : 0) | FocusChanged;
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool) {
	if (isBorderless && (m_Flag&Borderless)) return *this;
	else if (!isBorderless && (m_Flag&Borderless) == 0) return *this;
	uint32_t BorderFlag = !isBorderless ? WS_OVERLAPPEDWINDOW : WS_POPUP;
	uint32_t BorderFlagEx = !isBorderless ? (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) : WS_EX_APPWINDOW;
	HWND zOrder = HWND_TOP;
	RECT wRect;
	RECT cRect;
	LWVector2i PrevSize = m_Size;
	GetWindowRect(m_Context.m_WND, &wRect);
	GetClientRect(m_Context.m_WND, &cRect);

	if (!SetWindowLongPtr(m_Context.m_WND, GWL_STYLE, BorderFlag)) return *this;
	if (!SetWindowLongPtr(m_Context.m_WND, GWL_EXSTYLE, BorderFlagEx)) return *this;
	if (isBorderless) {
		int32_t BorderWidth = (wRect.right - wRect.left) - cRect.right;
		int32_t BorderHeight = (wRect.bottom - wRect.top) - cRect.bottom;
		int32_t TitleHeight = BorderHeight - BorderWidth / 2;
		SetWindowPos(m_Context.m_WND, zOrder, BorderWidth/2 + m_Position.x, m_Position.y+TitleHeight, PrevSize.x, PrevSize.y, SWP_FRAMECHANGED);
	} else {
		SetWindowPos(m_Context.m_WND, zOrder, m_Position.x, m_Position.y, PrevSize.x, PrevSize.y, SWP_FRAMECHANGED);
		GetWindowRect(m_Context.m_WND, &wRect);
		GetClientRect(m_Context.m_WND, &cRect);
		int32_t BorderWidth = (wRect.right - wRect.left) - cRect.right;
		int32_t BorderHeight = (wRect.bottom - wRect.top) - cRect.bottom;
		int32_t TitleHeight = BorderHeight - BorderWidth/2;
		SetWindowPos(m_Context.m_WND, zOrder, m_Position.x-BorderWidth/2, m_Position.y-TitleHeight, PrevSize.x + BorderWidth, PrevSize.y + BorderHeight, SWP_FRAMECHANGED);
	}
	ShowWindow(m_Context.m_WND, (m_Flag&Visible) ? SW_SHOW : SW_HIDE);
	
	m_Flag = (m_Flag&~Borderless) | (isBorderless ? Borderless : 0) | SizeChanged | PosChanged;
	return *this;
}


LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position){
	POINT pt = { Position.x, m_Size.y-Position.y };
	ClientToScreen(m_Context.m_WND, &pt);
	LWVector2i Diff = LWVector2i(pt.x- Position.x, pt.y- Position.y);
	SetCursorPos(pt.x, pt.y);
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool isVisible) {
	if (isVisible && (m_Flag&MouseVisible) == 0) ShowCursor(isVisible);
	else if (!isVisible && (m_Flag&MouseVisible) != 0) ShowCursor(isVisible);
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

LWWindow &LWWindow::OpenKeyboard(uint32_t ){
	m_Flag |= KeyboardPresent;
	return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t , uint32_t ){
	return *this;
}

LWWindow &LWWindow::SetKeyboardText(const LWUTF8Iterator &) {
	return *this;
}

LWWindow &LWWindow::CloseKeyboard(void){
	m_Flag &= ~KeyboardPresent;
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

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageParam, uint64_t lCurrentTime){
	MSG *M = (MSG*)MessageParam;
	if (Message == WM_CLOSE || Message == WM_QUIT){
		m_Flag |= Terminate;
		return true;
	}else if (Message == WM_KILLFOCUS){
		m_Flag = (m_Flag | FocusChanged)&~Focused;
		return true;
	}else if (Message == WM_SETFOCUS){
		m_Flag = (m_Flag | FocusChanged | Focused);
		return true;
	}else if (Message == WM_SIZE){
		m_Flag |= SizeChanged;
		m_Size = LWVector2i(M->lParam & 0xFFFF, (M->lParam >> 16) & 0xFFFF);
		return true;
	} else if (Message == WM_MOVE) {
		m_Flag |= PosChanged;
		m_Position = LWVector2i(M->lParam & 0xFFFF, (M->lParam >> 16) & 0xFFFF);
		return true;
	} else if (Message == WM_WINDOWPOSCHANGED) {
		WINDOWPOS *WPos = (WINDOWPOS*)M->lParam;
		RECT cRect;
		RECT wRect;
		GetClientRect(m_Context.m_WND, &cRect);
		GetWindowRect(m_Context.m_WND, &wRect);
		LWVector2i NewSize = LWVector2i(cRect.right, cRect.bottom);
		LWVector2i NewPos = LWVector2i(wRect.left, wRect.top);
		if (m_Size != NewSize) m_Flag |= SizeChanged;
		if (m_Position != NewPos) m_Flag |= PosChanged;
		m_Size = NewSize;
		m_Position = NewPos;
		return true;
	}else{
		for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()){
			if (Device->ProcessSystemMessage(Message, MessageParam, lCurrentTime, this)) return true;
		}
	}
	return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime){
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);
	MSG M;
	for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);

	while (PeekMessage(&M, m_Context.m_WND, 0, 0, PM_REMOVE)){
		if(!ProcessWindowMessage(M.message, &M, lCurrentTime)){
			TranslateMessage(&M);
			DispatchMessage(&M);
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

LWVector2i LWWindow::GetPosition(void) const{
	return m_Position;
}

LWVector2f LWWindow::GetPositionf(void) const {
	return LWVector2f((float)m_Position.x, (float)m_Position.y);
}

LWVector2i LWWindow::GetSize(void) const{
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
	m_Context.m_Instance = GetModuleHandle(nullptr);
	HICON AppIcon = LoadIcon(m_Context.m_Instance, IDI_APPLICATION);
	m_Context.m_WND = nullptr;

	auto WndCallBack = [](HWND h, UINT m, WPARAM w, LPARAM l)->LRESULT{
		LWWindow *Wnd = (LWWindow*)GetWindowLongPtr(h, GWLP_USERDATA);
		MSG M = { h, m, w, l, 0, { 0, 0 } };
		if (Wnd && Wnd->ProcessWindowMessage(m, &M, LWTimer::GetCurrent())) return true;
		return DefWindowProc(h, m, w, l);
	};
	WNDCLASS WndClass = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WndCallBack, 0, 0, m_Context.m_Instance, LoadIcon(nullptr, IDI_WINLOGO), LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, m_Name.c_str() };
	uint32_t BorderFlag = (m_Flag&Borderless)==0 ? WS_OVERLAPPEDWINDOW : WS_POPUP;
	uint32_t BorderFlagEx = (m_Flag&Borderless) == 0 ? (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE) : WS_EX_APPWINDOW;
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
	RECT WndRect = { m_Position.x, m_Position.y, m_Position.x + m_Size.x, m_Position.y + m_Size.y };

	if (!RegisterClass(&WndClass)) MakeDialog(u8"Error: 'RegisterClass'", u8"ERROR", DialogOK);
	else if (!AdjustWindowRectEx(&WndRect, BorderFlag, 0, BorderFlagEx)) MakeDialog(u8"Error: 'AdjustWindowRectEx'", u8"ERROR", DialogOK);
	else if ((m_Context.m_WND = CreateWindowEx(BorderFlagEx, m_Name.c_str(),  m_Title.c_str(), WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BorderFlag, WndRect.left, WndRect.top, WndRect.right - WndRect.left, WndRect.bottom - WndRect.top, nullptr, nullptr, m_Context.m_Instance, nullptr)) == nullptr) MakeDialog(u8"Error: 'CreateWindowEx'", u8"ERROR", DialogOK);
	else {
		SetForegroundWindow(m_Context.m_WND);
		SetFocus(m_Context.m_WND);
		ShowWindow(m_Context.m_WND, (m_Flag&Visible) ? SW_SHOW : SW_HIDE);
		if ((m_Flag&MouseVisible) == 0) ShowCursor(false);
		SetWindowLongPtr(m_Context.m_WND, GWLP_USERDATA, (LONG_PTR)this);
		SendMessage(m_Context.m_WND, WM_SETICON, ICON_SMALL, (LPARAM)AppIcon);
		SendMessage(m_Context.m_WND, WM_SETICON, ICON_BIG, (LPARAM)AppIcon);
		m_Flag ^= (Error | Focused);
	}
	m_Flag ^= Error;

	//Disable accessibility key prompts if they are already off, we do this even on error as destroying the window will cause the keys to revert anyway.
	m_Context.m_StickyKeyState = { sizeof(STICKYKEYS), 0 };
	m_Context.m_FilterKeyState = { sizeof(FILTERKEYS), 0 };
	m_Context.m_ToggleKeyState = { sizeof(TOGGLEKEYS), 0 };
	SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_Context.m_StickyKeyState, 0);
	SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_Context.m_ToggleKeyState, 0);
	SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &m_Context.m_FilterKeyState, 0);
	STICKYKEYS skOff = m_Context.m_StickyKeyState;
	FILTERKEYS fkOff = m_Context.m_FilterKeyState;
	TOGGLEKEYS tkOff = m_Context.m_ToggleKeyState;
	if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0) {
		skOff.dwFlags &= ~(SKF_HOTKEYACTIVE | SKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
	}
	if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0) {
		fkOff.dwFlags &= ~(FKF_HOTKEYACTIVE | FKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
	}
	if ((tkOff.dwFlags&TKF_TOGGLEKEYSON) == 0) {
		tkOff.dwFlags &= ~(TKF_HOTKEYACTIVE | TKF_CONFIRMHOTKEY);
		SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
	}

	if ((m_Flag&Error) == 0) {
		if (Flag&MouseDevice) m_MouseDevice = AttachInputDevice(Allocator.Create<LWMouse>())->AsMouse();
		if (Flag&KeyboardDevice) m_KeyboardDevice = AttachInputDevice(Allocator.Create<LWKeyboard>())->AsKeyboard();
		if (Flag&GamepadDevice) {
			for (uint32_t i = 0; i < MAXGAMEPADS; i++) m_GamepadDevice[i] = AttachInputDevice(Allocator.Create<LWGamePad>(i))->AsGamepad();
			m_ActiveGamepad = m_GamepadDevice[0];
		}
	}
}

LWWindow::~LWWindow(){
	if (m_Context.m_WND) DestroyWindow(m_Context.m_WND);
	if (m_Context.m_Instance) UnregisterClass(m_Name.c_str(), m_Context.m_Instance);

	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
	if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
	if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);

	//Reenable accessibility key prompts to their original values.
	SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_Context.m_StickyKeyState, 0);
	SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_Context.m_ToggleKeyState, 0);
	SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &m_Context.m_FilterKeyState, 0);
}

