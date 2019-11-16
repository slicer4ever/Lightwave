#include "LWCore/LWAllocators/LWAllocator_LocalHeap.h"

#include <iostream>

/*! \cond */

struct LWAllocator_LocalHeapEnvironment{
	LWAllocator_LocalHeapEnvironment *m_Next;
	LWAllocator_LocalHeapEnvironment *m_Prev;
	uint32_t m_Size;
	LWAllocator *m_Allocator;
};
/*! \endcond */

void *LWAllocator_LocalHeap::AllocateMemory(uint32_t Length){
	LWAllocator_LocalHeapEnvironment *Env = (LWAllocator_LocalHeapEnvironment*)m_NextAllocationUnit;
	while (Env && Env->m_Size < Length) Env = Env->m_Next;
	if (!Env) return nullptr; //could not find suitable location for placing memory.
	if (Env->m_Size > Length + sizeof(LWAllocator_LocalHeapEnvironment)){
		LWAllocator_LocalHeapEnvironment *NewHeap = (LWAllocator_LocalHeapEnvironment*)(((int8_t*)Env) + sizeof(LWAllocator_LocalHeapEnvironment)+Length);
		NewHeap->m_Prev = Env;
		NewHeap->m_Next = Env->m_Next;
		Env->m_Next = NewHeap;
		NewHeap->m_Size = Env->m_Size - Length - sizeof(LWAllocator_LocalHeapEnvironment);
		NewHeap->m_Allocator = this;
	}else Length = Env->m_Size;
	if (Env == (LWAllocator_LocalHeapEnvironment*)m_NextAllocationUnit) m_NextAllocationUnit = Env->m_Next;
	Env->m_Size = 0;
	m_AllocatedBytes += Length;
	return ((int8_t*)Env) + sizeof(LWAllocator_LocalHeapEnvironment);
}

void *LWAllocator_LocalHeap::DeallocateMemory(void *Memory){
	LWAllocator_LocalHeapEnvironment *Env = (LWAllocator_LocalHeapEnvironment *)((int8_t*)Memory - sizeof(LWAllocator_LocalHeapEnvironment));
	LWAllocator_LocalHeapEnvironment *Left = Env->m_Prev && Env->m_Prev->m_Size != 0 ? Env->m_Prev : Env;
	LWAllocator_LocalHeapEnvironment *Right = Env->m_Next && Env->m_Next->m_Size!=0 ? Env->m_Next->m_Next : Env->m_Next;
	
	//Updated allocated bytes!
	uintptr_t MemSize = 0;
	if (!Env->m_Next) MemSize = ((uintptr_t)(m_Buffer + m_BufferSize) - (uintptr_t)Env) - sizeof(LWAllocator_LocalHeapEnvironment);
	else MemSize = (uintptr_t)((int8_t*)Env->m_Next - (int8_t*)Env) - sizeof(LWAllocator_LocalHeapEnvironment);
	m_AllocatedBytes -= (uint32_t)MemSize;

	Left->m_Next = Right;
	if (Right){
		Right->m_Prev = Left;
		Left->m_Size = (uint32_t)((uintptr_t)((int8_t*)Right - (int8_t*)Left) - sizeof(LWAllocator_LocalHeapEnvironment));
	}else Left->m_Size = (uint32_t)((uintptr_t)((m_Buffer + m_BufferSize) - (uint8_t*)Left) - sizeof(LWAllocator_LocalHeapEnvironment));


	if ((int8_t*)Left < m_NextAllocationUnit) m_NextAllocationUnit = Left;
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
	Env->m_Next = nullptr;
	Env->m_Prev = nullptr;
	Env->m_Size = m_BufferSize - sizeof(LWAllocator_LocalHeapEnvironment);
	Env->m_Allocator = this;
}

LWAllocator_LocalHeap::~LWAllocator_LocalHeap(){
	delete[] m_Buffer;
}