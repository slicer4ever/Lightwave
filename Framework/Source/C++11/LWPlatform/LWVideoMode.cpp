#include "LWPlatform/LWVideoMode.h"

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