#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include <iostream>

uint32_t LWWindow::MakeDialog(const LWText &Text, const LWText &Header, uint32_t DialogFlags){
    std::cout << "No dialog is available for this platform:'" << Text.GetCharacters() << "'" << std::endl;
    /*
	uint32_t DFlag = MB_ICONINFORMATION;
	if (DialogFlags&DialogCancel){
		if (DialogFlags&DialogOK) DFlag |= MB_OKCANCEL;
		if (DFlag&(DialogYES | DialogNo)) DFlag |= MB_YESNOCANCEL;
	}
	else if (DialogFlags&DialogOK) DFlag |= MB_OK;
	else if (DialogFlags&(DialogYES | DialogNo)) DFlag |= MB_YESNO;
	int32_t Result = MessageBox(nullptr, (const char*)Text.GetCharacters(), (const char*)Header.GetCharacters(), DFlag);
	if (Result == IDCANCEL) return DialogCancel;
	else if (Result == IDOK)     return DialogOK;
	else if (Result == IDYES)    return DialogYES;
	else if (Result == IDNO)     return DialogNo;
	return 0;
     */
    return 0;
}

bool LWWindow::MakeSaveFileDialog(const LWText &Filter, char *Buffer, uint32_t BufferLen) {
	return false;
}

bool LWWindow::MakeLoadFileDialog(const LWText &Filter, char *Buffer, uint32_t BufferLen) {
	return false;
}

uint32_t LWWindow::MakeLoadFileMultipleDialog(const LWText &Filter, char **Bufer, uint32_t BufferLen, uint32_t BufferCount) {
	return 0;
}

bool LWWindow::WriteClipboardText(const LWText &Text) {
	NSPasteboard *Paste = [NSPasteboard generalPasteboard];
	[Paste clearContents];
	[Paste setString:[NSString stringWithUTF8String : (const char*)Text.GetCharacters()] forType : NSPasteboardTypeString];
	return true;
}

uint32_t LWWindow::ReadClipboardText(char *Buffer, uint32_t BufferLen) {
	NSPasteboard *Paste = [NSPasteboard generalPasteboard];
	NSString *Str = [Paste stringForType : NSPasteboardTypeString];
	strncpy(Buffer, [Str UTF8String], BufferLen);
	return strlen(Buffer);
}

LWWindow &LWWindow::SetTitle(const LWText &Title){
	m_Title.Set(Title.GetCharacters());
    [m_Context.Window setTitle:[NSString stringWithUTF8String:(const char*)Title.GetCharacters()]];
	return *this;
}

LWWindow &LWWindow::SetTitlef(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return SetTitle(Buffer);
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position){
    m_Position = Position;
    NSRect ViewSize = [m_Context.Window frame];
    NSRect ScreenSize = [[m_Context.Window screen] frame];
    [m_Context.Window setFrameOrigin:NSMakePoint(m_Position.x, (ScreenSize.size.height-ViewSize.size.height-m_Position.y))];
    m_Flag |= PosChanged;
    return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size){
    NSRect ScreenSize = [[m_Context.Window screen] frame];
    m_Size = Size;
    NSRect Rect = NSMakeRect(m_Position.x, (ScreenSize.size.height-Size.y-m_Position.y), Size.x, Size.y);
    [m_Context.Window setFrame:Rect display:false];
    m_Flag |= SizeChanged;
	return *this;
}

LWWindow &LWWindow::SetVisible(bool isVisible){
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused){
	return *this;
}

LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position){
    CGWarpMouseCursorPosition(CGPointMake((float)(m_Position.x+Position.x), (float)(m_Position.y+m_Size.y-Position.y)));
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool isVisible){
    if(isVisible && (m_Flag&MouseVisible)==0) [NSCursor hide];
    else if(!isVisible && (m_Flag&MouseVisible)!=0) [NSCursor unhide];
	m_Flag = (m_Flag&~MouseVisible) | (isVisible ? MouseVisible : 0);
	return *this;
}


LWWindow &LWWindow::SetActiveGamepad(LWGamePad *Gamepad) {
	m_ActiveGamepad = Gamepad;
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool isFullscreen) {
	return *this; //Borderless well require a bit more work on mac, added to TODO list.
}

LWInputDevice *LWWindow::AttachInputDevice(LWInputDevice *Device){
	Device->SetNext(m_FirstDevice);
	m_FirstDevice = Device;
	return Device;
}

LWWindow &LWWindow::OpenKeyboard(uint32_t KeyboardType){
    return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t CursorPosition, uint32_t EditSize){
    return *this;
}

LWWindow &LWWindow::CloseKeyboard(void){
    return *this;
}

LWWindow &LWWindow::GetKeyboardEditRange(uint32_t &CursorPosition, uint32_t &EditSize){
    return *this;
}

uint32_t LWWindow::GetKeyboardText(char *Buffer, uint32_t BufferLen){
	if (Buffer) *Buffer = '\0';
	return 0;
}

LWWindow &LWWindow::SetKeyboardText(const char *Text){
    return *this;
}

LWVector4f LWWindow::GetKeyboardLayout(void){
    return LWVector4f();
}

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime){
	for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()){
		if (Device->ProcessSystemMessage(Message, MessageData, lCurrentTime, this)) return true;
    }
    return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime){
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);
    for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);
    NSEvent *E = nullptr;
    while((E = [m_Context.App nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:NSEventTrackingRunLoopMode dequeue:true])){
        ProcessWindowMessage((uint32_t)[E type], (__bridge void*)E, lCurrentTime);
        [m_Context.App sendEvent:E];
    }
    [m_Context.App updateWindows];
 
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

const LWText &LWWindow::GetTitle(void) const{
	return m_Title;
}

const LWText &LWWindow::GetName(void) const{
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

bool LWWindow::isVirtualKeyboardPresent(void) const {
	return (m_Flag&KeyboardPresent) != 0;
}

LWWindow::LWWindow(const LWText &Title, const LWText &Name, LWAllocator &Allocator, uint32_t Flag, const LWVector2i &Position, const LWVector2i &Size) : m_Allocator(&Allocator), m_FirstDevice(nullptr), m_Title(LWText(Title.GetCharacters(), Allocator)), m_Name(LWText(Name.GetCharacters(), Allocator)), m_Position(Position), m_Size(Size), m_Flag(Flag){
	m_MouseDevice = nullptr;
	m_KeyboardDevice = nullptr;
	m_TouchDevice = nullptr;
	m_AccelerometerDevice = nullptr;
	m_GyroscopeDevice = nullptr;
	m_ActiveGamepad = nullptr;
	memset(m_GamepadDevice, 0, sizeof(m_GamepadDevice));
	m_Context.App = [NSApplication sharedApplication];
    NSRect Frame = NSMakeRect(m_Position.x, m_Position.y, m_Size.x, m_Size.y);
    NSUInteger Style = (m_Flag&Borderless)!=0?NSBorderlessWindowMask:(NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask);
    NSNotificationCenter *NotCenter = [NSNotificationCenter defaultCenter];
    NSOperationQueue *NotQueue = [NSOperationQueue mainQueue];
    m_Context.Window = [[NSWindow alloc] initWithContentRect:Frame styleMask:Style backing:NSBackingStoreBuffered defer:false];
    if(m_Context.Window!=nullptr){
        NSRect ViewSize = [[m_Context.Window contentView] frame];
        NSRect ScreenSize = [[m_Context.Window screen] frame];
        [m_Context.Window setFrameOrigin:NSMakePoint(m_Position.x, (ScreenSize.size.height-ViewSize.size.height-m_Position.y))];
        [m_Context.Window setAcceptsMouseMovedEvents:true];
        [m_Context.Window setTitle:[NSString stringWithUTF8String:(const char*)m_Title.GetCharacters()]];
        [m_Context.Window setBackgroundColor:[NSColor blackColor]];
        [m_Context.Window setOpaque:true];
        if((m_Flag&Fullscreen)!=0) [m_Context.Window setLevel:NSMainMenuWindowLevel+1];
        [m_Context.Window makeKeyAndOrderFront:m_Context.Window];
        ViewSize = [[m_Context.Window contentView] frame];
        m_Size.x = ViewSize.size.width;
        m_Size.y = ViewSize.size.height;
        
        [NotCenter addObserverForName:NSWindowWillCloseNotification object:m_Context.Window queue:NotQueue usingBlock:^(NSNotification *note){ m_Flag|=Terminate;}];
        [NotCenter addObserverForName:NSWindowDidMoveNotification object:m_Context.Window queue:NotQueue usingBlock:^(NSNotification *note){
            NSRect WndFrame = [m_Context.Window frame];
            NSRect ScreenFrame = [[m_Context.Window screen] frame];
            m_Position = LWVector2i(WndFrame.origin.x, ScreenFrame.size.height-(WndFrame.origin.y+WndFrame.size.height));
            m_Flag|=PosChanged;
        }];
        [NotCenter addObserverForName:NSWindowDidResizeNotification object:m_Context.Window queue:NotQueue usingBlock:^(NSNotification *note){
            NSRect WndFrame = [[m_Context.Window contentView] frame];
            m_Size = LWVector2i(WndFrame.size.width, WndFrame.size.height);
            m_Flag|=SizeChanged;
        }];
        [NotCenter addObserverForName:NSApplicationDidResignActiveNotification object:m_Context.App queue:NotQueue usingBlock:^(NSNotification *Note){ m_Flag=(m_Flag&~Focused)|FocusChanged; }];
        [NotCenter addObserverForName:NSApplicationDidBecomeActiveNotification object:m_Context.App queue:NotQueue usingBlock:^(NSNotification *Note){ m_Flag|=(Focused|FocusChanged);}];
		m_Flag |= Focused;
        m_Flag^=Error;
    }else MakeDialog(LWText("Error: 'NSWindow initWithContentRect'"), LWText("ERROR"), DialogOK);
	m_Flag ^= Error;
	if ((m_Flag&Error) == 0) {
		if (Flag&MouseDevice) m_MouseDevice = AttachInputDevice(Allocator.Allocate<LWMouse>())->AsMouse();
		if (Flag&KeyboardDevice) m_KeyboardDevice = AttachInputDevice(Allocator.Allocate<LWKeyboard>())->AsKeyboard();
	}
}

LWWindow::~LWWindow() {
	m_Context.Window = nullptr;
	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
	if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
	if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);
}


