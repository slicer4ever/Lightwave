#include "LWCore/LWAllocators/LWAllocator_ConcurrentCircular.h"

LWAllocator_ConcurrentCircular::LWAllocator_ConcurrentCircular(uint32_t BufferSize) : m_Buffer(new uint8_t[BufferSize]), m_BufferSize(BufferSize) {
	m_BufferPosition.store(0);
}

LWAllocator_ConcurrentCircular::~LWAllocator_ConcurrentCircular() {
	delete[] m_Buffer;
}

void *LWAllocator_ConcurrentCircular::AllocateBytes(uint32_t Length) {
	if (Length > m_BufferSize) return nullptr;
	uint32_t CachedPos = m_BufferPosition.load();
	uint32_t RealPos = 0;
	uint32_t TargetNew = 0;
	do {
		RealPos = CachedPos;
		TargetNew = CachedPos + Length;
		if (TargetNew > m_BufferSize) {
			TargetNew = Length;
			RealPos = 0;
		}
	} while (!m_BufferPosition.compare_exchange_weak(CachedPos, TargetNew));
	uint8_t *Ptr = m_Buffer + RealPos;
	return Ptr;
}

void *LWAllocator_ConcurrentCircular::DeallocateBytes(void *) {
	return nullptr;
}
