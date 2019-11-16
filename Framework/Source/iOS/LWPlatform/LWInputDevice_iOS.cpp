#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWTimer.h"
#include <iostream>

#pragma region LWMouse
bool LWMouse::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window){
	return false;
}

#pragma endregion

#pragma region LWKeyboard

bool LWKeyboard::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *){
    LWIOSEvent *E = (LWIOSEvent*)MessageData;
    if(MessageID==LWIOSEventCode::CharEvent){
        if(m_CharPressed>=MaxKeyChanges) return true;
        m_CharInputs[m_CharPressed++] = E->m_TouchPoints[0].x;
        return true;
    }else if(MessageID==LWIOSEventCode::KeyEvent){
        if(m_KeyChangeCount>=MaxKeyChanges) return true;
        m_KeyChanges[m_KeyChangeCount] = E->m_TouchPoints[0].x;
        m_KeyStates[m_KeyChangeCount] = true;
        m_KeyChangeCount++;
        return true;
    }
	return false;

}
#pragma endregion

#pragma region LWTouch

bool LWTouch::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
    LWVector2i WndSize = Window->GetSize();
    if(MessageID==TouchDown || MessageID==TouchMoved || MessageID==TouchEnded){
        LWIOSEvent *Event = (LWIOSEvent*)MessageData;
        float TouchScale = [[UIScreen mainScreen] scale];
        for(uint32_t i=0;i<Event->m_TouchCount;i++){
            m_Points[i].m_Position = Event->m_TouchPoints[i]*TouchScale;
            m_Points[i].m_Position.y = WndSize.y-m_Points[i].m_Position.y;
            m_Points[i].m_State = Event->m_TouchState[i]==UITouchPhaseBegan?LWTouchPoint::DOWN:(Event->m_TouchState[i]==UITouchPhaseEnded?LWTouchPoint::UP:LWTouchPoint::MOVED);
            if(m_Points[i].m_State==LWTouchPoint::DOWN){
                m_Points[i].m_DownTime = lCurrentTime;
                m_Points[i].m_InitPosition = m_Points[i].m_Position;
                m_Points[i].m_PrevPosition = m_Points[i].m_Position;
                m_Points[i].m_Size = Event->m_TouchSize[i];
            }
        }
        m_PointCount = Event->m_TouchCount;
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
	return false;
}

LWInputDevice &LWAccelerometer::Update(LWWindow *Window, uint64_t lCurrentTime) {
    const float EarthGravity = 9.80665f;
    LWGyroscope *Gyro = Window->GetGyroscopeDevice();
    if ((m_Flag&(RequestEnabled | Enabled)) == RequestEnabled) {
        if(!Gyro || !Gyro->IsEnabled()) [LWAppContext.m_MotionManager startDeviceMotionUpdates];
		m_Flag |= Enabled;
	} else if ((m_Flag&(RequestDisabled | Enabled)) == (RequestDisabled | Enabled)) {
        if(!Gyro || !Gyro->IsEnabled()) [LWAppContext.m_MotionManager stopDeviceMotionUpdates];
		m_Flag &= ~Enabled;
	}
	m_Flag &= ~(RequestEnabled | RequestDisabled);
	if (!(m_Flag&Enabled)) return *this;
    CMDeviceMotion *Motion = LWAppContext.m_MotionManager.deviceMotion;
    if(Motion==nil) return *this;
    //To stay in line with android's implementation with combine gravity and userAccel properys.
    CMAcceleration User = Motion.userAcceleration;
    CMAcceleration Gravity = Motion.gravity;
    m_Direction = LWVector3f(User.x+Gravity.x, User.y+Gravity.y, User.z+Gravity.z);
    //std::cout << "accel: " << m_Direction.x << " " << m_Direction.y << " " << m_Direction.z << std::endl;
	return *this;
}
#pragma endregion

#pragma region LWGyroscope

bool LWGyroscope::ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) {
	return false;
}

LWInputDevice &LWGyroscope::Update(LWWindow *Window, uint64_t lCurrentTime) {
    LWAccelerometer *Accel = Window->GetAccelerometerDevice();
	if ((m_Flag&(RequestEnabled | Enabled)) == RequestEnabled) {
        if(!Accel || !Accel->IsEnabled()) [LWAppContext.m_MotionManager startDeviceMotionUpdates];
        m_Flag |= Enabled;
	} else if ((m_Flag&(RequestDisabled | Enabled)) == (RequestDisabled | Enabled)) {
        if(!Accel || !Accel->IsEnabled()) [LWAppContext.m_MotionManager stopDeviceMotionUpdates];
		m_Flag &= ~Enabled;
	}
	m_Flag &= ~(RequestEnabled | RequestDisabled);
    if (!(m_Flag&Enabled)) return *this;
    CMDeviceMotion *Motion = LWAppContext.m_MotionManager.deviceMotion;
    if(Motion==nil) return *this;
    m_Rotation.x += Motion.rotationRate.x;
    m_Rotation.y += Motion.rotationRate.y;
    m_Rotation.z += Motion.rotationRate.z;
	//std::cout << "Gyro: " << m_Rotation.x << " " << m_Rotation.y  << " " << m_Rotation.z << std::endl;
	return *this;
}

#pragma endregion