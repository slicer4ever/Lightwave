#include "LWPlatform/LWInputDevice.h"
#include "LWCore/LWTimer.h"
#include <cstring>
#include <iostream>
#include "LWPlatform/LWWindow.h"

#pragma region LWInputDevice

LWMouse *LWInputDevice::AsMouse(void){
	return (LWMouse*)this;
}

LWKeyboard *LWInputDevice::AsKeyboard(void){
	return (LWKeyboard*)this;
}

LWTouch *LWInputDevice::AsTouch(void) {
	return (LWTouch*)this;
}

LWGamePad *LWInputDevice::AsGamepad(void) {
	return (LWGamePad*)this;
}

LWGyroscope *LWInputDevice::AsGyroscope(void) {
	return (LWGyroscope*)this;
}

LWAccelerometer *LWInputDevice::AsAccelerometer(void) {
	return (LWAccelerometer*)this;
}

LWInputDevice &LWInputDevice::SetNext(LWInputDevice *Next){
	m_Next = Next;
	return *this;
}

LWInputDevice *LWInputDevice::GetNext(void) const{
	return m_Next;
}

LWInputDevice::LWInputDevice() : m_Next(nullptr){}

#pragma endregion

#pragma region LWMouse

LWInputDevice &LWMouse::Update(LWWindow *, uint64_t lCurrentTime){
	m_DoubleClick = false;
	if (((m_CurrState^m_PrevState)&m_PrevState)) {
		if (LWTimer::ToMilliSecond(lCurrentTime - m_LastPressTime) < DoubleClickTime) {
			m_DoubleClick = true;
		}
		m_LastPressTime = lCurrentTime;
	}
	m_PrevState = m_CurrState;
	m_Scroll = 0;
	return *this;
}

bool LWMouse::ButtonUp(LWMouseKey Key) const{
	return (m_CurrState&(uint32_t)Key) == 0;
}

bool LWMouse::ButtonDown(LWMouseKey Key) const{
	return (m_CurrState&(uint32_t)Key) != 0;
}

bool LWMouse::ButtonReleased(LWMouseKey Key) const{
	return (m_PrevState&(uint32_t)Key) != 0 && (m_CurrState&(uint32_t)Key) == 0;
}

bool LWMouse::ButtonPressed(LWMouseKey Key) const{
	return (m_PrevState&(uint32_t)Key) == 0 && (m_CurrState&(uint32_t)Key) != 0;
}

bool LWMouse::DoubleClicked(void) const {
	return m_DoubleClick;
}

LWVector2i LWMouse::GetPosition(void) const{
	return m_Position;
}

LWVector2f LWMouse::GetPositionf(void) const {
	return LWVector2f((float)m_Position.x, (float)m_Position.y);
}

int32_t LWMouse::GetScroll(void) const{
	return m_Scroll;
}

LWMouse::LWMouse() : LWInputDevice(), m_Position(LWVector2i(-100)), m_Scroll(0), m_CurrState(0), m_PrevState(0), m_LastPressTime(0), m_DoubleClick(false){}

#pragma endregion

#pragma region LWKeyboard

LWInputDevice &LWKeyboard::Update(LWWindow *, uint64_t ){
	memcpy(m_PrevState, m_CurrState, sizeof(m_CurrState));
	m_KeyChangeCount = 0;
	m_CharPressed = 0;
	return *this;
}

bool LWKeyboard::ButtonDown(LWKey Key) const{
	return ButtonDown((uint32_t)Key);
}

bool LWKeyboard::ButtonDown(uint32_t Key) const{
	uint32_t Word = Key / (sizeof(uint32_t)* 8);
	uint32_t Bit = 1 << (Key % (sizeof(uint32_t)* 8));
	return (m_CurrState[Word] & Bit) != 0;
}

bool LWKeyboard::ButtonUp(LWKey Key) const{
	return ButtonUp((uint32_t)Key);
}

bool LWKeyboard::ButtonUp(uint32_t Key) const{
	uint32_t Word = Key / (sizeof(uint32_t)* 8);
	uint32_t Bit = 1 << (Key % (sizeof(uint32_t)* 8));
	return (m_CurrState[Word] & Bit) == 0;
}

bool LWKeyboard::ButtonPressed(LWKey Key) const{
	return ButtonPressed((uint32_t)Key);
}

bool LWKeyboard::ButtonPressed(uint32_t Key) const{
	uint32_t Word = Key / (sizeof(uint32_t)* 8);
	uint32_t Bit = 1 << (Key % (sizeof(uint32_t)* 8));
	return (m_CurrState[Word] & Bit) != 0 && (m_PrevState[Word] & Bit) == 0;
}

bool LWKeyboard::ButtonReleased(LWKey Key) const{
	return ButtonReleased((uint32_t)Key);
}

bool LWKeyboard::ButtonReleased(uint32_t Key) const{
	uint32_t Word = Key / (sizeof(uint32_t)* 8);
	uint32_t Bit = 1 << (Key % (sizeof(uint32_t)* 8));
	return (m_CurrState[Word] & Bit) == 0 && (m_PrevState[Word] & Bit) != 0;
}

uint32_t LWKeyboard::GetKeyChangeCount(void) const{
	return m_KeyChangeCount;
}

uint32_t LWKeyboard::GetKeyChanged(uint32_t i) const{
	return m_KeyChanges[i];
}

bool LWKeyboard::GetKeyState(uint32_t i) const {
	return m_KeyStates[i];
}

uint32_t LWKeyboard::GetChar(uint32_t i) const {
	return m_CharInputs[i];
}

uint32_t LWKeyboard::GetCharPressed(void) const {
	return m_CharPressed;
}

LWKeyboard::LWKeyboard() : m_KeyChangeCount(0){
	memset(m_CurrState, 0, sizeof(m_CurrState));
	memset(m_PrevState, 0, sizeof(m_PrevState));
}
#pragma endregion

#pragma region LWTouch

LWInputDevice &LWTouch::Update(LWWindow *, uint64_t lCurrentTime) {
	const uint64_t TapTime = LWTimer::GetResolution() / 2;
	const uint64_t FlickTime = LWTimer::GetResolution() / 4;
	const uint64_t DragTime = LWTimer::GetResolution();
	const uint32_t TapRadius = 32;

	if (m_PointCount == 1) {
		uint64_t TimeDown = lCurrentTime - m_Points[0].m_DownTime;
		if (m_Points[0].m_State == LWTouchPoint::UP) {
			if ((m_Points[0].m_Position - m_Points[0].m_InitPosition).LengthSquared() < TapRadius*TapRadius) {
				if (TimeDown < TapTime || m_Gesture.m_Type==LWGesture::Press) m_Gesture = { m_Points[0].m_Position, LWVector2f(), m_Points[0].m_Size, LWGesture::Tap };
				else m_Gesture = { LWVector2i(), LWVector2f(), m_Points[0].m_Size, LWGesture::None };
			} else {
				if (TimeDown < FlickTime) m_Gesture = { m_Points[0].m_InitPosition, LWVector2f((float)(m_Points[0].m_Position.x - m_Points[0].m_InitPosition.x), (float)(m_Points[0].m_Position.y - m_Points[0].m_InitPosition.y)), m_Points[0].m_Size, LWGesture::Flick };
				else m_Gesture = { LWVector2i(), LWVector2f(), m_Points[0].m_Size, LWGesture::None };
			}
		} else {
			if (TimeDown > FlickTime) {
				if ((m_Points[0].m_Position - m_Points[0].m_InitPosition).LengthSquared() < TapRadius*TapRadius) {
					m_Gesture = { m_Points[0].m_Position, LWVector2f(), m_Points[0].m_Size, LWGesture::Press };
				} else {
					if (m_Gesture.m_Type == LWGesture::Press || m_Gesture.m_Type == LWGesture::PressAndDrag) {
						m_Gesture = { m_Points[0].m_InitPosition, LWVector2f((float)(m_Points[0].m_Position.x - m_Points[0].m_InitPosition.x), (float)(m_Points[0].m_Position.y - m_Points[0].m_InitPosition.y)), m_Points[0].m_Size, LWGesture::PressAndDrag };
					} else m_Gesture = { m_Points[0].m_InitPosition, LWVector2f((float)(m_Points[0].m_Position.x - m_Points[0].m_InitPosition.x), (float)(m_Points[0].m_Position.y - m_Points[0].m_InitPosition.y)), m_Points[0].m_Size, LWGesture::Drag };
				}
			}
		}
	} else if(m_PointCount>=2){
		uint64_t TimeBothDown = m_Points[0].m_DownTime > m_Points[1].m_DownTime ? (m_Points[0].m_DownTime - m_Points[1].m_DownTime) : (m_Points[1].m_DownTime - m_Points[0].m_DownTime);
		if (TimeBothDown < FlickTime) {
			LWVector2i Dir = m_Points[1].m_InitPosition - m_Points[0].m_InitPosition;
			LWVector2i CDir = m_Points[1].m_Position - m_Points[0].m_Position;
			LWVector2i Src = m_Points[0].m_InitPosition + Dir / 2;
			float IDis = LWVector2f((float)Dir.x, (float)Dir.y).Length();
			float CDis = LWVector2f((float)CDir.x, (float)CDir.y).Length();
			m_Gesture = { Src, LWVector2f(), IDis - CDis, LWGesture::Pinch };
		}
	} else m_Gesture = { LWVector2i(), LWVector2f(), 1.0f, LWGesture::None };
	for (uint32_t i = 0; i < m_PointCount; i++) {
		m_Points[i].m_PrevPosition = m_Points[i].m_Position;
		if (m_Points[i].m_State == LWTouchPoint::UP) {
			memcpy(m_Points + i, m_Points + i + 1, sizeof(LWTouchPoint)*(m_PointCount - i - 1));
			m_PointCount--;
			i--;
		}
	}
	return *this;
}

const LWTouchPoint &LWTouch::GetPoint(uint32_t i) const {
	return m_Points[i];
}

const LWGesture &LWTouch::GetGesture(void) const {
	return m_Gesture;
}


uint32_t LWTouch::GetPointCount(void) const {
	return m_PointCount;
}

LWTouch::LWTouch() : LWInputDevice(), m_PointCount(0){
	m_Gesture.m_Type = LWGesture::None;
}
#pragma endregion

#pragma region LWGamePad


bool LWGamePad::ButtonDown(uint32_t Button) {
	return (m_Buttons&Button) != 0;
}

bool LWGamePad::ButtonUp(uint32_t Button) {
	return (m_Buttons&Button) == 0;
}

bool LWGamePad::ButtonPressed(uint32_t Button) {
	return (m_Buttons&Button) != 0 && (m_PrevButtons&Button) == 0;
}

bool LWGamePad::ButtonReleased(uint32_t Button) {
	return (m_Buttons&Button) == 0 && (m_PrevButtons&Button) != 0;
}

LWVector2f LWGamePad::GetLeftAxis(void) const {
	return m_LeftAxis;
}

LWVector2f LWGamePad::GetRightAxis(void) const {
	return m_RightAxis;
}

LWVector2f LWGamePad::GetBumperStrength(void) const {
	return m_BumperStrength;
}

uint32_t LWGamePad::GetDeviceIdx(void) const {
	return m_DeviceIdx;
}

LWGamePad::LWGamePad(uint32_t DeviceIdx) : LWInputDevice(), m_Buttons(0), m_PrevButtons(0), m_LeftAxis(LWVector2f()), m_RightAxis(LWVector2f()), m_BumperStrength(LWVector2f()), m_DeviceIdx(DeviceIdx) {}
#pragma endregion

#pragma region LWAccelerometer


LWAccelerometer &LWAccelerometer::SetBaseDirection(const LWVector3f &BaseDirection) {
	m_BaseDirection = BaseDirection;
	return *this;
}

LWAccelerometer &LWAccelerometer::Enable(void) {
	m_Flag |= RequestEnabled;
	return *this;
}

LWAccelerometer &LWAccelerometer::Disable(void) {
	m_Flag |= RequestDisabled;
	return *this;
}

LWVector3f LWAccelerometer::GetDirection(void) const {
	return m_Direction;
}

LWVector3f LWAccelerometer::GetBaseDirection(void) const {
	return m_BaseDirection;
}

LWVector3f LWAccelerometer::GetDirectionDiff(void) const {
	return m_BaseDirection - m_Direction;
}

bool LWAccelerometer::IsEnabled(void) const {
	return (m_Flag&Enabled) != 0;
}

LWAccelerometer::LWAccelerometer() : LWInputDevice(), m_Direction(LWVector3f(0.0f, 0.0f, 0.0f)), m_BaseDirection(LWVector3f(0.0f, 0.0f, 0.0f)), m_Flag(Enabled) {}

#pragma endregion

#pragma region LWGyroscope

LWGyroscope &LWGyroscope::ClearRotation(void) {
	m_Rotation = LWVector3f(0.0f);
	return *this;
}

LWGyroscope &LWGyroscope::Enable(void) {
	m_Flag |= RequestEnabled;
	return *this;
}

LWGyroscope &LWGyroscope::Disable(void) {
	m_Flag |= RequestDisabled;
	return *this;
}

float LWGyroscope::GetResolution(void) const {
	return m_Resolution;
}

float LWGyroscope::GetRange(void) const {
	return m_Range;
}

LWVector3f LWGyroscope::GetRawRotation(void) const {
	return m_Rotation;
}

LWVector3f LWGyroscope::GetRotation(uint32_t Orientation) const {
    if (Orientation == LWWindow::Rotation_90) return LWVector3f(-m_Rotation.y, m_Rotation.x, m_Rotation.z);
	else if (Orientation == LWWindow::Rotation_180) return LWVector3f(-m_Rotation.x, -m_Rotation.y, m_Rotation.z);
	else if (Orientation == LWWindow::Rotation_270) return LWVector3f(m_Rotation.y, -m_Rotation.x, m_Rotation.z);
	return m_Rotation; //Pitch, Yaw, Roll
}

LWVector3f LWGyroscope::GetNormalizedRotation(uint32_t Orientation) const {
	float iRange = 1.0f / m_Range;
	return GetRotation(Orientation)*iRange;
}

bool LWGyroscope::IsEnabled(void) const {
	return (m_Flag&Enabled) != 0;
}

LWGyroscope::LWGyroscope(float Resolution, float Range) : m_Rotation(LWVector3f(0.0f)), m_Flag(Enabled), m_Resolution(Resolution), m_Range(Range) {}
#pragma endregion
