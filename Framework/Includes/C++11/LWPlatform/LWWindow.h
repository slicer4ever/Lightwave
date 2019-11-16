#ifndef LWWINDOW_H
#define LWWINDOW_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWText.h"
#include "LWVideo/LWTypes.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWTypes.h"
#include "LWPlatform/LWInputDevice.h"
#include <cstdint>

#define MAXGAMEPADS 4

/*! \addtogroup LWPlatform
	@{
*/
/*!
	\brief LWWindow is a class for handling window operations, input management, upon creation the window is created immediately.
	\note BorderlessFullscreen and Fullscreen are considered the same under X11, and will resize the game to the currently active monitor dimension if used.  for android and iOS, the mode means nothing, and a fullscreen window is created regardless.  also many of the set functions do nothing for the mobile platforms.
*/
class LWWindow{
public:
	static const uint32_t Terminate    = 0x1; /*!< \brief the window has requested for a termination */
	static const uint32_t Visible      = 0x2; /*!< \brief the window is visible. */
	static const uint32_t Focused      = 0x4; /*!< \brief the window is focused or not. */
	static const uint32_t MouseVisible = 0x8; /*!< \brief rather or not the mouse is to be considered visible or not. */
	static const uint32_t PosChanged   = 0x10; /*!< \brief the window position has been changed. */
	static const uint32_t SizeChanged  = 0x20; /*!< \brief the window size has been changed. */
	static const uint32_t FocusChanged = 0x40; /*!< \brief the window focus has been changed. */
	static const uint32_t Error        = 0x80; /*!< \brief the window had an error during creation. */
	static const uint32_t Fullscreen   = 0x100; /*!< \brief the window is designed to be in fullscreen mode. note that for DX11 the display mode does not need to be changed if this flag is set. */
	static const uint32_t Borderless   = 0x200; /*!< \brief the window is designed to be a borderless window. */
	static const uint32_t OrientationChanged = 0x400; /*!< \brief signals that the window orientation has changed. */
	static const uint32_t KeyboardPresent = 0x800; /*!< \brief the windows softward keyboard is present, this flag is only viable for iOS and Android OS's. */
	static const uint32_t Rotation_0 = 0; /*!< \brief the current window is in default portrait mode, and is not rotated at all. */
	static const uint32_t Rotation_90 = 0x1000; /*!< \brief the current window is orientated in landscape mode. */
	static const uint32_t Rotation_180 = 0x2000; /*!< \brief the current window is orientated in upside down portrait mode. */
	static const uint32_t Rotation_270 = 0x4000; /*!< \brief the current window is orientated in upside down landscape mode. */

	static const uint32_t MouseDevice         = 0x8000; /*!< \brief flag to indicate window should create a mouse device if applicable to the platform(windows, mac, linux). */
	static const uint32_t KeyboardDevice      = 0x10000; /*!< \brief flag to indicate window should create a keyboard device if applicable to the platform. (windows, mac, linux, android, iOS) */
	static const uint32_t GamepadDevice       = 0x20000; /*!< \brief flag to indicate window should create the gamepad devices if applicable to the platform. (windows). */
	static const uint32_t TouchDevice         = 0x40000; /*!< \brief flag to indicate window should create the touchscreen device if applicable to the platform. (android, iOS). */
	static const uint32_t GyroscopeDevice     = 0x80000; /*!< \brief flag to indicate window should create and activate the gyroscope device if applicable to the platform. (android, iOS). */
	static const uint32_t AccelerometerDevice = 0x100000; /*!< \brief flag to indicate window should create and activate the accelerometer device if applicable to the platform. (android, IOS). */

    enum{
        WindowedMode = (Visible | MouseVisible), /*!< \brief default flag to pass to the constructor to create a default window. */
		BorderlessMode = (Visible | MouseVisible | Borderless), /*!< \brief combined flag to pass to the constructor to create a borderless window. */
		FullscreenMode = (Visible | MouseVisible | Borderless | Fullscreen), /*!< \brief combined flag to pass to the constructor to create a borderless window designed to run in fullscreen mode. */
        Keyboard_System = 0 /*!< \brief flag used to specify the system virtual keyboard is to be the default one opened. */
    };
	static const uint32_t DialogOK = 0x1; /*!< \brief provides the dialog OK button when creating a dialog. */
	static const uint32_t DialogYES = 0x2; /*!< \brief provides the dialog an yes and no button when creating a dialog. */
	static const uint32_t DialogNo = 0x4; /*!< \brief return value only on what MakeDialog creates. */
	static const uint32_t DialogCancel = 0x8; /*!< \brief provides the dialog an cancel button when creating a dialog. */

	/*! \brief constructs a system dialog for the user, use this only if an error occurs. */
	static uint32_t MakeDialog(const LWText &Text, const LWText &Header, uint32_t DialogFlags);

	/*!< \brief constructs a the os system's standard save dialog, currently only windows implementation is made at the moment.
		 \note Filter is a seperated list of terminated strings, the first string is the pattern to match(i.e: *.txt) and the second string is the user friendly name(i.e: Text Files), they are specified like so: "*.txt\0Text Files\0\0" an empty string indicates the end of the filter list. 
		 \return true if the dialog save button was pressed, results are written into Buffer.
	*/

	static bool MakeSaveFileDialog(const LWText &Filter, char *Buffer, uint32_t BufferLen);

	/*!< \brief constructs a the os system's standard load dialog, currently only windows implementation is made at the moment.
		 \note Filter is a seperated list of terminated strings, the first string is the pattern to match(i.e: *.txt) and the second string is the user friendly name(i.e: Text Files), they are specified like so: "*.txt\0Text Files\0\0" an empty string indicates the end of the filter list.
		 \return true if the dialog save button was pressed, results are written into Buffer.
	*/
	static bool MakeLoadFileDialog(const LWText &Filter, char *Buffer, uint32_t BufferLen);

	/*!< \brief writes to the internal clipboard buffer the text specified. 
		 \return true on success, false on failure.
	*/
	static bool WriteClipboardText(const LWText &Text);

	/*!< \brief reads into buffer the internal clipboard buffer. 
		 \return the number of bytes written into buffer.
	*/
	static uint32_t ReadClipboardText(char *Buffer, uint32_t BufferLen);

	/*! \brief Set's the window title. */
	LWWindow &SetTitle(const LWText &Title);

	/*! \brief set's a formatted string window title. */
	LWWindow &SetTitlef(const char *Fmt, ...);

	/*! \brief Set's the position of the window. */
	LWWindow &SetPosition(const LWVector2i &Position);

	/*! \brief Set's the size of the window. */
	LWWindow &SetSize(const LWVector2i &Size);

	/*! \brief Set's the window as being visible or invisible.
		\note this flag is ignored in linux at the moment.
	*/
	LWWindow &SetVisible(bool isVisible);

	/*! \brief Set's the window as having focus or not having focus. 
		\note this flag is ignored in linux.
	*/
	LWWindow &SetFocused(bool isFocused);

	/*!< \brief Set's the window to either have borderless style decorations, or normal window decorations.  isFullscreen should be set to true if the intent is to make the window fullscreen(this mitigates the window to be above the desktop's ui). (note: this does not set the actual fullscreen flag for the window, that is only useful in the creation of the window.) */
	LWWindow &SetBorderless(bool isBorderless, bool isFullscreen);

	/*! \brief Set's the cursor's position relative to the window, this method is not found in the LWMouse object because the input devices are generally read-only mechanisms, and are not suppose to actually modify the hardware they are reading from. */
	LWWindow &SetMousePosition(const LWVector2i &Position);

	/*! \brief Set's rather the mouse should be visible or not while inside the window area. */
	LWWindow &SetMouseVisible(bool isVisible);

	/*! \brief attaches the input device to the window.
		\return the input device.
		\note ownership of the input device is handed to the window when doing this. once an input device is attached, it can not be unattached.
	*/
	LWInputDevice *AttachInputDevice(LWInputDevice *Device);

    /*!< \brief opens the software keyboard if the platform is applicable for it.(i.e: iOS, Android), also supports a number of common keyboard layouts. */
    LWWindow &OpenKeyboard(uint32_t KeyboardType = Keyboard_System);
    
    /*!< \brief set's the internal editing text position and selected elements, this is necessary for any editing text in order to keep the keyboard autocorrect insync with the actual text box object. an editSize of 0 means no selection ring.*/
    LWWindow &SetKeyboardEditRange(uint32_t CursorPosition, uint32_t EditSize = 0);

	/*!< \brief set's the software keyboard internal text if necessary. */
	LWWindow &SetKeyboardText(const char *Text);

	/*!< \brief set's the active gamepad device(used for ui selection.) .*/
	LWWindow &SetActiveGamepad(LWGamePad *Gamepad);
    
    /*!< \brief closes the software keyboard if applicable. */
    LWWindow &CloseKeyboard(void);

    /*!< \brief returns the current keyboards editing range. */
    LWWindow &GetKeyboardEditRange(uint32_t &CursorPosition, uint32_t &EditSize);
    
    /*!< \brief retrieves the text entered into the virtual keyboard. if applicable
		 \param Buffer the buffer to receive the edit data.
		 \param BufferLen the size of the buffer.
         \return the number of characters written into buffer.
         \note Buffer may be null.
    */
    uint32_t GetKeyboardText(char *Buffer, uint32_t BufferLen);

	/*!< \brief returns the keyboards dimensions, position in xy, and size in wz. */
	LWVector4f GetKeyboardLayout(void);

	/*!< \brief returns the keyboards current type. */
	uint32_t GetKeyboardType(void);
    
	/*! \brief internally processes system and user messages, and updates/notifies the application as necessary. 
		\return true on success of processing the window message, false on failure.
	*/
	bool ProcessWindowMessage(uint32_t Message, void *MessageData, uint64_t lCurrentTime);

	/*! \brief updates the window and handles window messages. */
	LWWindow &Update(uint64_t lCurrentTime);

	/*! \brief returns the current title of the window. */
	const LWText &GetTitle(void) const;

	/*! \brief returns the name of the window. */
	const LWText &GetName(void) const;

	/*! \brief returns the position of the window. */
	LWVector2i GetPosition(void) const;

	/*!< \brief returns the position as a vector2f of the window. */
	LWVector2f GetPositionf(void) const;

	/*! \brief returns the size of the window. */
	LWVector2i GetSize(void) const;

	/*!< \brief returns the size as a vector2f of the window. */
	LWVector2f GetSizef(void) const;

	/*!< \brief returns the current aspect ratio of the window(width/height). */
	float GetAspect(void) const;

	/*! \brief returns the current flag of the window. */
	uint32_t GetFlag(void) const;

	/*!< \brief returns the part of the flag dealing with the orientation component. */
	uint32_t GetOrientation(void) const;

	/*! \brief returns the context of the underlying window information. */
	LWWindowContext &GetContext(void);

	/*!< \brief returns the input devices associated with the window. */
	LWInputDevice *GetInputDevices(void);

	/*!< \brief returns the mouse device that was associated with the window. */
	LWMouse *GetMouseDevice(void);

	/*!< \brief returns the keyboard device that was associated with the window. */
	LWKeyboard *GetKeyboardDevice(void);

	/*!< \brief returns the touch device that was associated with the window. */
	LWTouch *GetTouchDevice(void);

	/*!< \brief returns the nth(0-4) Gamepad device that was associated with the window. */
	LWGamePad *GetGamepadDevice(uint32_t i);

	/*!< \brief returns the accelerometer device that was associated with the window. */
	LWAccelerometer *GetAccelerometerDevice(void);

	/*!< \brief returns the gyroscope device that was associated with the window. */
	LWGyroscope *GetGyroscopeDevice(void);

	/*!< \brief returns the active gamepad device that was associated with the window(default gamepad0). */
	LWGamePad *GetActiveGamepadDevice(void);

	/*! \brief returns the allocator used for the window. */
	LWAllocator *GetAllocator(void) const;

	/*!< \brief returns true if the window is currently focused or not. */
	bool isFocused(void) const;

	/*!< \brief returns true if the size has changed in the last update call. */
	bool SizeUpdated(void) const;

	/*!< \brief returns true if the position has changed in the last update call. */
	bool PositionUpdated(void) const;

	/*!< \brief returns true if the focus has changed in the last update call. */
	bool FocusUpdated(void) const;

	/*!< \brief returns true if the window is set to be visible or not. */
	bool isVisible(void) const;

	/*! \brief Constructor for the window, if an error occurs, the error flag is set for the object.
		\param Title the window's title.
		\param Name the window's name.
		\param Position the position of the window.
		\param Size the size of the window.
		\param Allocator the allocator object for storing a title and name with.
		\param Flag the default flag to set the window to(including the requested input's for the window.).
		\note Allocator is stored for future allocations in the class.
		*/
	LWWindow(const LWText &Title, const LWText &Name, LWAllocator &Allocator, uint32_t Flag = WindowedMode|MouseDevice|KeyboardDevice, const LWVector2i &Position = LWVector2i(100, 100), const LWVector2i &Size = LWVector2i(640, 480));

	/*! \brief destructor for the window. */
	~LWWindow();

private:
	LWText m_Title;
	LWText m_Name;
	LWWindowContext m_Context;
	LWAllocator *m_Allocator;
	LWInputDevice *m_FirstDevice;
	LWMouse *m_MouseDevice;
	LWKeyboard *m_KeyboardDevice;
	LWTouch *m_TouchDevice;
	LWGyroscope *m_GyroscopeDevice;
	LWGamePad *m_ActiveGamepad;
	LWGamePad *m_GamepadDevice[MAXGAMEPADS];
	LWAccelerometer *m_AccelerometerDevice;

	LWVector2i m_Position;
	LWVector2i m_Size;
	uint32_t m_Flag;
};
/*! @} */

#endif