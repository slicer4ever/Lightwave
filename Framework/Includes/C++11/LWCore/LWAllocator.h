#ifndef LWALLOCATOR_H
#define LWALLOCATOR_H
#include <memory>
#include <new>
#include <cstdint>

/*! \defgroup LWAllocator LWAllocator
	\ingroup LWCore
	\brief LWAllocator group of allocation schemes used by the framework.
	@{
*/
/*!
	\brief LWAllocator is a pure abstract allocation class that can be overloaded to provide a variety of allocation schemes.
	LWAllocator provides a series of virtual methods to be used by other components for construction and destruction of components.
	Every component in the LWFramework rely's on the assumption that allocations are done via LWAllocator, and as such a crash will occur if you pass in memory that wasn't allocated via an LWAllocator.

*/

class LWAllocator{
public:
	/*! \brief Obtains the allocator used to allocate the memory. 
		\return returns null if the allocator is invalid, otherwise returns the allocator object.
	*/
	static LWAllocator *GetAllocator(void *Memory);

	/*! \brief Obtains the size of the allocation used when allocating memory.
	*/
	static uint32_t GetAllocationSize(void *Memory);

	/*! \brief automatically finds the allocator used in allocation, and destroys it.  returns null if successful, otherwise returns data if unsuccessful.
		\param Memory The memory to destroy.
		\return null on success, Memory on failure.
	*/
	template<class Type>
	static Type *Destroy(Type *Memory){
		if (!Memory) return Memory;
		LWAllocator *Alloc = GetAllocator(Memory);
		uint32_t AllocSize = GetAllocationSize(Memory);
		uint32_t Count = AllocSize / sizeof(Type);
		if (!std::is_trivial<Type>::value) {
			for (uint32_t i = 0; i < Count; i++) Memory[i].~Type();
		}
		return (Type*)Alloc->Deallocate(Memory);
	}

	/*! \brief Allocates memory from the internal allocation buffer.
		\return a pointer to the memory allocated, or null if allocation was unsuccessful.
	*/
	template<class Type, typename... Args>
	Type *Allocate(Args&&... Arg){
		Type *Mem = (Type*)AllocateMemory(sizeof(Type));
		if (!Mem) return Mem;
		return new(Mem) Type(std::forward<Args>(Arg)...);
	}

	/*! \brief Allocates n objects from the internal allocation buffer.
		\param Length the number of objects to allocate.
		\return a pointer to the memory allocated, or null if allocation was unsuccessful.
	*/
	template<class Type>
	Type *AllocateArray(uint32_t Length){
		Type *Mem = (Type*)AllocateMemory(sizeof(Type)*Length);
		if (!Mem) return Mem;
		if (!std::is_trivial<Type>::value) {
			for (uint32_t i = 0; i < Length; i++) new(Mem + i) Type();
		}
		return Mem;
		
	}

	/*! \brief Deallocates the memory from the internal allocation buffer.
		\param Memory pointer to the memory to be released.
		\return nullptr on success, or the memory object if failed.
	*/
	void *Deallocate(void *Memory);

	/*! \brief returns the number of bytes still in allocation.
	*/
	uint32_t GetAllocatedBytes(void);
protected:
	uint32_t m_AllocatedBytes = 0; /*!< \brief the total number of bytes still allocated, this value does include the meta data that is also allocated with LWAllocators. */

	/*! \brief the allocate memory function which allocates not only the memory requested, but additional meta data for tracking allocations. 
		\param Length the number of bytes to allocate.
		\return null if unable to allocate space, otherwise a pointer to the memory requested.
	*/

	virtual void *AllocateMemory(uint32_t Length);

	/*! \brief deallocates requested memory with supplementary meta data placed before Memory.
		\param Memory pointer to the data to be destroyed.
		\return null on success, Memory if failed to deallocate.
	*/
	virtual void *DeallocateMemory(void *Memory);

	/*! \brief allocates the raw bytes if no additional meta data is necessary for the allocator, only overload this method, however if additional meta data is required, overload AllocateMemory instead.
		\param Length the raw number of bytes requested.
		\return null if unable to allocate space, otherwise a pointer to the memory requested.
	*/
	virtual void *AllocateBytes(uint32_t Length) = 0;

	/*! \brief deallocates the pointer the memory points to if possible.  If no additional meta data is necessary for the allocator, only overload this method.  However if additional meta data is required, overload DeallocateMemory instead.
		\param Memory the pointer to the data to be deallocated.
	*/
	virtual void *DeallocateBytes(void *Memory) = 0;
};
/*! @} */

#endif
