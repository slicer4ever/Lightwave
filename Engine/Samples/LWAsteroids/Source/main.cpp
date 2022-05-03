#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_DefaultDebug.h>
#include <LWCore/LWLogger.h>
#include "App.h"

LWLOG_MULTIFILE("Log", true, 3)

int32_t LWMain(int32_t argc, LWUTF8I *argv) {
	LWAllocator_Default Allocator;
	//LWAllocator_DefaultDebug Allocator;
	{
		LWLogEvent("Starting.");
		App A(Allocator);
		A.Run();
	}

	LWLogCriticalIf<256>(Allocator.GetAllocatedBytes()==0, "MEMORY LEAK DETECTED: {}", Allocator.GetAllocatedBytes());
	return 0;
}