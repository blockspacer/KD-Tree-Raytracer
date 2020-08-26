#pragma once

#include "fb/lang/Config.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/Atomics.h"
#include "FBSingleThreadAssert.h"
#include "FBStaticAssert.h"
#include "FBAssert.h"

FB_PACKAGE1(lang)

class DefaultSharedPointerDeleter;
class SharedPointerImpl;

// Collect to base everything that doesn't actually require the type
class SharedPointerBaseImpl
{
public:
	SharedPointerBaseImpl(): rawPointer(0), impl(0) {}

	bool isUnique() const;
	bool operator!() const { return rawPointer == 0; }

	void reset();
	SharedPointerImpl *getImpl() const { return impl; }

	// implicit conversion to bool
	typedef void *SharedPointerBaseImpl::*OperatorBoolHack;
	operator OperatorBoolHack() const
	{
		return rawPointer == 0 ? 0 : &SharedPointerBaseImpl::rawPointer;
	}

protected:
	void increaseReferenceCount();
	void decreaseReferenceCount();
	void swap(SharedPointerBaseImpl &other);

	void *rawPointer;
	SharedPointerImpl *impl;
};

FB_END_PACKAGE1()

FB_PACKAGE0()

template<class T>
class SharedPointer: public lang::SharedPointerBaseImpl
{
public:
	SharedPointer();
	explicit SharedPointer(T *ptr);
	template<class D> explicit SharedPointer(T *ptr, const D &d);
	SharedPointer(const SharedPointer &other);
	SharedPointer(SharedPointer &&other) { swap(other); }
	template<class T2> SharedPointer(const SharedPointer<T2> &other);
	~SharedPointer();

	SharedPointer &operator=(const SharedPointer &other);
	SharedPointer &operator=(SharedPointer &&other) { swap(other); return *this; }
	template<class T2> SharedPointer &operator=(const SharedPointer<T2> &other);

	using lang::SharedPointerBaseImpl::reset;
	void reset(T *ptr);
	template<class D> void reset(T *ptr, D d);

	T *get() const { return (T*) rawPointer; }

	bool operator==(const T *ptr) const { return rawPointer == ptr; }
	bool operator!=(const T *ptr) const { return rawPointer != ptr; }
	T *operator->() const { fb_assert(rawPointer); return (T*) rawPointer; }
	T &operator*() const { fb_assert(rawPointer); return * ((T*) rawPointer); }
	bool operator==(const SharedPointer &other) const { return rawPointer == other.rawPointer; }
	bool operator!=(const SharedPointer &other) const { return rawPointer != other.rawPointer; }

	// Need this to keep type safety
	void swap(SharedPointer &other) { SharedPointerBaseImpl::swap(other); }

protected:
	// Don't store any state here
};

FB_END_PACKAGE0()

#include "SharedPointerInline.h"
