#pragma once

#include "fb/lang/Config.h"
#include "fb/lang/FBAssert.h"
#include "fb/lang/Swap.h"

FB_PACKAGE1(lang)

/**
 * Simple scope guard for automagically calling delete. Supports manual cancellation of
 * the deletion by using dismiss().
 */

class DefaultScopedPointerDeleter;

class ScopedPointerBaseImp
{
public:
	ScopedPointerBaseImp()
	:	pointer(nullptr)
	{
	}

protected:
	void *pointer;
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<typename T, typename D = lang::DefaultScopedPointerDeleter>
class ScopedPointer: public lang::ScopedPointerBaseImp
{
public:
	explicit ScopedPointer(T *pointer = 0);
	ScopedPointer(ScopedPointer &&other) { swap(other); }
	~ScopedPointer();

	/// Cancel deletion of the pointer
	void dismiss();
	/// Delete pointer and optionally replace it with a new one
	void reset(T *pointer = 0);

	inline T *get() const;

	// Operators
	inline bool operator !() const;
	inline T *operator -> () const;
	T &operator*() const { fb_assert(pointer); return *(T*)pointer; }
	bool operator==(const T *ptr) const { return pointer == ptr; }
	bool operator!=(const T *ptr) const { return pointer != ptr; }
	bool operator==(const ScopedPointer &other) const { return pointer == other.pointer; }
	bool operator!=(const ScopedPointer &other) const { return pointer != other.pointer; }
	
	void operator=(ScopedPointer &&other) { swap(other); }
	void swap(ScopedPointer &other)
	{
		lang::swap(other.pointer, pointer);
	}

	T *release()
	{
		T *ptr = (T*) pointer;
		pointer = 0;
		return ptr;
	}

	// implicit conversion to bool
	typedef void * ScopedPointer<T>::*OperatorBoolHack;
	operator OperatorBoolHack() const
	{
		return pointer == 0 ? 0 : &ScopedPointer<T>::pointer;
	}

private:
	// Private and not implemented to prevent problems
	ScopedPointer(const ScopedPointer &);
	void operator = (const ScopedPointer &);

	//T *pointer;
};

#include "ScopedPointerInline.h"

FB_END_PACKAGE0()
