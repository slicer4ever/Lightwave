#include "LWPlatform/LWVideoMode.h"
#include "LWPlatform/LWPlatform.h"
#include <cstring>
#include <algorithm>

uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize){
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	int32_t DPI = GetDpiForSystem();
	for (uint32_t i = 0; EnumDisplaySettings(nullptr, i, &dm) && ModeCount < MaxModes; i++) {
		Modes[ModeCount++] = LWVideoMode(LWVector2i(dm.dmPelsWidth, dm.dmPelsHeight), LWVector2i(DPI, DPI), dm.dmDisplayFrequency, ((dm.dmDisplayFlags&DM_INTERLACED) ? Interlaced : 0) | (dm.dmBitsPerPel == 4 ? Colored4Bit : (dm.dmBitsPerPel == 8 ? Colored8Bit : (dm.dmBitsPerPel == 16 ? Colored16Bit : (dm.dmBitsPerPel == 32 ? Colored32Bit : 0)))));
		for(uint32_t d=0;d<ModeCount-1;d++){
			if(Modes[d]==Modes[ModeCount-1]){
				ModeCount--;
				break;
			}
		}
	}
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	return ModeCount;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements){
	const uint32_t MaxModes = 256;

	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	int32_t DPI = GetDpiForSystem();
	for (uint32_t i = 0; EnumDisplaySettings(nullptr, i, &dm) && ModeCount < MaxModes; i++) {
		Modes[ModeCount] = LWVideoMode(LWVector2i(dm.dmPelsWidth, dm.dmPelsHeight), LWVector2i(DPI, DPI), dm.dmDisplayFrequency, ((dm.dmDisplayFlags&DM_INTERLACED) ? Interlaced : 0) | (dm.dmBitsPerPel == 4 ? Colored4Bit : (dm.dmBitsPerPel == 8 ? Colored8Bit : (dm.dmBitsPerPel == 16 ? Colored16Bit : (dm.dmBitsPerPel == 32 ? Colored32Bit : 0)))));
		if (Modes[ModeCount].GetSize().x < SizeRequirements.x) continue;
		if (Modes[ModeCount].GetSize().y < SizeRequirements.y) continue;
		if (SizeRequirements.z>0 && Modes[ModeCount].GetSize().x>SizeRequirements.z) continue;
		if (SizeRequirements.w>0 && Modes[ModeCount].GetSize().y > SizeRequirements.w) continue;
		if (Modes[ModeCount].GetFrequency() < (uint32_t)FrequencyRequirements.x) continue;
		if (FrequencyRequirements.y >0 && Modes[ModeCount].GetFrequency() > (uint32_t)FrequencyRequirements.y) continue;
		if (FlagRequirements != 0 && (Modes[ModeCount].GetFlag()&FlagRequirements) == 0) continue;
		ModeCount++;
		for (uint32_t d = 0; d < ModeCount-1; d++){
			if (Modes[d] == Modes[ModeCount - 1]){
				ModeCount--;
				break;
			}
		}

	}
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	return ModeCount;
}

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode){
	DEVMODE DM = { 0 };
	DM.dmSize = sizeof(DEVMODE);
	DM.dmPelsWidth = Mode.GetSize().x;
	DM.dmPelsHeight = Mode.GetSize().y;
	DM.dmBitsPerPel = 32; //Automatically select a 32 bit mode. */
	DM.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	return ChangeDisplaySettings(&DM, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
}

LWVideoMode LWVideoMode::GetActiveMode(void){
	DEVMODE dm = { 0 };
	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
	int32_t DPI = GetDpiForSystem();
	return LWVideoMode(LWVector2i(dm.dmPelsWidth, dm.dmPelsHeight), LWVector2i(DPI, DPI), dm.dmDisplayFrequency, ((dm.dmDisplayFlags&DM_INTERLACED)?Interlaced:0)|(dm.dmBitsPerPel==4?Colored4Bit:(dm.dmBitsPerPel==8?Colored8Bit:(dm.dmBitsPerPel==16?Colored16Bit:(dm.dmBitsPerPel==32?Colored32Bit:0)))));
}