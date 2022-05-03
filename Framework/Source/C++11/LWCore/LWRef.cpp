#include <LWCore/LWRef.h>


uint32_t LWRef_Counter::AddRef(void) {
	return UseCount++;
}

uint32_t LWRef_Counter::RemoveRef(void) {
	uint32_t Count = UseCount--;
	assert(Count > 0);
	if (Count > 1) return Count;
	//Since this control block+data are allocated together, we safely know where the data should begin, and where the destructor should be called on it.
	DeleteFunc((char *)this + sizeof(LWRef_Counter));
	LWAllocator::Destroy((char *)this);
	return Count;
}

LWRef_Counter::LWRef_Counter(LWRefDeleteFunc DeleteFunc) : DeleteFunc(DeleteFunc) {}