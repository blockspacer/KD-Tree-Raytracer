#pragma once

#include "fb/lang/Config.h"
#include "FBAssert.h"

FB_PACKAGE1(lang)

/**
 * Simple scope guard for automagically calling delete[]. Supports manual cancellation of
 * the deletion by using dismiss().
 */

class DefaultScopedArrayDeleter;

FB_END_PACKAGE1()

FB_PACKAGE0()

template<typename T, typename D = lang::DefaultScopedArrayDeleter>
class ScopedArray
{
public:
	explicit ScopedArray(T *pointer = 0);
	~ScopedArray();

	/// Cancel deletion of the pointer
	void dismiss();
	/// Delete pointer and optionally replace it with a new one
	void reset(T *pointer = 0);

	T *get() const;

	// Operators
	bool operator !() const;
	T *operator -> () const;
	T &operator*() const { fb_assert(pointer); return *pointer; }
	bool operator==(const T *ptr) const { return pointer == ptr; }
	bool operator!=(const T *ptr) const { return pointer != ptr; }
	bool operator==(const ScopedArray &other) const { return pointer == other.pointer; }
	bool operator!=(const ScopedArray &other) const { return pointer != other.pointer; }

	const T &operator[](SizeType i) const { return pointer[i]; }
	T &operator[](SizeType i) { return pointer[i]; }

	void swap(ScopedArray &other)
	{
		lang::swap(other.pointer, pointer);
	}

	T *release()
	{
		T *ptr = pointer;
		pointer = 0;
		return ptr;
	}

	// implicit conversion to bool
	typedef T * ScopedArray<T>::*OperatorBoolHack;
	operator OperatorBoolHack() const
	{
		return pointer == 0? 0 : &ScopedArray<T>::pointer;
	}

private:
	// Private and not implemented to prevent problems
	ScopedArray(const ScopedArray &);
	void operator = (const ScopedArray &);

	T *pointer;
};

#include "ScopedArrayInline.h"

FB_END_PACKAGE0()
