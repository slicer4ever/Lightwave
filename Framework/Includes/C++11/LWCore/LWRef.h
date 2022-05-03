#ifndef LWREF_H
#define LWREF_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWAllocator.h>
#include <atomic>
#include <functional>

/*!< \brief reference memory deleter. */
typedef void (*LWRefDeleteFunc)(void*);

/*!< \brief Reference's counter and delete function storage. */
struct LWRef_Counter {
	std::atomic<uint32_t> UseCount = 1;
	LWRefDeleteFunc DeleteFunc;

	template<class Type>
	static void Destroy(void *Mem) {
		((Type*)Mem)->~Type();
	};

	/*!< \brief add's a reference to the atomic counter, returns previous reference count. */
	uint32_t AddRef();

	/*!< \brief remove's a reference to the atomic counter, asserts that is not over de-referencing(UseCount is not already 0), returns previous use count, if previous use count was 1, then data was deleted. */
	uint32_t RemoveRef();

	LWRef_Counter(LWRefDeleteFunc DeleteFunc);
};

/*!< \brief LWRef is an automatic atomic reference counted type that allows for seamless shared reference, objects created will automatically be deleted by the correct deallocator, even if they have been transformed to a base type from a derived type. LWRef object's are constructed in a single allocation. */
template<class Type>
class LWRef {
public:
	
	/*!< \brief get's the contained value, a check to IsValid should be made first to ensure dereferencing does not cause a crash. */
	const Type &operator*() const {
		assert(m_Value != nullptr);
		return *m_Value;
	}

	/*!< \brief get's the contained value, a check to IsValid should be made first to ensure dereferencing does not cause a crash. */
	Type &operator*() {
		assert(m_Value != nullptr);
		return *m_Value;
	}

	/*!< \brief get's the contained value as a pointer. */
	const Type *operator->() const {
		assert(m_Value != nullptr);
		return m_Value;
	}

	/*!< \brief get's the contained value as a pointer. */
	Type *operator->() {
		assert(m_Value!=nullptr);
		return m_Value;
	}

	/*!< \brief get's the contained value as a pointer. */
	const Type *Get(void) const {
		assert(m_Value!=nullptr);
		return m_Value;
	}

	/*!< \brief get's the contained value as a pointer. */
	Type *Get(void) {
		assert(m_Value!=nullptr);
		return m_Value;
	}

	/*!< \brief move operator from a derived class of Type, if derived is not a child of Type then an error will be thrown. */
	template<class Derived>
	LWRef &operator = (LWRef<Derived> &&Other) noexcept {
		if(m_Counter==Other.m_Counter) return *this; //Exchanging with self, so don't do anything.
		Release();
		m_Counter = std::exchange(Other.m_Counter, nullptr);
		m_Value = std::exchange(Other.m_Value, nullptr);
		return *this;
	}

	/*!< \brief copy operator from a derived class of Type, if derived is not a child of Type, than an error will be thrown. */
	template<class Derived>
	LWRef &operator = (const LWRef<Derived> &Other) noexcept {
		if(m_Counter==Other.m_Counter) return *this; //Exchanging with self, so don't do anything.
		Release();
		m_Counter = Other.m_Counter;
		m_Value = Other.m_Value;
		if (m_Counter) m_Counter->AddRef();
		return *this;
	}

	/*!< \brief move operator from another reference object, does not incrment the reference count, and other will be set to a null state. */
	LWRef &operator = (LWRef<Type> &&Other) noexcept {
		if(m_Counter==Other.m_Counter) return *this; //Exchanging with self, so don't do anything.
		Release();
		m_Counter = std::exchange(Other.m_Counter, nullptr);
		m_Value = std::exchange(Other.m_Value, nullptr);
		return *this;
	}

	/*!< \brief copy operator from another reference object, does increment the reference count. */
	LWRef &operator = (const LWRef<Type> &Other) noexcept {
		if(m_Counter==Other.m_Counter) return *this; //Exchanging with self, so don't do anything.
		Release();
		m_Counter = Other.m_Counter;
		m_Value = Other.m_Value;
		if (m_Counter) m_Counter->AddRef();
		return *this;
	}

	/*!< \brief call's Type's move operator, and move's Other into m_Value */
	LWRef &operator = (Type &&Other) noexcept {
		if (!m_Counter) return *this;
		*m_Value = std::move(Other);
		return *this;
	}

	/*!< \brief call's Type's copy operator, and copy's Other into m_Value. */
	LWRef &operator = (const Type &Other) noexcept {
		if (!m_Counter) return *this;
		*m_Value = Other;
		return *this;
	}

	bool operator == (const LWRef<Type> &Other) const {
		return m_Counter==Other.m_Counter;
	}

	bool operator != (const LWRef<Type> &Other) const {
		return m_Counter!=Other.m_Counter;
	}

	explicit operator bool(void) const {
		return m_Counter!=nullptr;
	}

	/*!< \brief returns the current usecount for this reference, or 0 if not a valid object. */
	uint32_t UseCount(void) const {
		return m_Counter ? m_Counter->UseCount.load() : 0;
	}

	/*!< \brief releases this reference, and set's the object into a null state. */
	void Release(void) {
		LWRef_Counter *C = std::exchange(m_Counter, nullptr);
		m_Value = nullptr;
		if(C) C->RemoveRef();
		return;
	}

	/*!< \brief returns if this reference contains valid data or not. */
	bool IsValid(void) const {
		return m_Counter != nullptr;
	}

	/*!< \brief dervied reference type move construct. */
	template<class Derived>
	LWRef(LWRef<Derived> &&Other) noexcept {
		*this = std::move(Other);
	}

	/*!< \brief dervied reference type copy construct. */
	template<class Derived>
	LWRef(const LWRef<Derived> &Other) noexcept {
		*this = Other;
	}

	/*!< \brief reference type move construct. */
	LWRef(LWRef<Type> &&Other) noexcept {
		*this = std::move(Other);
	}

	/*!< \brief reference type Copy construct. */
	LWRef(const LWRef<Type> &Other) noexcept {
		*this = Other;
	}

	LWRef() = default;

	~LWRef() {
		Release();
	}

	template<class Derivied>
	friend class LWRef;

	friend class LWAllocator;

private:
	LWRef(LWRef_Counter *Counter, Type *Value) : m_Counter(Counter), m_Value(Value) {}

	LWRef_Counter *m_Counter = nullptr;
	Type *m_Value = nullptr;
};
#endif