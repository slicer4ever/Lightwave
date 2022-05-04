#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWUnicode.h"
#include "LWCore/LWTimer.h"
#include <signal.h>
#include <unistd.h>
#include <spawn.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <iostream>

void *LWSignal_UserData[LWSignal_Unknown];
LWSignalHandlerFunc LWSignal_Funcs[LWSignal_Unknown];

void LWSignal_Handler(int32_t Int) {
	uint32_t LWSignalIDs[5] = { LWSignal_CtrlC, LWSignal_Break, LWSignal_Close, LWSignal_Logoff, LWSignal_Shutdown };
	int32_t SignalMap[5] = {SIGINT, SIGTERM, SIGTERM, SIGTERM, SIGTERM};
	int32_t Signal = 0;
	for(;Signal<LWSignal_Unknown && SignalMap[Signal]!=Int;++Signal){}
	if (Signal >= LWSignal_Unknown) return;
	uint32_t ID = LWSignalIDs[Signal];
	if (ID >= LWSignal_Unknown) return;
	if (!LWSignal_Funcs[ID]) return;
	LWSignal_Funcs[ID](Signal, LWSignal_UserData[ID]);
	return;
};

void LWRegisterSignal(LWSignalHandlerFunc Handler, uint32_t Signal, void *UserData) {
	LWSignal_UserData[Signal] = UserData;
	LWSignal_Funcs[Signal] = Handler;
	return;
}

int main(int argc, char **argv){
	const uint32_t MaxIterList = 32;
	LWUTF8Iterator IterList[MaxIterList];
	uint32_t Cnt = std::min<uint32_t>(argc, MaxIterList);
	for (uint32_t i = 0; i < Cnt; i++) IterList[i] = LWUTF8Iterator((const char8_t*)argv[i]);
	signal(SIGINT, LWSignal_Handler);
	signal(SIGTERM, LWSignal_Handler);
	return LWMain(Cnt, IterList);
}

bool LWExecute(const LWUTF8Iterator &BinaryPath, const LWUTF8Iterator &Parameters) {
	auto CHMod = LWUTF8I::Fmt<256>("chmod +x {}", BinaryPath);
	auto Exec = LWUTF8I::Fmt<256>("{} {} &", BinaryPath, Parameters);
	system(*CHMod);
	system(*Exec);
	return true;
}

bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData) {
	uint64_t Prev = LWTimer::GetCurrent();
	while (true) {
		uint64_t Curr = LWTimer::GetCurrent();
		if (Curr - Prev >= Frequency) {
			if (!LoopFunc(UserData)) break;
			Prev += Frequency;
		}
	}
	return true;
}

bool LWEmail(const LWUTF8Iterator &SrcEmail, const LWUTF8Iterator &TargetEmail, const LWUTF8Iterator &Subject, const LWUTF8Iterator &Body, const LWUTF8Iterator &SMTPServer, const LWUTF8Iterator &SMTPUsername, const LWUTF8Iterator &SMTPPassword){
	return false;
}

bool LWBrowser(const LWUTF8Iterator &URL) {
	return false;
}

float LWSystemScale(void) {
	return 1.0f;
}