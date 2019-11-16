#ifndef LWVIDEOMODE_H
#define LWVIDEOMODE_H
#include "LWCore/LWVector.h"
/*! \addtogroup LWPlatform
	@{
*/

/*! \brief LWVideoMode contains methods for obtaining the supported video resolutions of the current display. 
	\note for X11 all modes are assumed to be non-interlaced 32 bit mode, and may not report the actual correct color modes at this time.
*/
class LWVideoMode{
public:
	static const uint32_t Interlaced = 0x1; /*!< \brief flag to represent that the video mode is interlaced. */
	static const uint32_t Colored4Bit = 0x2; /*!< \brief flag to represent that the display mode supports 4 bit Colored mode. */
	static const uint32_t Colored8Bit = 0x4; /*!< \brief flag to represent that the display mode supports 8 bit Colored mode. */
	static const uint32_t Colored16Bit = 0x10; /*!< \brief flag to represent that the display mode supports 16 bit Colored mode. */
	static const uint32_t Colored32Bit = 0x20; /*!< \brief flag to represent that the display mode supports 32 bit Colored mode, this is the most commonly expected mode to encounter. */
	static const uint32_t Rotation_0 = 0x0; /*!< \brief the flag that represents that the display is rotated 0 degrees, note that this is not an actual flag, but an ommision that no other rotation is set implies that this is the default rotation. */
	static const uint32_t Rotation_90 = 0x40; /*!< \brief flag to represent that the display is rotated 90 degrees. */
	static const uint32_t Rotation_180 = 0x80; /*!< \brief the flag to represent that the display is rotated 180 degrees. */
	static const uint32_t Rotation_270 = 0x100; /*!< \brief the flag to represent that the display is rotated 270 degrees. */
	/*! \brief writes into Buffer all of the supported display modes, and returns the number of modes.
		\param Buffer the buffer to receive the modes, this can be null, in which case nothing is written.
		\param BufferSize the number of LWVideoMode's buffer contains.
		\return the total number of display modes.
		\note passing null will simply return the number of display modes, which can be used to create the buffer to receive the actual display modes.
			  also note that a unique list is guaranteed to be generated, finally a maximum of 256 modes can be queried for at any given time due to how the internal system works.
	
	*/
	static uint32_t GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize);

	/*! \brief writes into Buffer all of the supported display modes which meet the specified requirements.
		\param Buffer the buffer to receive the modes, this can be null, in which case nothing is written.
		\param BufferSize the number of LWVideoMode's buffer contains.
		\param SizeRequirements a vector4 where the first two components represent the minimum size, and the last two components represent the maximum size, a component with 0 in it's field is disregarded for meeting the requirments.
		\param FrequencyRequirements a vector2 which specifies the minimum and maximum frequency you are looking for, leave either component to 0 to ignore that requirement.
		\param FlagRequirements a flag variable which must be anded with this requirement and return a non-zero result to be considered as meeting the requirement, set this to 0 if no flag requirement is required.
		\return the total number of display modes that meet these requirements.
		\note passing null to buffer will simply return the number of display modes which meet the requirements, which can be used to create the buffer to receive the actual display modes.
	*/
	static uint32_t GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements);

	/*! \brief changes the display mode to the specified screen mode.
		\note it is ABSOLUTELY RECOMMENDED that upon app completion, the user is to return the screen to the original size.
		\return true if the change was successful, false otherwise.
	*/
	static bool SetDisplayTo(const LWVideoMode &Mode);

	/*! \brief returns the current video mode the desktop is set to now. */
	static LWVideoMode GetActiveMode(void);

	/*! \brief returns rather two modes are equivalent. */
	bool operator == (const LWVideoMode &Rhs);

	/*! \brief returns the supported mode size. */
	LWVector2i GetSize(void) const;

	/*!< \brief returns the DPI of the screen horizontally and vertically. */
	LWVector2i GetDPI(void) const;

	/*! \brief returns the refresh rate of the video mode. */
	uint32_t GetFrequency(void) const;

	/*! \brief returns the flag for the video mode. */
	uint32_t GetFlag(void) const;

	/*! \brief returns the color mode for the display, or 0 if no known color mode is set. */
	uint32_t GetColorMode(void) const;

	/*! \brief returns the rotation of the display. */
	uint32_t GetRotation(void) const;

	/*! \brief constructs an LWVideoMode object. */
	LWVideoMode(const LWVector2i &Size, const LWVector2i &DPI, uint32_t Frequency, uint32_t Flag);

	/*! \brief constructs an empty object. */
	LWVideoMode();
private:
	LWVector2i m_Size;
	LWVector2i m_DPI;
	uint32_t m_Frequency;
	uint32_t m_Flag;
};

/*! @} */
#endif