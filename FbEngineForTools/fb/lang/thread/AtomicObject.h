#pragma once

#include "fb/lang/thread/Mutex.h"

FB_PACKAGE0()

/**
 * Note, since this makes an internal copy of the object and returns the objects as a copy rather than 
 * a reference it is not well suited for complex objects.
 */
template<class T>
class AtomicIncrementableObject
{
public:
	AtomicIncrementableObject(const T &obj)
		: obj(obj)
	{		
		fb_single_thread_assert();
	}

	~AtomicIncrementableObject()
	{
		fb_single_thread_assert();
	}

	operator T() const
	{
		mutex.enter();
		T ret = obj;
		mutex.leave();
		return ret;
	}

	// Note, this returns a copy of the object, no a reference. (because it would not be safe for atomic access)
	// TODO: this could be specialized to use real atomic assignment for primitive types where platform supports it
	T operator= (const T &value)
	{
		mutex.enter();
		obj = value;
		T ret = obj;
		mutex.leave();
		return ret;
	}

	// always remember that this is a postfix operation, you'll get the previous value as return value
	// TODO: this could be specialized to use real atomic increment for primitive types where platform supports it
	int operator++(int)
	{
		mutex.enter();
		T ret = obj;
		obj++;
		mutex.leave();
		return ret;
	}

	// always remember that this is a postfix operation, you'll get the previous value as return value
	// TODO: this could be specialized to use real atomic decrement for primitive types where platform supports it
	int operator--(int)
	{
		mutex.enter();
		T ret = obj;
		obj--;
		mutex.leave();
		return ret;
	}

protected:
	T obj;
	mutable Mutex mutex;

private:
	// for added safety, you are assumed to always provide a default value in the constructor.
	// it will make things a bit harder if you are trying to use this inside some container, etc. but then again,
	// you should be aware of the possibly high additional load that comes with this... So using this with a 
	// generic container might be a bad idea anyway.
	AtomicIncrementableObject();

	// NOTE: prefix is not supported, as that would be expected to return a reference.
	// and a reference would be a problem with concurrent access - the referenced value might get changed by
	// another thread, thus, trashing the entire point of an atomic object.
	const T &operator++();
	const T &operator--();
};

FB_END_PACKAGE0()
