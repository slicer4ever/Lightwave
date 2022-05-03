#include "LWPlatform/LWApplication.h"
#include "LWCore/LWTypes.h"
#include "LWCore/LWUnicode.h"
#include <functional>
#include "LWCore/LWTimer.h"
#include "LWPlatform/LWPlatform.h"

struct LoopedFunc {
	std::function<bool(void*)> m_Func;
	uint64_t m_Frequency;
	uint64_t m_Next;
	void *m_UserData;
};

const uint32_t MaxLoopedFuncs = 32;
LoopedFunc LWLoopedFuncs[MaxLoopedFuncs];
uint32_t LWLoopedFuncCnt = 0;

bool LWExecute(const LWUTF8Iterator &BinaryPath, const LWUTF8Iterator &Parameters) {
	return false;
}

bool LWRunLoop(std::function<bool(void*)> MainLoopFunc, uint64_t Frequency, void *UserData) {
	if (LWLoopedFuncCnt >= MaxLoopedFuncs) return false;
	LWLoopedFuncs[LWLoopedFuncCnt++] = { MainLoopFunc, Frequency, LWTimer::GetCurrent(), UserData };

	return true;
}

bool LWEmail(const LWUTF8Iterator &SrcEmail, const LWUTF8Iterator &TargetEmail, const LWUTF8Iterator &Subject, const LWUTF8Iterator &Body, const LWUTF8Iterator &SMTPServer, const LWUTF8Iterator &SMTPUsername, const LWUTF8Iterator &SMTPPassword) {
	return false;
}

bool LWBrowser(const LWUTF8Iterator &URL) {
	return false;
}

int main(int argc, char **argv) {
	const uint32_t MaxIterList = 32;
	LWUTF8Iterator IterList[MaxIterList];
	uint32_t Cnt = std::min<uint32_t>(argc, MaxIterList);
	for (uint32_t i = 0; i < Cnt; i++) IterList[i] = LWUTF8Iterator((const char8_t*)argv[i]);

	uint32_t Res = LWMain(Cnt, IterList);
	auto Looper = [] {
		uint64_t Current = LWTimer::GetCurrent();
		for (uint32_t i = 0; i < LWLoopedFuncCnt; i++) {
			if (Current < LWLoopedFuncs[i].m_Next) continue;
			LWLoopedFuncs[i].m_Next += LWLoopedFuncs[i].m_Frequency;
			if (!LWLoopedFuncs[i].m_Func(LWLoopedFuncs[i].m_UserData)) {
				std::swap(LWLoopedFuncs[i], LWLoopedFuncs[LWLoopedFuncCnt - 1]);
				i--;
				LWLoopedFuncCnt--;
			}
		}
	};

	emscripten_set_main_loop(Looper, 0, false);
	return Res;
}

float LWSystemScale(void) {
	return 1.0f;
}