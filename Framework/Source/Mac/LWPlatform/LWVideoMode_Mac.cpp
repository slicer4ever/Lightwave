#include "LWPlatform/LWVideoMode.h"
#include "LWPlatform/LWPlatform.h"
#include <cstring>
#include <algorithm>


uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize){
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
    CFArrayRef ModeList = CGDisplayCopyAllDisplayModes(CGDirectDisplayID(), NULL);
    CFIndex Count = CFArrayGetCount(ModeList);
    for (CFIndex i=0;i<Count;i++){
        CGDisplayModeRef ModeR = (CGDisplayModeRef)CFArrayGetValueAtIndex(ModeList, i);
        Modes[ModeCount++] = LWVideoMode(LWVector2i((uint32_t)CGDisplayModeGetWidth(ModeR),  (uint32_t)CGDisplayModeGetHeight(ModeR)), LWVector2i(72,72), CGDisplayModeGetRefreshRate(ModeR), Colored32Bit);
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

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode){
    CGDisplayConfigRef Config;
    CFArrayRef ModeList = CGDisplayCopyAllDisplayModes(CGDirectDisplayID(), NULL);
    CFIndex Count = CFArrayGetCount(ModeList);
    for(CFIndex i = 0; i < Count;i++){
        CGDisplayModeRef ModeR = (CGDisplayModeRef)CFArrayGetValueAtIndex(ModeList, i);
        LWVideoMode cMode = LWVideoMode(LWVector2i((uint32_t)CGDisplayModeGetWidth(ModeR), (uint32_t)CGDisplayModeGetHeight(ModeR)), LWVector2i(72,72), CGDisplayModeGetRefreshRate(ModeR), Colored32Bit);
        if(cMode==Mode){
            
            if(CGBeginDisplayConfiguration(&Config)!=kCGErrorSuccess) return false;
            CGConfigureDisplayWithDisplayMode(Config, CGMainDisplayID(), ModeR, NULL);
            CGCompleteDisplayConfiguration(Config, kCGConfigureForAppOnly);
            return true;
        }
    }
    return false;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements){
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
    CFArrayRef ModeList = CGDisplayCopyAllDisplayModes(CGDirectDisplayID(), NULL);
    CFIndex Count = CFArrayGetCount(ModeList);
    for (CFIndex i=0;i<Count;i++){
        CGDisplayModeRef ModeR = (CGDisplayModeRef)CFArrayGetValueAtIndex(ModeList, i);
        Modes[ModeCount] = LWVideoMode(LWVector2i((uint32_t)CGDisplayModeGetWidth(ModeR), (uint32_t)CGDisplayModeGetHeight(ModeR)), LWVector2i(72,72), CGDisplayModeGetRefreshRate(ModeR), Colored32Bit);
        if (Modes[ModeCount].GetSize().x < SizeRequirements.x) continue;
		if (Modes[ModeCount].GetSize().y < SizeRequirements.y) continue;
		if (SizeRequirements.z>0 && Modes[ModeCount].GetSize().x>SizeRequirements.z) continue;
		if (SizeRequirements.w>0 && Modes[ModeCount].GetSize().y > SizeRequirements.w) continue;
		if (Modes[ModeCount].GetFrequency() < (uint32_t)FrequencyRequirements.x) continue;
		if (FrequencyRequirements.y >0 && Modes[ModeCount].GetFrequency() > (uint32_t)FrequencyRequirements.y) continue;
		if (FlagRequirements != 0 && (Modes[ModeCount].GetFlag()&FlagRequirements) == 0) continue;
		ModeCount++;
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

LWVideoMode LWVideoMode::GetActiveMode(void){
    
    CGDisplayModeRef Active = CGDisplayCopyDisplayMode(CGDirectDisplayID());
    
	return LWVideoMode(LWVector2i((uint32_t)CGDisplayModeGetWidth(Active), (uint32_t)CGDisplayModeGetHeight(Active)), LWVector2i(72,72), CGDisplayModeGetRefreshRate(Active), Colored32Bit);
}