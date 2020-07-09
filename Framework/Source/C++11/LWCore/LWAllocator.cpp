#include "LWCore/LWAllocator.h"

/*! \cond */
struct alignas(16) LWAllocatorEnvironment{
	uint32_t m_Pad;
	uint32_t m_Size;
	alignas(8) LWAllocator *m_Allocator;
};
/*! \endcond */

LWAllocator *LWAllocator::GetAllocator(void *Memory){
	LWAllocatorEnvironment *Enviroment = (LWAllocatorEnvironment *)(((int8_t*)Memory) - sizeof(LWAllocatorEnvironment));
	return Enviroment->m_Allocator;
}

uint32_t LWAllocator::GetAllocationSize(void *Memory){
	LWAllocatorEnvironment *Enviroment = (LWAllocatorEnvironment *)(((int8_t*)Memory) - sizeof(LWAllocatorEnvironment));
	return Enviroment->m_Size;
}

uint32_t LWAllocator::GetAllocatedBytes(void){
	return m_AllocatedBytes;
}

void *LWAllocator::Deallocate(void *Memory){
	return DeallocateMemory(Memory);
}

void *LWAllocator::AllocateMemory(uint32_t Length){
	void *Memory = AllocateBytes(Length + sizeof(LWAllocatorEnvironment));
	if (!Memory) return nullptr;
	LWAllocatorEnvironment *Env = (LWAllocatorEnvironment*)Memory;
	Env->m_Allocator = this;
	Env->m_Size = Length;
	m_AllocatedBytes += (Length+sizeof(LWAllocatorEnvironment));
	return ((int8_t*)Memory) + sizeof(LWAllocatorEnvironment);
}

void *LWAllocator::DeallocateMemory(void *Memory){
	LWAllocatorEnvironment *Env = (LWAllocatorEnvironment*)(((int8_t*)Memory) - sizeof(LWAllocatorEnvironment));
	m_AllocatedBytes -= (Env->m_Size + sizeof(LWAllocatorEnvironment));
	return DeallocateBytes(Env);
}
