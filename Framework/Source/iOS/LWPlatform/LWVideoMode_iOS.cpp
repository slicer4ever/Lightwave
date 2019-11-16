#include "LWPlatform/LWVideoMode.h"
#include "LWPlatform/LWPlatform.h"
#include <cstring>
#include <algorithm>
#import <UIKit/UIKit.h>

uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize){
	if (Buffer) *Buffer = GetActiveMode();
    return 1;
}

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode){
    return false;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements){
	LWVideoMode Mode = GetActiveMode();
    if (Mode.GetSize().x < SizeRequirements.x) return 0;
    if (Mode.GetSize().y < SizeRequirements.y) return 0;
    if (SizeRequirements.z>0 && Mode.GetSize().x>SizeRequirements.z) return 0;
    if (SizeRequirements.w>0 && Mode.GetSize().y > SizeRequirements.w) return 0;
    if (Mode.GetFrequency() < (uint32_t)FrequencyRequirements.x) return 0;
    if (FrequencyRequirements.y >0 && Mode.GetFrequency() > (uint32_t)FrequencyRequirements.y) return 0;
    if (FlagRequirements != 0 && (Mode.GetFlag()&FlagRequirements) == 0) return 0;
	
	if (Buffer) *Buffer = Mode;
    return 1;
}

LWVideoMode LWVideoMode::GetActiveMode(void){
    CGRect ScreenSize = [[UIScreen mainScreen] bounds];
    CGFloat ScreenScale = [[UIScreen mainScreen] scale];
    
    int32_t DPI = [[UIScreen mainScreen] nativeScale] * ((UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) ? 132 : 163);
    //AFAIK all iOS devices run at 60Hz
	return LWVideoMode(LWVector2i(ScreenSize.size.width*ScreenScale, ScreenSize.size.height*ScreenScale), LWVector2i(DPI, DPI), 60, Colored32Bit);
}