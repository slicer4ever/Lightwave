#include "LWCore/LWAllocators/LWAllocator_DefaultDebug.h"
#include <iostream>
#include <cassert>

/*! \cond */
struct alignas(16) LWAllocatorEnvironmentDebug {
	void *m_NextAlloc;
	alignas(8) void *m_PrevAlloc;
	alignas(8) uint32_t m_Size;
	uint32_t m_AllocID;
	alignas(8) LWAllocator *m_Allocator;
};
/*! \endcond */

void LWAllocator_DefaultDebug::OutputUnfreedIDs(void) {
	if (!m_AllocatedBytes) {
		fmt::print("No leaks detected.\n");
		return;
	}
	if (!m_FirstAllocation) {
		fmt::print("Leak detected: {} However allocator detector failed.\n", m_AllocatedBytes);
		return;
	}
	fmt::print("Remaining: {}\n", m_AllocatedBytes);
	uint32_t c = 0;
	LWAllocatorEnvironmentDebug *A = (LWAllocatorEnvironmentDebug*)m_FirstAllocation;
	for (; A; A = (LWAllocatorEnvironmentDebug*)A->m_NextAlloc) {
		fmt::print("MemoryID: {} Allocated: {}\n", A->m_AllocID, A->m_Size);
		c += (A->m_Size+sizeof(LWAllocatorEnvironmentDebug));
	}
	if (c != m_AllocatedBytes) fmt::print("Error, untracked memory lost.\n");
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
	Env->m_AllocID = m_NextID.fetch_add(1);
	assert(Env->m_AllocID != m_CrashID);

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
	LWAllocatorEnvironmentDebug *Env = LWAllocator::GetEnvironment<LWAllocatorEnvironmentDebug>(Memory);
	
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