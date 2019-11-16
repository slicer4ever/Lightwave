#ifndef LWALLOCATOR_LOCALCIRCULAR_H
#define LWALLOCATOR_LOCALCIRCULAR_H
#include "LWCore/LWAllocator.h"
#include <cstdint>
/*! \addtogroup LWAllocator
	@{
*/

/*! \brief A circular buffer for allocating short lived temporary objects.  a pointer is constantly implemented forward until it reaches the end of the buffer where it circles to the beginning.
*/
class LWAllocator_LocalCircular : public LWAllocator{
public:

	/*! \brief allocates a buffer of BufferSize 
		\param BufferSize the size of the buffer that's allocatable.
	*/
	LWAllocator_LocalCircular(uint32_t BufferSize);

	~LWAllocator_LocalCircular();
protected:
	virtual void *AllocateBytes(uint32_t Length);

	virtual void *DeallocateBytes(void *Memory);

	uint8_t *m_Buffer; /*!< \brief the buffer used for allocation. */
	uint32_t m_BufferSize; /*!< \brief the size of the buffer for allocations. */
	uint32_t m_BufferPosition = 0; /*!< \brief the current position of the buffer stream. */

};
/*! @} */
#endif