#ifndef LWALLOCATOR_DEFAULTDEBUG_H
#define LWALLOCATOR_DEFAULTDEBUG_H
#include "LWCore/LWAllocator.h"
#include <mutex>

/*! \addtogroup LWAllocator
@{
*/
/*! \brief Default allocator that can help track down unfreed memory allocations, uses malloc/free internal buffer.
*/
class LWAllocator_DefaultDebug : public LWAllocator {
public:
	void OutputUnfreedIDs(void);

	void SetCrashID(uint32_t ID);

	LWAllocator_DefaultDebug();

	~LWAllocator_DefaultDebug();
private:
	virtual void *AllocateMemory(uint32_t Length);

	virtual void *DeallocateMemory(void *Memory);

	virtual void *AllocateBytes(uint32_t Length);

	virtual void *DeallocateBytes(void *Memory);

	std::atomic<uint32_t> m_NextID;
	std::mutex m_Lock;
	uint32_t m_CrashID;
	void *m_FirstAllocation;

};
/* @} */
#endif