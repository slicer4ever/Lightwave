#ifndef LWALLOCATOR_DEFAULT_H
#define LWALLOCATOR_DEFAULT_H
#include "LWCore/LWAllocator.h"

/*! \addtogroup LWAllocator
	@{
*/
/*! \brief Default allocator using c's malloc/free internal buffer.  note: AllocatedBytes may return incorrect values if used in a multi-threaded system.
*/
class LWAllocator_Default : public LWAllocator{
public:

	~LWAllocator_Default();
private:
	virtual void *AllocateBytes(uint32_t Length);

	virtual void *DeallocateBytes(void *Memory);
};
/* @} */
#endif