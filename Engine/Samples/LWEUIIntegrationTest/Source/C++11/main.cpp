#include <LWCore/LWTypes.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_DefaultDebug.h>
#include "App.h"

int32_t LWMain(int32_t argc, LWUTF8Iterator *Argv) {
	//LWAllocator_DefaultDebug DefAlloc;
	LWAllocator_Default DefAlloc;
	App *A = DefAlloc.Create<App>(DefAlloc);
	A->Run();
	LWAllocator::Destroy(A);
	if (DefAlloc.GetAllocatedBytes()) {
		//DefAlloc.OutputUnfreedIDs();
		fmt::print("Memory leak detected: {}\n", DefAlloc.GetAllocatedBytes());
	}
	return 0;
}