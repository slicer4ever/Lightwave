#ifndef LWINPUTDEVICE_H
#define LWINPUTDEVICE_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWPlatform/LWTypes.h"

/*! \addtogroup LWPlatform
	@{
*/

/*! \brief an LWKey enum for mapping virtual key's to LWKey's. */
enum class LWKey : uint32_t {
	A, /*!< \brief A key. */
	B, /*!< \brief B key. */
	C, /*!< \brief C key. */
	D, /*!< \brief D key. */
	E, /*!< \brief E key. */
	F, /*!< \brief F key. */
	G, /*!< \brief G key. */
	H, /*!< \brief H key. */
	I, /*!< \brief I key. */
	J, /*!< \brief J key. */
	K, /*!< \brief K key. */
	L, /*!< \brief L key. */
	M, /*!< \brief M key. */
	N, /*!< \brief N key. */
	O, /*!< \brief O key. */
	P, /*!< \brief P key. */
	Q, /*!< \brief Q key. */
	R, /*!< \brief R key. */
	S, /*!< \brief S key. */
	T, /*!< \brief T key. */
	U, /*!< \brief U key. */
	V, /*!< \brief V key. */
	W, /*!< \brief W key. */
	X, /*!< \brief X key. */
	Y, /*!< \brief Y key. */
	Z, /*!< \brief Z key. */
	Key0, /*!< \brief 0 key. */
	Key1, /*!< \brief 1 key. */
	Key2, /*!< \brief 2 key. */
	Key3, /*!< \brief 3 key. */
	Key4, /*!< \brief 4 key. */
	Key5, /*!< \brief 5 key. */
	Key6, /*!< \brief 6 key. */
	Key7, /*!< \brief 7 key. */
	Key8, /*!< \brief 8 key. */
	Key9, /*!< \brief 9 key. */
	Tab, /*!< \brief Tab key. */
	CapLock, /*!< \brief caplocks key. */
	LShift, /*!< \brief left shift key. */
	LCtrl, /*!< \brief left ctrl key. */
	LMenu, /*!< \brief left menu key. */
	LAlt, /*!< \brief left alt key. */
	Space, /*!< \brief space key. */
	RAlt, /*!< \brief right alt key. */
	RMenu, /*!< \brief right menu key. */
	RCtrl, /*!< \brief right ctrl key. */
	RShift, /*!< \brief right shift key. */
	Return, /*!< \brief return key. */
	Back, /*!< \brief backspace key. */
	OEM_COMMA, /*!< \brief ,/< key. */
	OEM_MINUS, /*!< \brief -/_ key. */
	OEM_PLUS, /*!< \brief =/+ key. */
	OEM_PERIOD, /*!< \brief ./> key. */
	OEM_0, /*!< \brief `/~ Key on US Keyboards. */
	OEM_1, /*!< \brief //? key on US Keyboards. */
	OEM_2, /*!< \brief ;/: key on US Keyboards. */
	OEM_3, /*!< \brief '/" key on US Keyboards. */
	OEM_4, /*!< \brief [/{ key on US Keyboards. */
	OEM_5, /*!< \brief ]/} key on US Keyboards. */
	OEM_6, /*!< \brief \/| key on US Keyboards. */
	Left, /*!< \brief left arrow key. */
	Right, /*!< \brief right arrow key. */
	Up, /*!< \brief up arrow key. */
	Down, /*!< \brief down arrow key. */
	Insert, /*!< \brief Insert key. */
	Delete, /*!< \brief delete key. */
	Home, /*!< \brief home key. */
	End, /*!< \brief End key. */
	PageUp, /*!< \brief Page up key. */
	PageDown, /*!< \brief Page down key. */
	PrintScreen, /*!< \brief Print screen key. */
	ScrollLock, /*!< \brief scroll lock key. */
	Pause, /*!< \brief pause key. */
	NumLock, /*!< \brief Num lock key. */
	Num0, /*!< \brief Numpad 0 key. */
	Num1, /*!< \brief Numpad 1 key. */
	Num2, /*!< \brief Numpad 2 key. */
	Num3, /*!< \brief Numpad 3 key. */
	Num4, /*!< \brief Numpad 4 key. */
	Num5, /*!< \brief Numpad 5 key. */
	Num6, /*!< \brief Numpad 6 key. */
	Num7, /*!< \brief Numpad 7 key. */
	Num8, /*!< \brief Numpad 8 key. */
	Num9, /*!< \brief Numpad 9 key. */
	NumDecimal, /*!< \brief Numpad . key. */
	NumReturn, /*!< \brief Numpad return key. */
	NumAdd, /*!< \brief Numpad + key. */
	NumMinus, /*!< \brief Numpad minus key. */
	NumMultiply, /*!< \brief Numpad multiply key. */
	NumDivide, /*!< \brief Numpad divide key. */
	NumEqual, /*!< \brief Numpad equal key. */
	Esc, /*!< \brief Escape key. */
	F1, /*!< \brief F1 key. */
	F2, /*!< \brief F1 key. */
	F3, /*!< \brief F1 key. */
	F4, /*!< \brief F1 key. */
	F5, /*!< \brief F1 key. */
	F6, /*!< \brief F1 key. */
	F7, /*!< \brief F1 key. */
	F8, /*!< \brief F1 key. */
	F9, /*!< \brief F1 key. */
	F10, /*!< \brief F1 key. */
	F11, /*!< \brief F1 key. */
	F12, /*!< \brief F1 key. */
	F13, /*!< \brief F1 key. */
	F14, /*!< \brief F1 key. */
	F15, /*!< \brief F1 key. */
	F16, /*!< \brief F1 key. */
	F17, /*!< \brief F1 key. */
	F18, /*!< \brief F1 key. */
	F19, /*!< \brief F1 key. */
	MediaPlayPause, /*!< \brief extended keyboard media play key. */
	MediaNext, /*!< \brief extended keyboard media next key. */
	MediaPrev, /*!< \brief extended keyboard media prev key. */
	VolumeMute, /*!< \brief extended keyboard volume mute key. */
	VolumeDown, /*!< \brief extended keyboard volume down key. */
	VolumeUp, /*!< \brief extended keyboard volume up key. */
	Mail, /*!< \brief extended keyboard mail key. */
	Unknown /*!< \brief a vk key that we can't translate was pressed. */
};

/*! \brief an LWMouseKey enum for mapping system mouse buttons to LWMouse buttons. */
enum class LWMouseKey : uint32_t{
	Left = 0x1, /*!< \brief the left mouse button. */
	Right = 0x2, /*!< \brief the right mouse button. */
	Middle = 0x4, /*!< \brief the middle mouse button. */
	X1 = 0x8, /*!< \brief the X1 mouse button. */
	X2 = 0x10, /*!< \brief the X2 mouse button. */
};

/*! \brief a pure virtual class of the base input devices which can be attached to all the windows. */
class LWInputDevice{
public:

	/*! \brief returns this object casted as an mouse object. */
	LWMouse *AsMouse(void);

	/*! \brief returns this object casted as an keyboard object. */
	LWKeyboard *AsKeyboard(void);

	/*!< \brief returns this object casted as an touch object. */
	LWTouch *AsTouch(void);

	/*!< \brief returns this object casted as an gamepad object. */
	LWGamePad *AsGamepad(void);

	/*!< \brief returns this object casted as an gyroscope object. */
	LWGyroscope *AsGyroscope(void);

	/*!< \brief returns this object casted as an accelerometer object. */
	LWAccelerometer *AsAccelerometer(void);

	/*! \brief set's the next input device in the chain. 
		\return this input device.
	*/
	LWInputDevice &SetNext(LWInputDevice *Next);

	/*! \brief processes per system messages. 
		\returns rather the message was parsed or not.
	*/
	virtual bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window) = 0;

	/*! \brief updates the internal input device. */
	virtual LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime) = 0;

	/*! \brief returns the next input device in a single-link list chain. */
	LWInputDevice *GetNext(void) const;

	/*! \brief default constructor that zero's this object. */
	LWInputDevice();

protected:

	LWInputDevice *m_Next; /*!< \brief the next input device in the link list. */
};

/*! \brief the LWMouse class that represents an on screen mouse. */
class LWMouse : public LWInputDevice{
public:
	enum {
		DoubleClickTime = 250 //Time in MilliSeconds.
	};
	/*! \brief processes local system messages. 
		\return rather the message was parsed or not.
	*/
	virtual bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	/*! \brief update's the mouse object, this is called internally by the attached window object. */
	virtual LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	/*! \brief returns rather the button is down for a particular key. */
	bool ButtonDown(LWMouseKey Key) const;

	/*! \brief returns rather the button is up for a particular key. */
	bool ButtonUp(LWMouseKey Key) const;

	/*! \brief returns rather the button has just been released for a particular key. */
	bool ButtonReleased(LWMouseKey Key) const;
	
	/*! \brief returns rather the button has just been pressed for a particular key. */
	bool ButtonPressed(LWMouseKey Key) const;

	/*!< \breif returns if the current mouse button in a pressed state has performed a double click(this function may return false positives if alot of mouse buttons are being pressed rapidly). */
	bool DoubleClicked(void) const;

	/*! \brief returns the current position of the mouse, relative to the parent window. */
	LWVector2i GetPosition(void) const;

	/*!< \brief returns the positions of the mouse as a vector2f, relative to the parent window. */
	LWVector2f GetPositionf(void) const;

	/*! \brief returns the current scroll factor of the mouse object. */
	int32_t GetScroll(void) const;

	/*! \brief constructor for a LWMouse object. */
	LWMouse();
private:
	LWVector2i m_Position;
	uint64_t m_LastPressTime;
	bool m_DoubleClick;
	int32_t m_Scroll;
	uint32_t m_CurrState;
	uint32_t m_PrevState;
};

/*! \brief The default keyboard class which captures and manages the keyboard state.  this keyboard object is only designed to capture the ascii range of characters. */
class LWKeyboard : public LWInputDevice{
public:
	enum {
		MaxKeyChanges = 32,
		MaxKeys = 256
	};
	/*! \brief processes local system messages.
		\return rather the message was parsed or not.
	*/
	virtual bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	/*! \brief update's the keyboard object, this is called internally by the attached window. */
	virtual LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	/*! \brief return's rather that key is down or not, using an LWKey enum value.*/
	bool ButtonDown(LWKey Key) const;

	/*! \overload bool ButtonDown(uint32_t) */
	bool ButtonDown(uint32_t Key) const;

	/*! \brief return's rather that key is up or not, using an LWKey enum value. */
	bool ButtonUp(LWKey Key) const;

	/*! \overload ButtonUp(uint32_t) */
	bool ButtonUp(uint32_t Key) const;

	/*! \brief return's rather that key has just been pressed or not, using an LWKey enum value. */
	bool ButtonPressed(LWKey Key) const;

	/*! \overload ButtonPressed(uint32_t) */
	bool ButtonPressed(uint32_t Key) const;

	/*! \brief return's rather that key has been released or not, using an LWKey enum value. */
	bool ButtonReleased(LWKey Key) const;

	/*! \overload ButtonReleased(uint32_t) */
	bool ButtonReleased(uint32_t Key) const;

	/*! \brief returns the key that changed during the last frame in the key change list.
	\param i the index of the changed key list to request.
	*/
	uint32_t GetKeyChanged(uint32_t i) const;

	/*!< \brief returns the new state of the key for the changed at index(true = down, false = up), ButtonDown is unreliable in that an up/down state change may occur in a single step.
	*/
	bool GetKeyState(uint32_t i) const;

	/*!< \brief returns the utf-32 char that was pressed during the last frame.
		 \param i the index of the char in the list.
	*/
	uint32_t GetChar(uint32_t i) const;

	/*! \brief returns the number of key's changed last frame. */
	uint32_t GetKeyChangeCount(void) const;

	/*!< \brief returns the number of utf-32 characters pressed in the last frame. */
	uint32_t GetCharPressed(void) const;

	/*! \brief default constructor for the keyboard object, and zero's all keys. */
	LWKeyboard();
private:
	uint32_t m_KeyChanges[MaxKeyChanges];
	bool m_KeyStates[MaxKeyChanges];
	uint32_t m_CharInputs[MaxKeyChanges];
	uint32_t m_CurrState[(MaxKeys/32)];
	uint32_t m_PrevState[(MaxKeys/32)];
	uint32_t m_CharPressed;
	uint32_t m_KeyChangeCount;
};

/*!< a point object associated with touch press. */
struct LWTouchPoint {
	enum {
		DOWN,
		MOVED,
		UP
	};
	LWVector2i m_Position; /*!< \brief the last reported position of the touched point. */
	LWVector2i m_InitPosition; /*!< \brief the initial position of the touch point on down. */
	LWVector2i m_PrevPosition; /*!< \brief the position previously reported for this touch point. */
	uint64_t m_DownTime; /*!< \brief the timestamp when this touch point was created. */
	uint32_t m_State; /*!< \brief the current state the touch point is in. */
	float m_Size; /*!< \brief the size of the touch point. */
};

/*!< \brief touch gestures, which an LWTouch object will track, and with the variables enclosed derive the information necessary to respond to the gesture. */

struct LWGesture {
	enum {
		None, /*!< \brief gesture has no state at the moment. */
		Drag, /*!< \brief when the fingertip is placed over a surface, and does not lose contact. */
		Flick, /*!< \brief a sudden directional flick occurred, which tracks from source toward the direction of the flick, direction is a magnitude, scale is unused. */
		Pinch, /*!< \brief a pinch, where source is the center of the pinch, and direction is the way the fingers are moving away from, scale is a magnitude of that distance traveled. */
		Tap, /*!< \brief when a finger is quickly presses on a point, the source is this point, direction is unused. */
		Press, /*!< \brief when a finger is held on a point for a certain amount of time. */
		PressAndDrag, /*!< \brief when a finger is held on a point for certain amount of time, before dragging the finger. */
	};
	LWVector2i m_Source;
	LWVector2f m_Direction;
	float m_Scale;
	uint32_t m_Type;
};

class LWTouch : public LWInputDevice{
public:
	enum {
		MaxTouchPoints = 8
	};

	/*! \brief processes local system messages.
		 \return rather the message was parsed or not.
	*/
	virtual bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	/*!< \brief updates the touch object. */
	virtual LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	/*!< \brief returns the specified point. */
	const LWTouchPoint *GetPoint(uint32_t i) const;

	/*!< \brief returns the gesture. */
	const LWGesture &GetGesture() const;

	/*!< \brief return the number of points currently active. */
	uint32_t GetPointCount(void) const;

	/*!< \brief constructs the touch object. */
	LWTouch();
private:
	LWTouchPoint m_Points[MaxTouchPoints];
	LWGesture m_Gesture;
	uint32_t m_PointCount;
};


/*!< \brief Gamepad device, which is designed after the xbox controller scheme. */
class LWGamePad : public LWInputDevice {
public:
	enum {
		A = 0x1, /*!< \brief A button on xbox controllers(or X on playstation). */
		B = 0x2, /*!< \brief B button on xbox controller(or O on playstation). */
		X = 0x4, /*!< \brief X button on xbox controller(or [] on playstation). */
		Y = 0x8, /*!< \brief Y button on xbox controller(or /\ on playstation). */
		Left = 0x10, /*!< \brief left dpad button. */
		Right = 0x20, /*!< \brief right dpad button. */
		Up = 0x40, /*!< \brief up dpad button. */
		Down = 0x80, /*!< \brief down dpad button. */
		Start = 0x100, /*!< \brief start button. */
		Select = 0x200, /*!< \brief select button. */
		L1 = 0x400, /*!< \brief L1 button. */
		L2 = 0x800, /*!< \brief L2 button. */
		L3 = 0x1000, /*!< \brief L3 button. */
		R1 = 0x2000, /*!< \brief R1 button. */
		R2 = 0x4000, /*!< \brief R2 button. */
		R3 = 0x8000, /*!< \brief R3 button. */
		X1 = 0x10000, /*!< \brief optional additional unknown button. */
		x2 = 0x20000, /*!< \brief optional additional unknown button. */
		X3 = 0x40000, /*!< \brief optional additional unknown button. */
		X4 = 0x80000 /*!< \brief optional additional unknown button, if anymore unknown buttons  are encountered, they are ignored. */
	};

	/*! \brief processes local system messages.
		 \return rather the message was parsed or not.
	*/
	virtual bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	/*!< \brief updates the gamepad object. */
	virtual LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	/*!< \brief returns true if the button is currently down. */
	bool ButtonDown(uint32_t Button);

	/*<! \brief reutrns true if the button is currently up. */
	bool ButtonUp(uint32_t Button);

	/*!< \brief returns true if the button has just been pressed down. */
	bool ButtonPressed(uint32_t Button);

	/*!< \brief returns true if the button has just been released. */
	bool ButtonReleased(uint32_t Button);

	/*!< \brief returns the left joystick's normalized direction. */
	LWVector2f GetLeftAxis(void) const;

	/*!< \brief returns the right joystick's normalized direction.*/
	LWVector2f GetRightAxis(void) const;

	/*!< \brief returns the normalized pressure applied to the bumpers(x = left, y= right). */
	LWVector2f GetBumperStrength(void) const;

	/*!< \brief returns the gamepad's index. */
	uint32_t GetDeviceIdx(void) const;

	/*!< \brief gamepad constructor. */
	LWGamePad(uint32_t DeviceIdx);
private:
	uint32_t m_Buttons;
	uint32_t m_PrevButtons;
	LWVector2f m_LeftAxis;
	LWVector2f m_RightAxis;
	LWVector2f m_BumperStrength;
	uint32_t m_DeviceIdx;
};

/*!< \brief Accelerometer device. */
class LWAccelerometer : public LWInputDevice{
public:
	enum {
		Enabled = 0x1,
		RequestEnabled = 0x2,
		RequestDisabled = 0x4
	};
	bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	LWAccelerometer &SetBaseDirection(const LWVector3f &BaseDirection);

	LWAccelerometer &Enable(void);

	LWAccelerometer &Disable(void);

	LWVector3f GetDirection(void) const;

	LWVector3f GetBaseDirection(void) const;

	LWVector3f GetDirectionDiff(void) const;

	bool IsEnabled(void) const;

	LWAccelerometer();
private:
	LWVector3f m_Direction;
	LWVector3f m_BaseDirection;
	uint32_t m_Flag;

};

/*!< \brief Gyroscope device. */
class LWGyroscope : public LWInputDevice {
public:
	enum {
		Enabled = 0x1,
		RequestEnabled = 0x2,
		RequestDisabled = 0x4
	};
	bool ProcessSystemMessage(uint32_t MessageID, void *MessageData, uint64_t lCurrentTime, LWWindow *Window);

	LWInputDevice &Update(LWWindow *Window, uint64_t lCurrentTime);

	LWGyroscope &ClearRotation(void);

	LWGyroscope &Disable(void);

	LWGyroscope &Enable(void);

	float GetRange(void) const;

	float GetResolution(void) const;

	LWVector3f GetRawRotation(void) const;
	
	LWVector3f GetRotation(uint32_t Orientation) const;

	LWVector3f GetNormalizedRotation(uint32_t Orientation) const;

	bool IsEnabled(void) const;

	LWGyroscope(float Resolution, float Range);
private:
	LWVector3f m_Rotation;
	float m_Resolution;
	float m_Range;
	uint32_t m_Flag;

};

/*! @} */

#endif