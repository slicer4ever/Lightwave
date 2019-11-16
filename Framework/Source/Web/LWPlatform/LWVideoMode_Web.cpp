#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWVideoMode.h"
#include <cstring>
#include <algorithm>

uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize) {
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 1;
	LWVector2i Size = LWVector2i();
	Size.x = EM_ASM_INT(return window.innerWidth;);
	Size.y = EM_ASM_INT(return window.innerHeight;);
	Modes[0] = LWVideoMode(Size, LWVector2i(), 0, LWVideoMode::Colored32Bit);
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	return ModeCount;
}

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode) {
	return false;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements) {
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 1;
	LWVector2i Size = LWVector2i();
	Size.x = EM_ASM_INT(return window.innerWidth;);
	Size.y = EM_ASM_INT(return window.innerHeight;);
	Modes[0] = LWVideoMode(Size, LWVector2i(), 0, LWVideoMode::Colored32Bit);
	if (Size.x < SizeRequirements.x) return 0;
	if (Size.y < SizeRequirements.y) return 0;
	if (SizeRequirements.z > 0 && Size.x > SizeRequirements.z) return 0;
	if (SizeRequirements.w > 0 && Size.y > SizeRequirements.w) return 0;
	if (FlagRequirements != 0 && (Modes[0].GetFlag()&FlagRequirements) == 0) return 0;
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	return ModeCount;
}

LWVideoMode LWVideoMode::GetActiveMode(void) {
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 1;
	LWVector2i Size = LWVector2i();
	Size.x = EM_ASM_INT(return window.innerWidth;);
	Size.y = EM_ASM_INT(return window.innerHeight;);
	return LWVideoMode(Size, LWVector2i(0, 0), 0, LWVideoMode::Colored32Bit);
}