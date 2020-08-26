#pragma once

#include "fb/lang/thread/ScopedRefDecl.h"

FB_DECLARE0(Mutex);

FB_PACKAGE0()

#ifndef FB_DATAGUARD_DEBUGGING
#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_DATAGUARD_DEBUGGING FB_TRUE
#else
	#define FB_DATAGUARD_DEBUGGING FB_FALSE
#endif
#endif

#ifndef FB_DATAGUARD_PRINTF_ENABLED
#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_DATAGUARD_PRINTF_ENABLED FB_FALSE
#else
	#define FB_DATAGUARD_PRINTF_ENABLED FB_FALSE
#endif
#endif

#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
#define FB_SCOPED_REF_DEBUG_NAME storedDebugNamePtr
#else
#define FB_SCOPED_REF_DEBUG_NAME nullptr
#endif

#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
#define FB_DATAGUARD_PRINTF(...) FB_PRINTF(__VA_ARGS__)
#else
#define FB_DATAGUARD_PRINTF(...) do { } while (false)
#endif

class ScopedRefBase
{
public:
	static void enterMutex(Mutex &mutex, const char *debugName);
	static void leaveMutex(Mutex &mutex, const char *debugName);
};

class ScopedRefMutexHack
{
public:
	template<typename T>
	inline static Mutex &get(ScopedRef<T> &s);

	template<typename T>
	inline static Mutex &get(ScopedRefReadOnly<T> &s);
};

template <typename T>
class ScopedRef
{
public:
	~ScopedRef()
	{
		ScopedRefBase::leaveMutex(mutex, FB_SCOPED_REF_DEBUG_NAME);
	}

	ScopedRef(ScopedRef &&other)
		: t(other.t)
		, mutex(other.mutex)
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
		, storedDebugNamePtr(debugNamePtr)
#endif
	{
		FB_DATAGUARD_PRINTF("ScopedRef - moved\n");
	}
	ScopedRef(const ScopedRef &other)
		: ScopedRef(other.t, other.mutex, "ScopedRef - Copy")
	{
		FB_DATAGUARD_PRINTF("ScopedRef - copied\n");
	}

	T *operator->() { return &t; }
	const T *operator->() const { return &t; }
	T &getRawRef() { return t; }
	const T &getRawRef() const { return t; }
	T copy() const { return t; }
	void operator=(const T &other) { t = other; }

	inline ScopedRef(T &t, Mutex &mutex, const char *debugNamePtr)
		: t(t)
		, mutex(mutex)
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
		, storedDebugNamePtr(debugNamePtr)
#endif
	{
		ScopedRefBase::enterMutex(mutex, debugNamePtr);
	}

	// Steals mutex from other ScopedRef
	template<typename U>
	inline ScopedRef(T &t, ScopedRef<U> &other, const char *debugNamePtr)
		: ScopedRef(t, ScopedRefMutexHack::get(other), debugNamePtr)
	{
	}

private:
	friend class ScopedRefReadOnly<T>;
	friend class ScopedRefMutexHack;

	T &t;
	Mutex &mutex;
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
	const char *storedDebugNamePtr = "No name";
#endif
};

template <typename T>
class ScopedRefReadOnly
{
public:
	~ScopedRefReadOnly()
	{
		ScopedRefBase::leaveMutex(mutex, FB_SCOPED_REF_DEBUG_NAME);
	}

	ScopedRefReadOnly(ScopedRefReadOnly &&other)
		: t(other.t)
		, mutex(other.mutex)
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
		, storedDebugNamePtr(debugNamePtr)
#endif
	{
		FB_DATAGUARD_PRINTF("ScopedRef - moved\n");
	}
	ScopedRefReadOnly(const ScopedRefReadOnly &other)
		: ScopedRefReadOnly(other.t, other.mutex, "ScopedRef - Copy")
	{
		FB_DATAGUARD_PRINTF("ScopedRef - copied\n");
	}

	ScopedRefReadOnly(const ScopedRef<T> &other)
		: ScopedRefReadOnly(other.t, other.mutex, "ScopedRef - Copy")
	{
	}

	const T *operator->() const { return &t; }
	const T &getRawRef() const { return t; }
	T copy() const { return t; }

	inline ScopedRefReadOnly(const T &t, Mutex &mutex, const char *debugNamePtr)
		: t(t)
		, mutex(mutex)
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
		, storedDebugNamePtr(debugNamePtr)
#endif
	{
		ScopedRefBase::enterMutex(mutex, debugNamePtr);
	}

	// Steals mutex from other ScopedRef
	template<typename U>
	inline ScopedRefReadOnly(T &t, ScopedRef<U> &other, const char *debugNamePtr)
		: ScopedRefReadOnly(t, ScopedRefMutexHack::get(other), debugNamePtr)
	{
	}

	// Steals mutex from other ScopedRefReadOnly
	template<typename U>
	inline ScopedRefReadOnly(T &t, ScopedRefReadOnly<U> &other, const char *debugNamePtr)
		: ScopedRefReadOnly(t, ScopedRefMutexHack::get(other), debugNamePtr)
	{
	}

private:
	friend class ScopedRef<T>;

	const T &t;
	Mutex &mutex;
#if FB_DATAGUARD_PRINTF_ENABLED == FB_TRUE
	const char *storedDebugNamePtr = "No name";
#endif
};

template<typename T>
Mutex &ScopedRefMutexHack::get(ScopedRef<T> &s)
{
	return s.mutex;
}

template<typename T>
Mutex &ScopedRefMutexHack::get(ScopedRefReadOnly<T> &s)
{
	return s.mutex;
}

FB_END_PACKAGE0()
