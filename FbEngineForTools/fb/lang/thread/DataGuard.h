#pragma once

#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/lang/thread/ScopedRef.h"
#include "fb/lang/Types.h"

#ifndef FB_DATAGUARD_DEBUGGING
	#if FB_BUILD != FB_FINAL_RELEASE
		#define FB_DATAGUARD_DEBUGGING FB_TRUE
	#else
		#define FB_DATAGUARD_DEBUGGING FB_FALSE
	#endif
#endif

#if FB_DATAGUARD_DEBUGGING == FB_TRUE
	#include "fb/lang/Atomics.h"
#endif

FB_PACKAGE0()

#ifndef FB_DATAGUARD_PRINTF_ENABLED
#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_DATAGUARD_PRINTF_ENABLED FB_FALSE
#else
	#define FB_DATAGUARD_PRINTF_ENABLED FB_FALSE
#endif
#endif

#ifndef FB_DATAGUARD_PRINTF
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
	#define FB_DATAGUARD_PRINTF(...) FB_PRINTF(__VA_ARGS__)
#else
	#define FB_DATAGUARD_PRINTF(...) do { } while (false)
#endif
#endif

class DataGuardBase
{
protected:
#if FB_DATAGUARD_DEBUGGING == FB_TRUE
	void setUsageLoggingInterval(SizeType interval) { usageLoggingInterval = interval; }
	void setDebugName(const char *nameParam, const Mutex *mutexPtr);
	void checkHeap();
	const char *getDebugName() const { return name; }
private:
	mutable lang::AtomicUInt32 usageCounter;
	SizeType usageLoggingInterval = 1024U;
	const char *name = "DataGuard";
#else
	FB_FORCEINLINE void setUsageLoggingInterval(SizeType interval) {}
	FB_FORCEINLINE void setDebugName(const char *, const Mutex *) {}
	FB_FORCEINLINE void checkHeap() {}
	FB_FORCEINLINE const char *getDebugName() const { return "DataGuard"; }
#endif
};

template <typename T>
class DataGuard : DataGuardBase
{
public:
	DataGuard(const char *debugName = "Unnamed DataGuard")
	{
		setDebugName(debugName, &mutex);
		checkHeap();
	}
	DataGuard(const T &t, const char *debugName = "Unnamed DataGuard")
		: t(t)
	{
		setDebugName(debugName, &mutex);
		checkHeap();
	}

	ScopedRef<T> get()
	{
		return ScopedRef<T>(t, mutex, getDebugName());
	}
	ScopedRefReadOnly<T> get() const
	{
		return ScopedRefReadOnly<T>(t, mutex, getDebugName());
	}

	T copy() const { return get().copy(); }
	operator ScopedRef<T>() { return get(); }
	operator ScopedRefReadOnly<T>() const { return get(); }
	ScopedRef<T> operator->() { return get(); }
	ScopedRefReadOnly<T> operator->() const { return get(); }
	void operator=(const T &other) { get() = other; }

private:
	mutable Mutex mutex;
	T t;
};

template <typename T>
class DataGuardSharedMutex : DataGuardBase
{
public:
	DataGuardSharedMutex(Mutex &mutex, const char *debugName = "Unnamed DataGuardSharedMutex")
		: mutex(mutex)
	{
		setDebugName(debugName, &mutex);
	}
	DataGuardSharedMutex(const T &t, Mutex &mutex, const char *debugName = "Unnamed DataGuardSharedMutex")
		: t(t)
		, mutex(mutex)
	{
		setDebugName(debugName, &mutex);
	}

	ScopedRef<T> get()
	{
		return ScopedRef<T>(t, mutex, getDebugName());
	}
	ScopedRefReadOnly<T> get() const
	{
		return ScopedRefReadOnly<T>(t, mutex, getDebugName());
	}

	T copy() const { return get().copy(); }
	operator ScopedRef<T>() { return get(); }
	operator ScopedRefReadOnly<T>() const { return get(); }
	ScopedRef<T> operator->() { return get(); }
	ScopedRefReadOnly<T> operator->() const { return get(); }
	void operator=(const T &other) { get() = other; }

private:
	Mutex &mutex;
	T t;
};

template <typename T>
class DataGuardPtr : DataGuardBase
{
public:
	DataGuardPtr(const char *debugName = "Unnamed DataGuard")
	{
		setDebugName(debugName, &mutex);
	}
	DataGuardPtr(T *t, const char *debugName = "Unnamed DataGuard")
		: t(t)
	{
		setDebugName(debugName, &mutex);
	}

	ScopedRef<T> get()
	{
		fb_assert(t);
		return ScopedRef<T>(*t, mutex, getDebugName());
	}
	ScopedRefReadOnly<T> get() const
	{
		fb_assert(t);
		return ScopedRefReadOnly<T>(*t, mutex, getDebugName());
	}

	T copy() const { return get().copy(); }
	operator ScopedRef<T>() { return get(); }
	operator ScopedRefReadOnly<T>() const { return get(); }
	ScopedRef<T> operator->() { return get(); }
	ScopedRefReadOnly<T> operator->() const { return get(); }
	void operator=(const T &other) { get() = other; }

	void deleteObject()
	{
		if (t)
		{
			delete t;
			t = nullptr;
		}
	}

private:
	mutable Mutex mutex;
	T *t = nullptr;
};

FB_END_PACKAGE0()
