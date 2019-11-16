#ifndef LWALLOCATOR_LOCALHEAP_H
#define LWALLOCATOR_LOCALHEAP_H
#include "LWCore/LWAllocator.h"

/*! \addtogroup LWAllocator
@{
*/

/*! \brief A local heap allocation buffer for allocating objects without going through the system allocation scheme.
*/
class LWAllocator_LocalHeap : public LWAllocator{
public:

	/*! \brief allocates a buffer of BufferSize */
	LWAllocator_LocalHeap(uint32_t BufferSize);

	~LWAllocator_LocalHeap();
protected:
	virtual void *AllocateMemory(uint32_t Length);

	virtual void *DeallocateMemory(void *Memory);

	virtual void *AllocateBytes(uint32_t Length);

	virtual void *DeallocateBytes(void *Memory);

	uint8_t *m_Buffer; /*!< \brief the buffer used for allocations. */
	void *m_NextAllocationUnit; /*!< \brief the next available allocation unit. */
	uint32_t m_BufferSize; /*!< \brief the total buffer size. */

};
/*! @} */
#endif