#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWVideoMode.h"
#include <cstring>
#include <algorithm>

uint32_t LWVideoMode::GetAllDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize){
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
	Display *Dsp = XOpenDisplay(nullptr);
	int32_t NbrSizes = 0;
	Window Root = DefaultRootWindow(Dsp);
	int32_t RootScreen = XRRRootToScreen(Dsp, Root);
	XRRScreenSize *Screens = XRRSizes(Dsp, RootScreen, &NbrSizes);
	for(int32_t i=0;i<NbrSizes;i++){
		int32_t NbrRates = 0;
		int16_t *Rates = XRRRates(Dsp, RootScreen, i, &NbrRates);
		for(int32_t d=0;d<NbrRates;d++){
			Modes[ModeCount++] = LWVideoMode(LWVector2i(Screens[i].width, Screens[i].height), LWVector2i(), Rates[d], Colored32Bit);
			for(uint32_t k=0;k<ModeCount-1;k++){
				if(Modes[k]==Modes[ModeCount-1]){
					ModeCount--;
					break;
				}
			}
		}
	}
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	XCloseDisplay(Dsp);
	return ModeCount;
}

bool LWVideoMode::SetDisplayTo(const LWVideoMode &Mode){
	Display *Dsp = XOpenDisplay(nullptr);
	int32_t NbrSizes = 0;
	Window Root = DefaultRootWindow(Dsp);
	int32_t RootScreen = XRRRootToScreen(Dsp, Root);
	XRRScreenSize *Screens = XRRSizes(Dsp, RootScreen, &NbrSizes);
	XRRScreenConfiguration *Config = XRRGetScreenInfo(Dsp, Root);
	for (int32_t i = 0; i < NbrSizes; i++){
		int32_t NbrRates = 0;
		int16_t *Rates = XRRRates(Dsp, RootScreen, i, &NbrRates);
		for (int32_t d = 0; d < NbrRates; d++){
			LWVideoMode CMode = LWVideoMode(LWVector2i(Screens[i].width, Screens[i].height), LWVector2i(0, 0), Rates[d], Colored32Bit);
			if(CMode==Mode){
				XRRSetScreenConfigAndRate(Dsp, Config, Root, i, RR_Rotate_0, Rates[d], CurrentTime);
				XRRFreeScreenConfigInfo(Config);
				XCloseDisplay(Dsp);
				return true;
			}
		}
	}
	XRRFreeScreenConfigInfo(Config);
	XCloseDisplay(Dsp);
	return false;
}

uint32_t LWVideoMode::GetDisplayModes(LWVideoMode *Buffer, uint32_t BufferSize, const LWVector4i &SizeRequirements, const LWVector2i &FrequencyRequirements, uint32_t FlagRequirements){
	const uint32_t MaxModes = 256;
	LWVideoMode Modes[MaxModes];
	uint32_t ModeCount = 0;
	Display *Dsp = XOpenDisplay(nullptr);
	int32_t NbrSizes = 0;
	Window Root = DefaultRootWindow(Dsp);
	int32_t RootScreen = XRRRootToScreen(Dsp, Root);
	XRRScreenSize *Screens = XRRSizes(Dsp, RootScreen, &NbrSizes);
	for (int32_t i = 0; i < NbrSizes; i++){
		int32_t NbrRates = 0;
		int16_t *Rates = XRRRates(Dsp, RootScreen, i, &NbrRates);
		for (int32_t d = 0; d < NbrRates; d++){
			Modes[ModeCount] = LWVideoMode(LWVector2i(Screens[i].width, Screens[i].height), LWVector2i(0,0), Rates[d], Colored32Bit);
			if (Modes[ModeCount].GetSize().x < SizeRequirements.x) continue;
			if (Modes[ModeCount].GetSize().y < SizeRequirements.y) continue;
			if (SizeRequirements.z>0 && Modes[ModeCount].GetSize().x>SizeRequirements.z) continue;
			if (SizeRequirements.w>0 && Modes[ModeCount].GetSize().y > SizeRequirements.w) continue;
			if (Modes[ModeCount].GetFrequency() < (uint32_t)FrequencyRequirements.x) continue;
			if (FrequencyRequirements.y >0 && Modes[ModeCount].GetFrequency() > (uint32_t)FrequencyRequirements.y) continue;
			if (FlagRequirements != 0 && (Modes[ModeCount].GetFlag()&FlagRequirements) == 0) continue;
			ModeCount++;
			for (uint32_t k = 0; k < ModeCount-1; k++){
				if (Modes[k] == Modes[ModeCount - 1]){
					ModeCount--;
					break;
				}
			}
		}
	}
	if (Buffer) std::memcpy(Buffer, Modes, sizeof(LWVideoMode)*std::min<uint32_t>(ModeCount, BufferSize));
	XCloseDisplay(Dsp);
	return ModeCount;
}

LWVideoMode LWVideoMode::GetActiveMode(void){
	Display *Dsp = XOpenDisplay(nullptr);
	int32_t NbrSizes = 0;
	Window Root = DefaultRootWindow(Dsp);
	int32_t RootScreen = XRRRootToScreen(Dsp, Root);
	XRRScreenSize *Screens = XRRSizes(Dsp, RootScreen, &NbrSizes);
	if (NbrSizes == 0) return LWVideoMode();

	XRRScreenConfiguration *Config = XRRGetScreenInfo(Dsp, Root);
	uint16_t RotationID;
	int32_t CurrentScreenID = XRRConfigCurrentConfiguration(Config, &RotationID);
	int16_t CurrentRate = XRRConfigCurrentRate(Config);
	LWVideoMode Mode = LWVideoMode(LWVector2i(Screens[CurrentScreenID].width, Screens[CurrentScreenID].height), LWVector2i(0, 0), (uint32_t)CurrentRate, Colored32Bit);
	XRRFreeScreenConfigInfo(Config);
	XCloseDisplay(Dsp);
	return Mode;
}