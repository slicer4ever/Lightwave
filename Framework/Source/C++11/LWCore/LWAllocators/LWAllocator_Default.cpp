#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include <cstdlib>

void *LWAllocator_Default::AllocateBytes(uint32_t Length){
	return malloc(Length);
}

void *LWAllocator_Default::DeallocateBytes(void *Memory){
	free(Memory);
	return nullptr;
}

LWAllocator_Default::~LWAllocator_Default(){}
