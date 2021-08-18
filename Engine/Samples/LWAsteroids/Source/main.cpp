#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_DefaultDebug.h>
#include "LWELogger.h"
#include "App.h"

int32_t LWMain(int32_t argc, LWUTF8I *argv) {
	LWAllocator_Default Allocator;
	//LWAllocator_DefaultDebug Allocator;
	{
		App A(Allocator);
		A.Run();
	}
	if (Allocator.GetAllocatedBytes()) {
		//Allocator.OutputUnfreedIDs();
		LWELogCritical<256>("MEMORY LEAK DETECTED: {}\n", Allocator.GetAllocatedBytes());
	}
	return 0;
}