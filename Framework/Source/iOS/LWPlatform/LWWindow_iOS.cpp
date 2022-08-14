#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWVideo/LWVideoDriver.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWLogger.h"
#include <iostream>

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

uint32_t LWWindow::MakeDialog(const LWUTF8Iterator &Text, const LWText &Header, uint32_t DialogFlags){
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
	[UIPasteboard generalPasteboard].string = [NSString stringWithUTF8String : *Text.c_str<256>()];

	return true;
}

uint32_t LWWindow::ReadClipboardText(char8_t *Buffer, uint32_t BufferLen) {
    const char *Str = [[UIPasteboard generalPasteboard]UTF8String];
	strlcpy((char*)Buffer, Str, BufferLen);
	return strlen(Str)+1;
}

LWWindow &LWWindow::SetTitle(const LWUTF8Iterator &Title){
    m_Title = Title;
	return *this;
}

LWWindow &LWWindow::SetPosition(const LWVector2i &Position){
    return *this;
}

LWWindow &LWWindow::SetSize(const LWVector2i &Size){
	return *this;
}

LWWindow &LWWindow::SetVisible(bool iVisible){
	return *this;
}

LWWindow &LWWindow::SetFocused(bool isFocused){
	return *this;
}

LWWindow &LWWindow::SetMousePosition(const LWVector2i &Position){
	return *this;
}

LWWindow &LWWindow::SetMouseVisible(bool iVisible){
	return *this;
}

LWInputDevice *LWWindow::AttachInputDevice(LWInputDevice *Device){
	Device->SetNext(m_FirstDevice);
	m_FirstDevice = Device;
	return Device;
}

LWWindow &LWWindow::OpenKeyboard(uint32_t KeyboardType){
    LWIOSEvent Event = {LWIOSEventCode::KeyboardOpen};
    LWAppContext.m_AppLoop.Push(Event);
    return *this;
}

LWWindow &LWWindow::SetKeyboardEditRange(uint32_t CursorPosition, uint32_t EditSize){
    //UITextPosition *Begin = LWAppContext.m_KeyboardTextField.beginningOfDocument;
    //UITextPosition *End = [LWAppContext.m_KeyboardTextField positionFromPosition:Begin offset:CursorPosition];
    //[LWAppContext.m_KeyboardTextField offsetFromPosition:Begin toPosition:End];
    return *this;
}

LWWindow &LWWindow::SetActiveGamepad(LWGamePad *Gamepad) {
	m_ActiveGamepad = Gamepad;
	return *this;
}

LWWindow &LWWindow::SetBorderless(bool isBorderless, bool isFullscreen) {
	return *this; //Borderless well require a bit more work on mac, added to TODO list.
}

LWWindow &LWWindow::CloseKeyboard(void){
    LWIOSEvent Event = {LWIOSEventCode::KeyboardClose};
    LWAppContext.m_AppLoop.Push(Event);
    return *this;
}

LWWindow &LWWindow::GetKeyboardEditRange(uint32_t &CursorPosition, uint32_t &EditSize){
    UITextRange *SelRange = LWAppContext.m_KeyboardTextField.selectedTextRange;
    NSInteger CursorPos = [LWAppContext.m_KeyboardTextField offsetFromPosition:LWAppContext.m_KeyboardTextField.beginningOfDocument toPosition:SelRange.start];
    NSInteger SelectRange = [LWAppContext.m_KeyboardTextField offsetFromPosition:SelRange.start toPosition:SelRange.end];
    CursorPosition = (uint32_t)CursorPos;
    EditSize = (uint32_t)SelectRange;
    return *this;
}

uint32_t LWWindow::GetKeyboardText(char *Buffer, uint32_t BufferSize){
    const char *UTF8 = LWAppContext.m_KeyboardTextField.text.UTF8String;
	if (Buffer) strlcpy(Buffer, UTF8, BufferSize);
    return (uint32_t)strlen(UTF8)+1;
}

LWWindow &LWWindow::SetKeyboardText(const LWUTF8Iterator &Text){
    //NSString *s = [NSString stringWithCString:Text encoding:NSUTF8StringEncoding];
    //[LWAppContext.m_KeyboardTextField setText:s];
    return *this;
}

LWVector4f LWWindow::GetKeyboardLayout(void){
    float h = LWAppContext.m_KeyboardLayout.size.height*[[UIScreen mainScreen] scale];
    return LWVector4f(0.0f, 0.0f, 0.0f, h);
}

bool LWWindow::ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime){
    if(Message==(uint32_t)LWIOSEventCode::FocusGained){
		m_Flag |= Focused | FocusChanged;
        if((m_AccelerometerDevice && m_AccelerometerDevice->IsEnabled()) || (m_GyroscopeDevice && m_GyroscopeDevice->IsEnabled())){
            [LWAppContext.m_MotionManager startDeviceMotionUpdates];
        }
        return true;
    }else if(Message==(uint32_t)LWIOSEventCode::FocusLost){
        LWAppContext.m_SyncState = true;
        m_Flag = (m_Flag&~Focused)|FocusChanged;
        if((m_AccelerometerDevice && m_AccelerometerDevice->IsEnabled()) || (m_GyroscopeDevice && m_GyroscopeDevice->IsEnabled())){
            [LWAppContext.m_MotionManager stopDeviceMotionUpdates];
        }
		return true;
    }else if(Message==(uint32_t)LWIOSEventCode::OrientationChanged){
        UIInterfaceOrientation Direction = [LWAppContext.m_Applicaiton statusBarOrientation];
        uint32_t Longest = std::max<uint32_t>(m_Size.x, m_Size.y);
        uint32_t Shortest= std::min<uint32_t>(m_Size.x, m_Size.y);
        if((Direction==UIInterfaceOrientation::UIInterfaceOrientationPortrait) || Direction==UIInterfaceOrientationPortraitUpsideDown) m_Size = LWVector2i(Shortest, Longest);
        else m_Size = LWVector2i(Longest, Shortest);
		m_Flag = (m_Flag&~(Rotation_0|Rotation_90|Rotation_180|Rotation_270))|(Direction == UIInterfaceOrientation::UIInterfaceOrientationPortrait ? Rotation_0 : (Direction == UIInterfaceOrientation::UIInterfaceOrientationLandscapeLeft ? Rotation_270 : (Direction == UIInterfaceOrientation::UIInterfaceOrientationPortraitUpsideDown ? Rotation_180 : Rotation_90)))|OrientationChanged|SizeChanged;
        return true;
    }else if(Message==(uint32_t)LWIOSEventCode::Destroy){
        m_Flag |= Terminate;
        return true;
    }else if(Message==(uint32_t)LWIOSEventCode::KeyboardOpen){
        m_Flag|=KeyboardPresent;
    }else if(Message==(uint32_t)LWIOSEventCode::KeyboardClose){
        m_Flag&=~KeyboardPresent;
    }else{
        for(LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()){
            if(Device->ProcessSystemMessage(Message, MessageData, lCurrentTime, this)) return true;
        }
    }
    return false;
}

LWWindow &LWWindow::Update(uint64_t lCurrentTime){
	m_Flag &= (Terminate | Fullscreen | Visible | Focused | MouseVisible | Borderless | Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270 | KeyboardPresent);
    if(LWAppContext.m_SyncState){
        LWAppContext.m_EventState++;
        LWAppContext.m_SyncState = false;
    }
    for (LWInputDevice *Device = m_FirstDevice; Device; Device = Device->GetNext()) Device->Update(this, lCurrentTime);
    LWIOSEvent Event;
    while((m_Flag&Terminate)==0 && LWAppContext.m_EventLoop.Pop(Event)) ProcessWindowMessage(Event.m_EventCode, &Event, lCurrentTime);
    
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
	return (m_Flag&(Rotation_0 | Rotation_90 | Rotation_180 | Rotation_270));
}

LWWindowContext &LWWindow::GetContext(void){
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

LWWindow::LWWindow(const LWUTF8Iterator &Title, const LWUTF8Iterator &Name, LWAllocator &Allocator, uint32_t Flag, const LWVector2i &Position, const LWVector2i &Size) : m_Allocator(&Allocator), m_FirstDevice(nullptr), m_Title(Title, Allocator), m_Name(Name, Allocator), m_Position(Position), m_Size(Size), m_Flag(Flag){
	m_MouseDevice = nullptr;
	m_KeyboardDevice = nullptr;
	m_TouchDevice = nullptr;
	m_AccelerometerDevice = nullptr;
	m_GyroscopeDevice = nullptr;
	m_ActiveGamepad = nullptr;
	memset(m_GamepadDevice, 0, sizeof(m_GamepadDevice));
	if(LWAppContext.m_Window!=nullptr){
		CGRect WndRect = LWAppContext.m_Window.bounds;
		CGFloat Scale = [[UIScreen mainScreen] scale];
		UIViewController *VC = [[UIViewController alloc] init];
		LWAppContext.m_Window.rootViewController = VC;
		UIInterfaceOrientation Direction = [LWAppContext.m_Applicaiton statusBarOrientation];
        m_Size = LWVector2i(WndRect.size.width*Scale, WndRect.size.height*Scale);
        //std::cout << "window: " << WndRect.origin.x << " " << WndRect.origin.y << " " << WndRect.size.width << " " << WndRect.size.height << " " << Scale << std::endl;
		m_Flag |= Direction == UIInterfaceOrientation::UIInterfaceOrientationPortrait ? Rotation_0 : (Direction == UIInterfaceOrientation::UIInterfaceOrientationLandscapeLeft ? Rotation_270 : (Direction == UIInterfaceOrientation::UIInterfaceOrientationPortraitUpsideDown ? Rotation_180 : Rotation_90));
		m_Position = LWVector2i();
		LWAppContext.m_EventState++;
		m_Flag |= Focused;
        m_Flag^=Error;
    }
	m_Flag ^= Error;
	if ((m_Flag&Error) == 0) {
		if (Flag&TouchDevice) m_TouchDevice = AttachInputDevice(Allocator.Create<LWTouch>())->AsTouch();
		if (Flag&KeyboardDevice) m_KeyboardDevice = AttachInputDevice(Allocator.Create<LWKeyboard>())->AsKeyboard();
        LWAppContext.m_MotionManager.deviceMotionUpdateInterval = 1.0/60;
        if(Flag&(AccelerometerDevice|GyroscopeDevice) && LWAppContext.m_MotionManager.isDeviceMotionAvailable){
            if(Flag&AccelerometerDevice) m_AccelerometerDevice = AttachInputDevice(Allocator.Create<LWAccelerometer>())->AsAccelerometer();
            if(Flag&GyroscopeDevice) m_GyroscopeDevice = AttachInputDevice(Allocator.Create<LWGyroscope>(1.0f, 1.0f))->AsGyroscope();
            [LWAppContext.m_MotionManager startDeviceMotionUpdates];
        }
	}
}

LWWindow::~LWWindow() {
	if (m_MouseDevice) LWAllocator::Destroy(m_MouseDevice);
	if (m_KeyboardDevice) LWAllocator::Destroy(m_KeyboardDevice);
	if (m_TouchDevice) LWAllocator::Destroy(m_TouchDevice);
    if (m_AccelerometerDevice) LWAllocator::Destroy(m_AccelerometerDevice);
    if (m_GyroscopeDevice) LWAllocator::Destroy(m_GyroscopeDevice);
    if(m_AccelerometerDevice || m_GyroscopeDevice) [LWAppContext.m_MotionManager stopDeviceMotionUpdates];
	for (uint32_t i = 0; i < MAXGAMEPADS; i++) LWAllocator::Destroy(m_GamepadDevice[i]);
}
