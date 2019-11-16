#include "LWCore/LWAllocators/LWAllocator_LocalCircular.h"

LWAllocator_LocalCircular::LWAllocator_LocalCircular(uint32_t BufferSize) : m_Buffer(new uint8_t[BufferSize]), m_BufferSize(BufferSize){}

LWAllocator_LocalCircular::~LWAllocator_LocalCircular(){
	delete[] m_Buffer;
}

void *LWAllocator_LocalCircular::AllocateBytes(uint32_t Length){
	if (Length > m_BufferSize) return nullptr;
	if (m_BufferPosition + Length > m_BufferSize) m_BufferPosition = 0;
	uint8_t *Ptr = m_Buffer+m_BufferPosition; 
	m_BufferPosition += Length;
	return Ptr;
}

void *LWAllocator_LocalCircular::DeallocateBytes(void *){
	return nullptr;
}
