#include "LWPlatform/LWVideoMode.h"

const uint32_t LWVideoMode::Interlaced; /*!< \brief flag to represent that the video mode is interlaced. */
const uint32_t LWVideoMode::Colored4Bit; /*!< \brief flag to represent that the display mode supports 4 bit Colored mode. */
const uint32_t LWVideoMode::Colored8Bit; /*!< \brief flag to represent that the display mode supports 8 bit Colored mode. */
const uint32_t LWVideoMode::Colored16Bit; /*!< \brief flag to represent that the display mode supports 16 bit Colored mode. */
const uint32_t LWVideoMode::Colored32Bit; /*!< \brief flag to represent that the display mode supports 32 bit Colored mode, this is the most commonly expected mode to encounter. */
const uint32_t LWVideoMode::Rotation_0; /*!< \brief the flag that represents that the display is rotated 0 degrees, note that this is not an actual flag, but an ommision that no other rotation is set implies that this is the default rotation. */
const uint32_t LWVideoMode::Rotation_90; /*!< \brief flag to represent that the display is rotated 90 degrees. */
const uint32_t LWVideoMode::Rotation_180; /*!< \brief the flag to represent that the display is rotated 180 degrees. */
const uint32_t LWVideoMode::Rotation_270; /*!< \brief the flag to represent that the display is rotated 270 degrees. */

bool LWVideoMode::operator==(const LWVideoMode &Rhs){
	return (m_Size == Rhs.GetSize() && m_Frequency == Rhs.GetFrequency() && m_Flag == Rhs.GetFlag());
}

LWVector2i LWVideoMode::GetSize(void) const{
	return m_Size;
}

uint32_t LWVideoMode::GetFrequency(void) const{
	return m_Frequency;
}

uint32_t LWVideoMode::GetFlag(void) const{
	return m_Flag;
}

bool LWVideoMode::isInterlaced(void) const {
	return (m_Flag & Interlaced) != 0;
}

uint32_t LWVideoMode::GetColorMode(void) const{
	return (m_Flag&(Colored32Bit | Colored16Bit | Colored8Bit | Colored4Bit));
}

uint32_t LWVideoMode::GetRotation(void) const{
	return (m_Flag&(Rotation_90 | Rotation_180 | Rotation_270));
}

LWVector2i LWVideoMode::GetDPI(void) const {
	return m_DPI;
}

LWVideoMode::LWVideoMode(const LWVector2i &Size, const LWVector2i &DPI, uint32_t Frequency, uint32_t Flag) : m_Size(Size), m_DPI(DPI), m_Frequency(Frequency),m_Flag(Flag){}

LWVideoMode::LWVideoMode() : m_Size(LWVector2i()), m_Frequency(0), m_Flag(0){}