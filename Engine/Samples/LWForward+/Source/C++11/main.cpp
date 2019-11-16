#include "App.h"
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWPlatform/LWThread.h>
#include <LWPlatform/LWApplication.h>
#include <LWCore/LWTimer.h>

void UpdateT(LWThread *T) {
	App *A = (App*)T->GetUserData();
	while (!A->isTerminate()) A->Update(LWTimer::GetCurrent());
	return;
}


int LWMain(int argc, char **argv) {
	LWAllocator_Default DefAlloc;
	App *A = DefAlloc.Allocate<App>(nullptr, DefAlloc);
	LWThread UThread(UpdateT, A);
	while (!A->isTerminate()) {
		uint64_t Current = LWTimer::GetCurrent();
		A->ProcessInput(Current).Render(Current);
	}
	UThread.Join();
	LWAllocator::Destroy(A);
	return 0;
}

