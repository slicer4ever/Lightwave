#include "LWCore/LWAllocators/LWAllocator_DefaultDebug.h"
#include <iostream>
#include <cassert>

/*! \cond */
struct LWAllocatorEnvironmentDebug {
	uint32_t m_AllocID;
	void *m_NextAlloc;
	void *m_PrevAlloc;
	uint32_t m_Size;
	LWAllocator *m_Allocator;
};
/*! \endcond */

void LWAllocator_DefaultDebug::OutputUnfreedIDs(void) {
	if (!m_AllocatedBytes) {
		std::cout << "No leaks detected." << std::endl;
		return;
	}
	if (!m_FirstAllocation) {
		std::cout << "Leak detected: " << m_AllocatedBytes << " However allocator detector failed." << std::endl;
		return;
	}
	std::cout << "Remaining: " << m_AllocatedBytes << std::endl;
	uint32_t c = 0;
	LWAllocatorEnvironmentDebug *A = (LWAllocatorEnvironmentDebug*)m_FirstAllocation;
	for (; A; A = (LWAllocatorEnvironmentDebug*)A->m_NextAlloc) {
		std::cout << "MemoryID: " << A->m_AllocID << " Allocated: " << A->m_Size << std::endl;
		c += (A->m_Size+sizeof(LWAllocatorEnvironmentDebug));
	}
	if (c != m_AllocatedBytes) std::cout << "Error, untracked memory lost." << std::endl;
	return;
}

void LWAllocator_DefaultDebug::SetCrashID(uint32_t ID) {
	m_CrashID = ID;
	return;
}

LWAllocator_DefaultDebug::LWAllocator_DefaultDebug() : m_NextID(0), m_CrashID(0xFFFFFFFF), m_FirstAllocation(nullptr) {}

LWAllocator_DefaultDebug::~LWAllocator_DefaultDebug() {}

void *LWAllocator_DefaultDebug::AllocateMemory(uint32_t Length) {
	void *Memory = AllocateBytes(Length + sizeof(LWAllocatorEnvironmentDebug));
	if (!Memory) return nullptr;
	LWAllocatorEnvironmentDebug *Env = (LWAllocatorEnvironmentDebug*)Memory;
	assert(m_NextID != m_CrashID);
	Env->m_AllocID = m_NextID++;
	Env->m_NextAlloc = m_FirstAllocation;
	Env->m_PrevAlloc = nullptr;
	if (m_FirstAllocation) ((LWAllocatorEnvironmentDebug*)m_FirstAllocation)->m_PrevAlloc = Env;
	m_FirstAllocation = Env;

	Env->m_Allocator = this;
	Env->m_Size = Length;
	m_AllocatedBytes += (Length + sizeof(LWAllocatorEnvironmentDebug));
	return ((int8_t*)Memory) + sizeof(LWAllocatorEnvironmentDebug);
}

void *LWAllocator_DefaultDebug::DeallocateMemory(void *Memory) {
	LWAllocatorEnvironmentDebug *Env = (LWAllocatorEnvironmentDebug*)(((int8_t*)Memory) - sizeof(LWAllocatorEnvironmentDebug));
	
	if (Env->m_NextAlloc) ((LWAllocatorEnvironmentDebug*)Env->m_NextAlloc)->m_PrevAlloc = Env->m_PrevAlloc;
	if (Env->m_PrevAlloc) ((LWAllocatorEnvironmentDebug*)Env->m_PrevAlloc)->m_NextAlloc = Env->m_NextAlloc;
	if (Env == m_FirstAllocation) m_FirstAllocation = Env->m_NextAlloc;
	m_AllocatedBytes -= (Env->m_Size + sizeof(LWAllocatorEnvironmentDebug));
	return DeallocateBytes(Env);
}

void *LWAllocator_DefaultDebug::AllocateBytes(uint32_t Length) {
	return malloc(Length);
}

void *LWAllocator_DefaultDebug::DeallocateBytes(void *Memory) {
	free(Memory);
	return nullptr;
}