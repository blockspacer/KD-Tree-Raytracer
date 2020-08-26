#ifndef FB_LANG_SHAREDARRAY_H
#define FB_LANG_SHAREDARRAY_H

#include "SharedPointer.h"

FB_PACKAGE1(lang)

class DefaultSharedArrayDeleter
{
public:
	template<class T>
	void operator()(T *pointer)
	{
		delete[] pointer;
	}
};

template<class T>
class SharedArray : public SharedPointer<T>
{
	typedef SharedPointer<T> BaseClass;
public:

	SharedArray()
	{
	}

	explicit SharedArray(T *ptr)
	{
		BaseClass::reset(ptr, DefaultSharedArrayDeleter());
	}

	template<class D> explicit SharedArray(T *ptr, const D &d)
	{
		BaseClass::reset(ptr, d);
	}

	SharedArray(const SharedArray &other)
	{
		(*this) = other;
	}

	void reset()
	{
		BaseClass::reset();
	}

	void reset(T *ptr)
	{
		BaseClass::reset(ptr, DefaultSharedArrayDeleter());
	}

	template<class D> void reset(T *ptr, const D &d)
	{
		BaseClass::reset(ptr, d);
	}

	SharedArray &operator=(const SharedArray &other)
	{
		*(BaseClass*)this = *(BaseClass*)&other;
		return *this;
	}

	T &operator[](SizeType i) const
	{
		T *ptr = (T*) BaseClass::rawPointer;
		fb_assert(ptr);
		return ptr[i];
	}
};

FB_END_PACKAGE1()

#endif
