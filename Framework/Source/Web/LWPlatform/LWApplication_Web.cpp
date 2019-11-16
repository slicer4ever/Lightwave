#include "LWPlatform/LWApplication.h"
#include "LWCore/LWTypes.h"
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

bool LWExecute(const char *BinaryPath, const char *Parameters) {
	return false;
}

bool LWRunLoop(std::function<bool(void*)> MainLoopFunc, uint64_t Frequency, void *UserData) {
	if (LWLoopedFuncCnt >= MaxLoopedFuncs) return false;
	LWLoopedFuncs[LWLoopedFuncCnt++] = { MainLoopFunc, Frequency, LWTimer::GetCurrent(), UserData };

	return true;
}

bool LWEmail(const char *SrcEmail, const char *TargetEmail, const char *Subject, const char *Body, const char *SMTPServer, const char *SMTPUsername, const char *SMTPPassword) {
	return false;
}

bool LWBrowser(const char *URL) {
	return false;
}

int main(int argc, char **argv) {
	uint32_t Res = LWMain(argc, argv);
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