#include "LWCore/LWAllocators/LWAllocator_LocalHeap.h"
#include <iostream>

/*! \cond */

struct alignas(16) LWAllocator_LocalHeapEnvironment{
	LWAllocator_LocalHeapEnvironment *m_Next = nullptr;
	alignas(8) void *m_PadP = nullptr; //Padded pointer
	alignas(8) uint32_t m_Size = 0;
	uint32_t m_Pad = 0;
	alignas(8) LWAllocator *m_Allocator = nullptr;

	LWAllocator_LocalHeapEnvironment *NextEnvPos(void) {
		return (LWAllocator_LocalHeapEnvironment*)(((int8_t*)this) + m_Size);
	}

	LWAllocator_LocalHeapEnvironment(LWAllocator_LocalHeapEnvironment *Next, uint32_t Size, LWAllocator *Alloc) : m_Next(Next), m_Size(Size), m_Allocator(Alloc) {}
};
/*! \endcond */

void *LWAllocator_LocalHeap::AllocateMemory(uint32_t Length){
	LWAllocator_LocalHeapEnvironment *NextAllocUnit = (LWAllocator_LocalHeapEnvironment*)m_NextAllocationUnit;
	LWAllocator_LocalHeapEnvironment *PrevAllocUnit = nullptr;
	LWAllocator_LocalHeapEnvironment *Env = NextAllocUnit;
	Length += sizeof(LWAllocator_LocalHeapEnvironment);
	while (Env && Env->m_Size <= Length) {
		PrevAllocUnit = Env;
		Env = Env->m_Next;
	}
	if (!Env) return nullptr; //could not find suitable location for placing memory.
	if (Env->m_Size > Length) {
		LWAllocator_LocalHeapEnvironment *nHeap = (LWAllocator_LocalHeapEnvironment*)(((int8_t*)Env) + Length);
		*nHeap = LWAllocator_LocalHeapEnvironment(Env->m_Next, Env->m_Size - Length, this);
		Env->m_Next = nHeap;
	}
	Env->m_Size = Length - sizeof(LWAllocator_LocalHeapEnvironment);
	if (!PrevAllocUnit) m_NextAllocationUnit = Env->m_Next;
	else PrevAllocUnit->m_Next = Env->m_Next;
	m_AllocatedBytes += Length;
	return ((int8_t*)Env) + sizeof(LWAllocator_LocalHeapEnvironment);
}

void *LWAllocator_LocalHeap::DeallocateMemory(void *Memory){
	LWAllocator_LocalHeapEnvironment *Env = (LWAllocator_LocalHeapEnvironment *)((int8_t*)Memory - sizeof(LWAllocator_LocalHeapEnvironment));
	LWAllocator_LocalHeapEnvironment *NextAllocUnit = (LWAllocator_LocalHeapEnvironment*)m_NextAllocationUnit;
	LWAllocator_LocalHeapEnvironment *PrevAllocUnit = nullptr;
	LWAllocator_LocalHeapEnvironment *PrevPrevAllocUnit = nullptr; //Needed in case we merge with PrevAllocUnit
	//Updated allocated bytes!
	Env->m_Size += sizeof(LWAllocator_LocalHeapEnvironment);
	m_AllocatedBytes -= Env->m_Size;
	for (; NextAllocUnit < Env; NextAllocUnit = NextAllocUnit->m_Next) {
		PrevPrevAllocUnit = PrevAllocUnit;
		PrevAllocUnit = NextAllocUnit;
	}
	//See if we can merge with prev:
	if (PrevAllocUnit) {
		if (PrevAllocUnit->NextEnvPos() == Env) {
			PrevAllocUnit->m_Size += Env->m_Size;
			Env = PrevAllocUnit;
			PrevAllocUnit = PrevPrevAllocUnit;
		}
	}

	//See if we can merge with Next:
	if (NextAllocUnit) {
		if (Env->NextEnvPos() == NextAllocUnit) {
			Env->m_Size += NextAllocUnit->m_Size;
			NextAllocUnit = NextAllocUnit->m_Next;
		}
	}
	//Update starting allocation unit's:
	if (PrevAllocUnit) PrevAllocUnit->m_Next = Env;
	else m_NextAllocationUnit = Env;
	Env->m_Next = NextAllocUnit;
	return nullptr;
}

void *LWAllocator_LocalHeap::AllocateBytes(uint32_t){
	return nullptr;
}

void *LWAllocator_LocalHeap::DeallocateBytes(void *){
	return nullptr;
}

LWAllocator_LocalHeap::LWAllocator_LocalHeap(uint32_t BufferSize) : LWAllocator(), m_Buffer(new uint8_t[BufferSize]), m_BufferSize(BufferSize){
	m_NextAllocationUnit = (LWAllocator_LocalHeapEnvironment*)m_Buffer;
	LWAllocator_LocalHeapEnvironment *Env = (LWAllocator_LocalHeapEnvironment*)m_NextAllocationUnit;
	*Env = LWAllocator_LocalHeapEnvironment(nullptr, m_BufferSize, this);
}

LWAllocator_LocalHeap::~LWAllocator_LocalHeap(){
	delete[] m_Buffer;
}