#pragma once

#include "fb/lang/Atomics.h"
#include "fb/lang/FBAssert.h"
#include "fb/lang/Types.h"

FB_PACKAGE1(lang)

class RefCountPointerBase
{
protected:
	void *pointer = nullptr;
};

// Member variable for atomic reference count
class AtomicRefCountMember
{
public:
	AtomicRefCountMember()
	{
	}

	~AtomicRefCountMember()
	{
		fb_assert(lang::atomicLoadRelaxed(value) == 0);
	}

	AtomicRefCountMember(const AtomicRefCountMember &)
	{
		// copying classes should not copy reference count!
	}

	void operator=(const AtomicRefCountMember &)
	{
		// copying classes should not copy reference count!
	}

	int get()
	{
		return lang::atomicLoadRelaxed(value);
	}

	void increase()
	{
		lang::atomicIncRelaxed(value);
	}

	bool decrease()
	{
		// release/acquire semantics based on boost
		const int oldRefCount = lang::atomicDecRelease(value);
		fb_assert(oldRefCount >= 1);
		const bool isZero = (oldRefCount == 1);
		if (isZero)
		{
			lang::atomicThreadFenceAcquire();
		}
		return isZero;
	}

	void reset()
	{
		lang::atomicStoreRelease(value, 0);
		lang::atomicThreadFenceAcquire();
	}

private:
	lang::AtomicInt32 value;
};

// Member variable for single threaded reference count
class SingleThreadedRefCountMember
{
public:
	SingleThreadedRefCountMember()
	{
	}

	~SingleThreadedRefCountMember()
	{
		fb_assert(value == 0);
	}

	SingleThreadedRefCountMember(const SingleThreadedRefCountMember &)
	{
		// copying classes should not copy reference count!
	}

	void operator=(const SingleThreadedRefCountMember &)
	{
		// copying classes should not copy reference count!
	}

	int get() const
	{
		return value;
	}

	void increase()
	{
		value++;
	}

	bool decrease()
	{
		const int oldRefCount = value--;
		fb_assert(oldRefCount >= 1);
		const bool isZero = (oldRefCount == 1);
		return isZero;
	}

private:
	int32_t value = 0;
};

template<class T>
struct DefaultReferrer
{
	static void addReference(T *pointer)
	{
		pointer->addReference();
	}
	static void removeReference(T *pointer)
	{
		pointer->removeReference();
	}
};

FB_END_PACKAGE1()

FB_PACKAGE0()

// Pointer to an object that uses reference counting with addReference/removeReference
template<class T, typename Referrer = lang::DefaultReferrer<T>>
class RefCountPointer : public lang::RefCountPointerBase
{
public:
	RefCountPointer()
	{
	}

	RefCountPointer(const RefCountPointer &other)
	{
		pointer = other.pointer;
		if (pointer)
			Referrer::addReference((T*)pointer);
	}
	
	RefCountPointer(RefCountPointer &&other)
	{
		pointer = nullptr;
		swap(other);
	}

	explicit RefCountPointer(T *pointer_)
	{
		pointer = pointer_;
		if (pointer)
			Referrer::addReference((T*)pointer);
	}

	~RefCountPointer()
	{
		if (pointer)
			Referrer::removeReference((T*)pointer);
	}

	RefCountPointer(nullptr_t ptr)
	{
		pointer = nullptr;
	}

	RefCountPointer &operator=(nullptr_t ptr)
	{
		if (pointer)
			Referrer::removeReference((T*)pointer);

		pointer = nullptr;
		return *this;
	}

	RefCountPointer &operator=(const RefCountPointer &other)
	{
		if (other.pointer)
			Referrer::addReference((T*)other.pointer);

		if (pointer)
			Referrer::removeReference((T*)pointer);

		pointer = other.pointer;
		return *this;
	}

	void reset(T *pointer_)
	{
		if (pointer)
			Referrer::removeReference((T*)pointer);

		pointer = pointer_;

		if (pointer)
			Referrer::addReference((T*)pointer);
	}

	void reset()
	{
		if (pointer)
			Referrer::removeReference((T*)pointer);

		pointer = nullptr;
	}
	
	void operator=(RefCountPointer &&other) { swap(other); }
	void swap(RefCountPointer &other)
	{
		lang::swap(other.pointer, pointer);
	}

	bool operator !() const { return !pointer; }
	bool operator==(const T *ptr) const { return pointer == ptr; }
	bool operator!=(const T *ptr) const { return pointer != ptr; }
	bool operator==(const RefCountPointer &other) const { return pointer == other.pointer; }
	bool operator!=(const RefCountPointer &other) const { return pointer != other.pointer; }

	T *operator->() const { return (T*)pointer; }
	T &operator*() const { return *(T*)pointer; }
	
	// implicit conversion to bool
	typedef void * RefCountPointer::*OperatorBoolHack;
	operator OperatorBoolHack() const
	{
		return pointer == 0 ? 0 : &RefCountPointer::pointer;
	}
	
	T *get() const { return (T*)pointer; }
};


// Object for which ownership is handled by reference count (thread safe)
class RefCountedObject
{
public:
	virtual ~RefCountedObject() {}

	void addReference()
	{
		refCount.increase();
	}

	void removeReference()
	{
		if (refCount.decrease())
			delete this;
	}

	int getReferenceCount()
	{
		return refCount.get();
	}
	
private:
	lang::AtomicRefCountMember refCount;
};


// Object for which ownership is handled by reference count (thread safe) (no virtual table)
#define FB_REF_COUNTED_OBJECT() \
	public: \
		void addReference() { refCount.increase(); } \
		void removeReference() { if (refCount.decrease()) delete this; } \
		int getReferenceCount() { return refCount.get(); } \
	private: \
		lang::AtomicRefCountMember refCount; \


// Object for which ownership is handled by reference count from a single thread
class SingleThreadedRefCountedObject
{
public:
	virtual ~SingleThreadedRefCountedObject() {}

	void addReference()
	{
		refCount.increase();
	}

	void removeReference()
	{
		if (refCount.decrease())
			delete this;
	}

	int getReferenceCount() const
	{
		return refCount.get();
	}
	
private:
	lang::SingleThreadedRefCountMember refCount;
};

FB_END_PACKAGE0()
