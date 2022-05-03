#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWUnicode.h"
#include <errno.h>
#include <unistd.h>

extern char **env;

int main(int argc, char **argv){
	const uint32_t MaxIterList = 32;
	LWUTF8Iterator IterList[MaxIterList];
	uint32_t Cnt = std::min<uint32_t>(argc, MaxIterList);
	for (uint32_t i = 0; i < Cnt; i++) IterList[i] = LWUTF8Iterator((const char8_t*)argv[i]);
	errno = 0;
	return LWMain(Cnt, IterList);
}

bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData) {
	uint64_t Prev = LWTimer::GetCurrent();
	while (true) {
		uint64_t Curr = LWTimer::GetCurrent();
		if (Curr - Prev >= Frequency) {
			if (!LoopFunc(UserData)) break;
			Prev += Frequency;
		} else std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return true;
}

bool LWExecute(const LWUTF8Iterator &BinaryPath, const LWUTF8Iterator &Parameters) {
	auto CHMod = LWUTF8I::Fmt<256>("chmod +x {}", BinaryPath);
	auto Exec = LWUTF8I::Fmt<256>("{} {} &", BinaryPath, Parameters);
	system(*CHMod);
	system(*Exec);
	return true;
}

bool LWEmail(const LWUTF8Iterator &SrcEmail, const LWUTF8Iterator &TargetEmail, const LWUTF8Iterator &Subject, const LWUTF8Iterator &Body, const LWUTF8Iterator &SMTPServer, const LWUTF8Iterator &SMTPUsername, const LWUTF8Iterator &SMTPPassword) {
	return false;
}

bool LWBrowser(const LWUTF8Iterator &URL) {
	return false;
}

float LWSystemScale(void) {
	return 1.0f;
}