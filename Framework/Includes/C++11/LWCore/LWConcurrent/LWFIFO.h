#ifndef LWCONCURRENTFIFO_H
#define LWCONCURRENTFIFO_H
#include <atomic>
#include <LWCore/LWAllocator.h>
#include <cstdio>
/*! \addtogroup LWCore
	@{
*/

/*! \brief this class represents an abstract FIFO queue, it's underlying mechanisms can be overridden for simpler or more complex FIFO queues available for use. */
template<class Type>
class LWFIFO{
public:

	/*! \brief the default pop method to be used from the underlying type. 
		\param Result the result to receive the stored type.
		\param Peek rather to only peek at the value, but not remove it or not.
		\return rather an item was removed or not.
	*/
	virtual bool Pop(Type &Result, bool Peek = false) = 0;

	/*! \brief the default push method to be used from the underlying type.
		\return rather the item was added to the queue to not.
	*/
	virtual bool Push(const Type &Item) = 0;

	/*! \brief returns the number of elements in the queue. */
	virtual uint32_t Length(void) = 0;

};

/*!< \brief this is a concurrent First-in First-Out queue class that supports one thread pushing, and any thread popping. it is written so that only a max finite number of elements can be in the queue at any given time.  MaxElementCount is the number of elements that the FIFO can sustain at any time. */
template<class Type, uint32_t MaxElementCount>
class LWFIFOOneAny : public LWFIFO<Type> {
public:
	/*! \brief removes an item from the list.  note: Peeking is not thread-safe, as it does not gurantee another thread hasn't already read and removed the object before Result is written to.  it is not advised to use peek unless you are certain only a single thread is popping at a time. */
	virtual bool Pop(Type &Result, bool Peek = false) {
		uint32_t ReservePos = m_ReserveReadPos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos == m_WritePos) return false;
			if (Peek) break;
			Target = (ReservePos + 1);// == MaxElementCount ? 0 : ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos % MaxElementCount;
		Result = m_QueueBuffer[Idx];
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief removes an item from the list and returns a pointer to the underlying object.
	\note all operations on the object are considered unsafe, be sure you know what your doing! */
	virtual bool Pop(Type **Result) {
		uint32_t ReservePos = m_ReserveReadPos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos == m_WritePos) return false;
			Target = (ReservePos + 1);
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos % MaxElementCount;
		*Result = &m_QueueBuffer[Idx];
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief removes an item from the list but doesn't increment the internal index until PopFinished is called, allowing the underyling object to be worked on safely.  not calling a PopFinish with the target and reserpos received from this function will stall any and other Pop's that occur */
	virtual bool PopStart(Type **Result, uint32_t &Target, uint32_t &ReservePos) {
		ReservePos = m_ReserveReadPos.load();
		Target = 0;
		do {
			if (ReservePos == m_WritePos) return false;
			Target = ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos % MaxElementCount;
		*Result = &m_QueueBuffer[Idx];
		return true;
	}

	/*!< \brief finishs the pop with the associated target and reservePos, the item is now unsafe to work on after this call completes. */
	virtual bool PopFinshed(uint32_t Target, uint32_t ReservePos) {
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*! \brief pushes an item onto the list. */
	virtual bool Push(const Type &Item) {
		if ((m_WritePos - m_ReadPos.load()) >= MaxElementCount) return false;
		uint32_t Idx = m_WritePos % MaxElementCount;
		m_QueueBuffer[Idx] = Item;
		m_WritePos++;
		return true;
	}

	/*!< \brief pushs an item onto the list and returns a pointer to the underlying object if ItemPtr is not null.
		 \note any operations on the pointer object is considered unsafe.
	*/
	virtual bool Push(const Type &Item, Type **ItemPtr) {
		if ((m_WritePos - m_ReadPos.load()) >= MaxElementCount) return false;
		uint32_t Idx = m_WritePos % MaxElementCount;
		m_QueueBuffer[Idx] = Item;
		if (ItemPtr) *ItemPtr = &m_QueueBuffer[Idx];
		m_WritePos++;
		return true;
	}

	/*!< \brief push an item from the internal list into Item but doesn't increment the internal counter, this let's the internal item be worked on safely before it's commited to the list.  not calling a PushFinish with the returned target and reservepos will cause a stall on any other push operations. */
	virtual bool PushStart(Type **ItemPtr) {
		if ((m_WritePos - m_ReadPos.load()) >= MaxElementCount) return false;
		*ItemPtr = &m_QueueBuffer[m_WritePos%MaxElementCount];
		return true;
	}

	/*!< \brief finshs a corrisponding PushStart call, the item returned from PushStart is now no longer safe to work on. */
	virtual bool PushFinished(void) {
		m_WritePos++;
		return true;
	}

	/*! \brief returns the number of elements in the queue.*/
	virtual uint32_t Length(void) {
		return m_WritePos - m_ReadPos.load();
	}

	/*! \brief constructor for the FIFO object, this object is available for use the moment that this object is returned. */
	LWFIFOOneAny() {
		m_ReserveReadPos.store(0);
		m_ReadPos.store(0);
		m_WritePos = 0;
	}

	/*! \brief destroys the FIFO object, be sure all other threads have given up access to this object before you destroy the Concurrent list. */
	~LWFIFOOneAny() {}
private:

	Type m_QueueBuffer[MaxElementCount];
	std::atomic<uint32_t> m_ReserveReadPos;
	std::atomic<uint32_t> m_ReadPos;
	uint32_t m_WritePos;
};

/*! \brief This is a concurrent First-in First-Out queue class that supports any thread pushing, and any thread popping, at any given time. it is written so that only a max finite number of elements can be in the queue at any given time. MaxElementCount is the number of elements that the concurrent FIFO can sustain at any time.*/
template<class Type, uint32_t MaxElementCount>
class LWConcurrentFIFO : public LWFIFO<Type> {
public:
	/*! \brief removes an item from the list.  note: Peeking is not thread-safe, as it does not gurantee another thread hasn't already read and removed the object before Result is written to.  it is not advised to use peek unless you are certain only a single thread is popping at a time. */
	virtual bool Pop(Type &Result, bool Peek = false) {
		uint32_t ReservePos = m_ReserveReadPos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos == m_WritePos.load()) return false;
			if (Peek) break;
			Target = (ReservePos + 1);// == MaxElementCount ? 0 : ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		Result = m_QueueBuffer[Idx];
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief removes an item from the list and returns a pointer to the underlying object.  
		 \note all operations on the object are considered unsafe, be sure you know what your doing! */
	virtual bool Pop(Type **Result) {
		uint32_t ReservePos = m_ReserveReadPos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos == m_WritePos.load()) return false;
			Target = (ReservePos + 1);
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		*Result = &m_QueueBuffer[Idx];
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief removes an item from the list but doesn't increment the internal index until PopFinished is called, allowing the underyling object to be worked on safely.  not calling a PopFinish with the target and reserpos received from this function will stall any and other Pop's that occur */
	virtual bool PopStart(Type **Result, uint32_t &Target, uint32_t &ReservePos) {
		ReservePos = m_ReserveReadPos.load();
		Target = 0;
		do {
			if (ReservePos == m_WritePos.load()) return false;
			Target = ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveReadPos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		*Result = &m_QueueBuffer[Idx];
		return true;
	}

	/*!< \brief finishs the pop with the associated target and reservePos, the item is now unsafe to work on after this call completes. */
	virtual bool PopFinshed(uint32_t Target, uint32_t ReservePos) {
		uint32_t Expected = ReservePos;
		while (!m_ReadPos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*! \brief pushes an item onto the list. */
	virtual bool Push(const Type &Item) {
		uint32_t ReservePos = m_ReserveWritePos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos - m_ReadPos.load() >= MaxElementCount) return false;
			Target = (ReservePos + 1);// == MaxElementCount ? 0 : ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveWritePos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		m_QueueBuffer[Idx] = (Item);
		uint32_t Expected = ReservePos;
		while (!m_WritePos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief pushs an item onto the list and returns a pointer to the underlying object if ItemPtr is not null.
		 \note any operations on the pointer object is considered unsafe.
	*/
	virtual bool Push(const Type &Item, Type **ItemPtr) {
		uint32_t ReservePos = m_ReserveWritePos.load();
		uint32_t Target = 0;
		do {
			if (ReservePos - m_ReadPos.load() >= MaxElementCount) return false;
			Target = (ReservePos + 1);// == MaxElementCount ? 0 : ReservePos + 1;
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveWritePos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		m_QueueBuffer[Idx] = Item;
		if(ItemPtr) *ItemPtr = &m_QueueBuffer[Idx];
		uint32_t Expected = ReservePos;
		while (!m_WritePos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*!< \brief push an item from the internal list into Item but doesn't increment the internal counter, this let's the internal item be worked on safely before it's commited to the list.  not calling a PushFinish with the returned target and reservepos will cause a stall on any other push operations. */
	virtual bool PushStart(Type **ItemPtr, uint32_t &Target, uint32_t &ReservePos) {
		ReservePos = m_ReserveWritePos.load();
		Target = 0;
		do {
			if (ReservePos - m_ReadPos.load() >= MaxElementCount) return false;
			Target = (ReservePos + 1);
			if (!Target) Target = (ReservePos%MaxElementCount) + 1;
		} while (!m_ReserveWritePos.compare_exchange_weak(ReservePos, Target));
		uint32_t Idx = ReservePos%MaxElementCount;
		*ItemPtr = &m_QueueBuffer[Idx];
		return true;
	}

	/*!< \brief finshs a corrisponding PushStart call, the item returned from PushStart is now no longer safe to work on. */
	virtual bool PushFinished(uint32_t Target, uint32_t ReservePos) {
		uint32_t Expected = ReservePos;
		while (!m_WritePos.compare_exchange_weak(Expected, Target)) Expected = ReservePos;
		return true;
	}

	/*! \brief returns the number of elements in the queue.*/
	virtual uint32_t Length(void) {
		return m_WritePos.load() - m_ReadPos.load();
	}

	/*! \brief constructor for the FIFO object, this object is available for use the moment that this object is returned. */
	LWConcurrentFIFO() {
		m_ReserveWritePos.store(0);
		m_ReserveReadPos.store(0);
		m_WritePos.store(0);
		m_ReadPos.store(0);
	}

	/*! \brief destroys the FIFO object, be sure all other threads have given up access to this object before you destroy the Concurrent list. */
	~LWConcurrentFIFO() {}
private:
	Type m_QueueBuffer[MaxElementCount];
	std::atomic<uint32_t> m_ReserveWritePos;
	std::atomic<uint32_t> m_ReserveReadPos;
	std::atomic<uint32_t> m_WritePos;
	std::atomic<uint32_t> m_ReadPos;
};
/*! @} */
#endif

