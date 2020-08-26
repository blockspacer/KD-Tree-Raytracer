#pragma once

#include "fb/lang/platform/Compiler.h"

// Default implementation is with C++11 atomics.
// Might have to do this manually for some platforms after looking at what code is being generated ..

#if FB_VS2017_IN_USE == FB_TRUE
#pragma warning(push)
/* 'initializing': conversion from 'XXX' to 'YYY', signed/unsigned mismatch */
#pragma warning( disable: 4365 )
/* expression before comma has no effect; expected expression with side-effect */
#pragma warning( disable: 4548 )
#endif

#include <atomic>
#include <intrin.h>

#if FB_VS2017_IN_USE == FB_TRUE
#pragma warning(pop)
#endif

#if ATOMIC_INT_LOCK_FREE != 2
	#error "Implement this manually.";
#endif
#if (ATOMIC_POINTER_LOCK_FREE != 2) || (ATOMIC_LLONG_LOCK_FREE != 2)
	#error "Implement this manually.";
#endif

FB_PACKAGE1(lang)

// Add/sub/inc/dec etc return value of atomic variable _before_ modification

// Fences

static inline void atomicThreadFenceAcquire() { std::atomic_thread_fence(std::memory_order_acquire); }
static inline void atomicThreadFenceRelease() { std::atomic_thread_fence(std::memory_order_release); }
// For testing. Otherwise you are probably doing something wrong.
static inline void atomicThreadFenceSeqCst() { std::atomic_thread_fence(std::memory_order_seq_cst); }

// Utils
extern void atomicThreadPause();

// -----
// Int32
// -----
typedef std::atomic<int32_t> AtomicInt32Imp;
struct AtomicInt32
{
	AtomicInt32() : imp(0) { }
	AtomicInt32Imp imp;
};

// Relaxed
static inline int32_t atomicLoadRelaxed(const AtomicInt32 &a) { return a.imp.load(std::memory_order_relaxed); }
static inline void atomicStoreRelaxed(AtomicInt32 &a, int32_t value) { a.imp.store(value, std::memory_order_relaxed); }
static inline int32_t atomicAddRelaxed(AtomicInt32 &a, int32_t value) { return a.imp.fetch_add(value, std::memory_order_relaxed); }
static inline int32_t atomicSubRelaxed(AtomicInt32 &a, int32_t value) { return a.imp.fetch_sub(value, std::memory_order_relaxed); }
static inline int32_t atomicIncRelaxed(AtomicInt32 &a) { return atomicAddRelaxed(a, 1); }
static inline int32_t atomicDecRelaxed(AtomicInt32 &a) { return atomicSubRelaxed(a, 1); }
static inline int32_t atomicOrRelaxed(AtomicInt32 &a, int32_t value) { return a.imp.fetch_or(value, std::memory_order_relaxed); }
static inline int32_t atomicAndRelaxed(AtomicInt32 &a, int32_t value) { return a.imp.fetch_and(value, std::memory_order_relaxed); }
// Release
static inline void atomicStoreRelease(AtomicInt32 &a, int32_t value) { a.imp.store(value, std::memory_order_release); }
static inline int32_t atomicAddRelease(AtomicInt32 &a, int32_t value) { return a.imp.fetch_add(value, std::memory_order_release); }
static inline int32_t atomicSubRelease(AtomicInt32 &a, int32_t value) { return a.imp.fetch_sub(value, std::memory_order_release); }
static inline int32_t atomicIncRelease(AtomicInt32 &a) { return atomicAddRelease(a, 1); }
static inline int32_t atomicDecRelease(AtomicInt32 &a) { return atomicSubRelease(a, 1); }
// Acquire
static inline int32_t atomicLoadAcquire(const AtomicInt32 &a) { return a.imp.load(std::memory_order_acquire); }
static inline int32_t atomicAddAcquire(AtomicInt32 &a, int32_t value) { return a.imp.fetch_add(value, std::memory_order_acquire); }
static inline int32_t atomicSubAcquire(AtomicInt32 &a, int32_t value) { return a.imp.fetch_sub(value, std::memory_order_acquire); }
static inline int32_t atomicIncAcquire(AtomicInt32 &a) { return atomicAddAcquire(a, 1); }
static inline int32_t atomicDecAcquire(AtomicInt32 &a) { return atomicSubAcquire(a, 1); }
static inline int32_t atomicOrAcquire(AtomicInt32 &a, int32_t value) { return a.imp.fetch_or(value, std::memory_order_acquire); }
// Acquire/release
static inline int32_t atomicAddAcquireRelease(AtomicInt32 &a, int32_t value) { return a.imp.fetch_add(value, std::memory_order_acq_rel); }
static inline int32_t atomicSubAcquireRelease(AtomicInt32 &a, int32_t value) { return a.imp.fetch_sub(value, std::memory_order_acq_rel); }
static inline int32_t atomicIncAcquireRelease(AtomicInt32 &a) { return atomicAddAcquireRelease(a, 1); }
static inline int32_t atomicDecAcquireRelease(AtomicInt32 &a) { return atomicSubAcquireRelease(a, 1); }
// CAS
static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicInt32 &a, int32_t &expectedValue, int32_t newValue) { return a.imp.compare_exchange_weak(expectedValue, newValue, std::memory_order_acq_rel); }
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicInt32 &a, int32_t &expectedValue, int32_t newValue) { return a.imp.compare_exchange_strong(expectedValue, newValue, std::memory_order_acq_rel); }

// -----
// UInt32
// -----
typedef std::atomic<uint32_t> AtomicUInt32Imp;
struct AtomicUInt32
{
	AtomicUInt32() : imp(0) { }
	AtomicUInt32Imp imp;
};

// Relaxed
static inline uint32_t atomicLoadRelaxed(const AtomicUInt32 &a) { return a.imp.load(std::memory_order_relaxed); }
static inline void atomicStoreRelaxed(AtomicUInt32 &a, uint32_t value) { a.imp.store(value, std::memory_order_relaxed); }
static inline uint32_t atomicAddRelaxed(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_add(value, std::memory_order_relaxed); }
static inline uint32_t atomicSubRelaxed(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_sub(value, std::memory_order_relaxed); }
static inline uint32_t atomicIncRelaxed(AtomicUInt32 &a) { return atomicAddRelaxed(a, 1); }
static inline uint32_t atomicDecRelaxed(AtomicUInt32 &a) { return atomicSubRelaxed(a, 1); }
static inline uint32_t atomicOrRelaxed(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_or(value, std::memory_order_relaxed); }
static inline uint32_t atomicAndRelaxed(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_and(value, std::memory_order_relaxed); }
// Release
static inline void atomicStoreRelease(AtomicUInt32 &a, uint32_t value) { a.imp.store(value, std::memory_order_release); }
static inline uint32_t atomicAddRelease(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_add(value, std::memory_order_release); }
static inline uint32_t atomicSubRelease(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_sub(value, std::memory_order_release); }
static inline uint32_t atomicIncRelease(AtomicUInt32 &a) { return atomicAddRelease(a, 1); }
static inline uint32_t atomicDecRelease(AtomicUInt32 &a) { return atomicSubRelease(a, 1); }
// Acquire
static inline uint32_t atomicLoadAcquire(const AtomicUInt32 &a) { return a.imp.load(std::memory_order_acquire); }
static inline uint32_t atomicAddAcquire(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_add(value, std::memory_order_acquire); }
static inline uint32_t atomicSubAcquire(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_sub(value, std::memory_order_acquire); }
static inline uint32_t atomicIncAcquire(AtomicUInt32 &a) { return atomicAddAcquire(a, 1); }
static inline uint32_t atomicDecAcquire(AtomicUInt32 &a) { return atomicSubAcquire(a, 1); }
static inline uint32_t atomicOrAcquire(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_or(value, std::memory_order_acquire); }
// Acquire/release
static inline uint32_t atomicAddAcquireRelease(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_add(value, std::memory_order_acq_rel); }
static inline uint32_t atomicSubAcquireRelease(AtomicUInt32 &a, uint32_t value) { return a.imp.fetch_sub(value, std::memory_order_acq_rel); }
static inline uint32_t atomicIncAcquireRelease(AtomicUInt32 &a) { return atomicAddAcquireRelease(a, 1); }
static inline uint32_t atomicDecAcquireRelease(AtomicUInt32 &a) { return atomicSubAcquireRelease(a, 1); }
// CAS
static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicUInt32 &a, uint32_t &expectedValue, uint32_t newValue) { return a.imp.compare_exchange_weak(expectedValue, newValue, std::memory_order_acq_rel); }
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicUInt32 &a, uint32_t &expectedValue, uint32_t newValue) { return a.imp.compare_exchange_strong(expectedValue, newValue, std::memory_order_acq_rel); }

// -----
// Int64
// -----
typedef std::atomic<int64_t> AtomicInt64Imp;
struct AtomicInt64
{
	AtomicInt64() : imp(0) { }
	AtomicInt64Imp imp;
};

// Relaxed
static inline int64_t atomicLoadRelaxed(const AtomicInt64 &a) { return a.imp.load(std::memory_order_relaxed); }
static inline void atomicStoreRelaxed(AtomicInt64 &a, int64_t value) { a.imp.store(value, std::memory_order_relaxed); }
static inline int64_t atomicAddRelaxed(AtomicInt64 &a, int64_t value) { return a.imp.fetch_add(value, std::memory_order_relaxed); }
static inline int64_t atomicSubRelaxed(AtomicInt64 &a, int64_t value) { return a.imp.fetch_sub(value, std::memory_order_relaxed); }
static inline int64_t atomicIncRelaxed(AtomicInt64 &a) { return atomicAddRelaxed(a, 1); }
static inline int64_t atomicDecRelaxed(AtomicInt64 &a) { return atomicSubRelaxed(a, 1); }
static inline int64_t atomicOrRelaxed(AtomicInt64 &a, int64_t value) { return a.imp.fetch_or(value, std::memory_order_relaxed); }
static inline int64_t atomicAndRelaxed(AtomicInt64 &a, int64_t value) { return a.imp.fetch_and(value, std::memory_order_relaxed); }
// Release
static inline void atomicStoreRelease(AtomicInt64 &a, int64_t value) { a.imp.store(value, std::memory_order_release); }
static inline int64_t atomicAddRelease(AtomicInt64 &a, int64_t value) { return a.imp.fetch_add(value, std::memory_order_release); }
static inline int64_t atomicSubRelease(AtomicInt64 &a, int64_t value) { return a.imp.fetch_sub(value, std::memory_order_release); }
static inline int64_t atomicIncRelease(AtomicInt64 &a) { return atomicAddRelease(a, 1); }
static inline int64_t atomicDecRelease(AtomicInt64 &a) { return atomicSubRelease(a, 1); }
// Acquire
static inline int64_t atomicLoadAcquire(const AtomicInt64 &a) { return a.imp.load(std::memory_order_acquire); }
static inline int64_t atomicAddAcquire(AtomicInt64 &a, int64_t value) { return a.imp.fetch_add(value, std::memory_order_acquire); }
static inline int64_t atomicSubAcquire(AtomicInt64 &a, int64_t value) { return a.imp.fetch_sub(value, std::memory_order_acquire); }
static inline int64_t atomicIncAcquire(AtomicInt64 &a) { return atomicAddAcquire(a, 1); }
static inline int64_t atomicDecAcquire(AtomicInt64 &a) { return atomicSubAcquire(a, 1); }
static inline int64_t atomicOrAcquire(AtomicInt64 &a, int64_t value) { return a.imp.fetch_or(value, std::memory_order_acquire); }
// Acquire/release
static inline int64_t atomicAddAcquireRelease(AtomicInt64 &a, int64_t value) { return a.imp.fetch_add(value, std::memory_order_acq_rel); }
static inline int64_t atomicSubAcquireRelease(AtomicInt64 &a, int64_t value) { return a.imp.fetch_sub(value, std::memory_order_acq_rel); }
static inline int64_t atomicIncAcquireRelease(AtomicInt64 &a) { return atomicAddAcquireRelease(a, 1); }
static inline int64_t atomicDecAcquireRelease(AtomicInt64 &a) { return atomicSubAcquireRelease(a, 1); }
// CAS
static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicInt64 &a, int64_t &expectedValue, int64_t newValue) { return a.imp.compare_exchange_weak(expectedValue, newValue, std::memory_order_acq_rel); }
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicInt64 &a, int64_t &expectedValue, int64_t newValue) { return a.imp.compare_exchange_strong(expectedValue, newValue, std::memory_order_acq_rel); }

// -----
// UInt64
// -----
typedef std::atomic<uint64_t> AtomicUInt64Imp;
struct AtomicUInt64
{
	AtomicUInt64() : imp(0) { }
	AtomicUInt64Imp imp;
};

// Relaxed
static inline uint64_t atomicLoadRelaxed(const AtomicUInt64 &a) { return a.imp.load(std::memory_order_relaxed); }
static inline void atomicStoreRelaxed(AtomicUInt64 &a, uint64_t value) { a.imp.store(value, std::memory_order_relaxed); }
static inline uint64_t atomicAddRelaxed(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_add(value, std::memory_order_relaxed); }
static inline uint64_t atomicSubRelaxed(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_sub(value, std::memory_order_relaxed); }
static inline uint64_t atomicIncRelaxed(AtomicUInt64 &a) { return atomicAddRelaxed(a, 1); }
static inline uint64_t atomicDecRelaxed(AtomicUInt64 &a) { return atomicSubRelaxed(a, 1); }
static inline uint64_t atomicOrRelaxed(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_or(value, std::memory_order_relaxed); }
static inline uint64_t atomicAndRelaxed(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_and(value, std::memory_order_relaxed); }
// Release
static inline void atomicStoreRelease(AtomicUInt64 &a, uint64_t value) { a.imp.store(value, std::memory_order_release); }
static inline uint64_t atomicAddRelease(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_add(value, std::memory_order_release); }
static inline uint64_t atomicSubRelease(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_sub(value, std::memory_order_release); }
static inline uint64_t atomicIncRelease(AtomicUInt64 &a) { return atomicAddRelease(a, 1); }
static inline uint64_t atomicDecRelease(AtomicUInt64 &a) { return atomicSubRelease(a, 1); }
// Acquire
static inline uint64_t atomicLoadAcquire(const AtomicUInt64 &a) { return a.imp.load(std::memory_order_acquire); }
static inline uint64_t atomicAddAcquire(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_add(value, std::memory_order_acquire); }
static inline uint64_t atomicSubAcquire(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_sub(value, std::memory_order_acquire); }
static inline uint64_t atomicIncAcquire(AtomicUInt64 &a) { return atomicAddAcquire(a, 1); }
static inline uint64_t atomicDecAcquire(AtomicUInt64 &a) { return atomicSubAcquire(a, 1); }
static inline uint64_t atomicOrAcquire(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_or(value, std::memory_order_acquire); }
// Acquire/release
static inline uint64_t atomicAddAcquireRelease(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_add(value, std::memory_order_acq_rel); }
static inline uint64_t atomicSubAcquireRelease(AtomicUInt64 &a, uint64_t value) { return a.imp.fetch_sub(value, std::memory_order_acq_rel); }
static inline uint64_t atomicIncAcquireRelease(AtomicUInt64 &a) { return atomicAddAcquireRelease(a, 1); }
static inline uint64_t atomicDecAcquireRelease(AtomicUInt64 &a) { return atomicSubAcquireRelease(a, 1); }
// CAS
static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicUInt64 &a, uint64_t &expectedValue, uint64_t newValue) { return a.imp.compare_exchange_weak(expectedValue, newValue, std::memory_order_acq_rel); }
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicUInt64 &a, uint64_t &expectedValue, uint64_t newValue) { return a.imp.compare_exchange_strong(expectedValue, newValue, std::memory_order_acq_rel); }

// -------
// Pointer
// -------
typedef uintptr_t AtomicPointerIntType;
typedef std::atomic<AtomicPointerIntType> AtomicPointerImp;
struct AtomicPointer
{
	AtomicPointer() : imp(0) { }
	AtomicPointerImp imp;
};

// Relaxed
static inline void *atomicLoadRelaxed(const AtomicPointer &a) { return (void*) a.imp.load(std::memory_order_relaxed); }
static inline void atomicStoreRelaxed(AtomicPointer &a, void *value) { a.imp.store((AtomicPointerIntType) value, std::memory_order_relaxed); }
// Release
static inline void atomicStoreRelease(AtomicPointer &a, void *value) { a.imp.store((AtomicPointerIntType) value, std::memory_order_release); }
static inline AtomicPointerIntType atomicAndRelease(AtomicPointer &a, AtomicPointerIntType value) { return a.imp.fetch_and(value, std::memory_order_release); }
// Acquire
static inline void *atomicLoadAcquire(AtomicPointer &a) { return (void*) a.imp.load(std::memory_order_acquire); }
static inline AtomicPointerIntType atomicOrAcquire(AtomicPointer &a, AtomicPointerIntType value) { return a.imp.fetch_or(value, std::memory_order_acquire); }
// CAS
static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicPointer &a, void *&expectedValue, void *newValue) 
{ 
	AtomicPointerIntType expectedValueTmp = (AtomicPointerIntType) expectedValue; 
	bool result = a.imp.compare_exchange_weak(expectedValueTmp, (AtomicPointerIntType) newValue, std::memory_order_acq_rel); 
	expectedValue = (void*) expectedValueTmp;
	return result;
}
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicPointer &a, void *&expectedValue, void *newValue) 
{ 
	AtomicPointerIntType expectedValueTmp = (AtomicPointerIntType) expectedValue; 
	bool result = a.imp.compare_exchange_strong(expectedValueTmp, (AtomicPointerIntType) newValue, std::memory_order_acq_rel); 
	expectedValue = (void*) expectedValueTmp;
	return result;
}

// -----------
// ABA pointer
// -----------
// For generic X64 there are no 128bit wide reliable atomic loads/stores which is pretty awful.
// movdqa only works on new'ish processors.
// Does NOT work on AMD Jaguar (which consoles are based on), but they might have needed modifications, needs testing).
// See https://cbloomrants.blogspot.com/2014/11/11-11-14-x64-movdqa-atomic-test.html
// Wrap cmpxchg16b and have the boilerplate outside platform defines for now.

struct alignas(16) AtomicAbaPointer
{
	__int64 values[2] = { 0 };
	void *getPointer() { return (void*)values[0]; }
	void setPointer(void *ptr) { values[0] = (__int64) ptr; }
	void step() { ++values[1]; }
};

struct alignas(16) AtomicAbaNode
{
	AtomicAbaPointer imp;
};

static inline bool atomicCompareExchangeWeakAcquireRelease(AtomicAbaNode &a, AtomicAbaPointer &expectedValue, AtomicAbaPointer newValue) 
{ 
	return _InterlockedCompareExchange128(a.imp.values, newValue.values[1], newValue.values[0], expectedValue.values) == 1;
}

// Relaxed
static inline AtomicAbaPointer atomicLoadRelaxed(const AtomicAbaNode &a) 
{ 
	// Do a dummy store with identical reference and new value.
	AtomicAbaPointer tmp;
	atomicCompareExchangeWeakAcquireRelease(const_cast<AtomicAbaNode &> (a), tmp, tmp);
	return tmp;
}
// Release
static inline void atomicStoreRelease(AtomicAbaNode &a, AtomicAbaPointer value) 
{ 
	// Non-atomic load. It's just an initial guess.
	AtomicAbaPointer ref = a.imp;
	// Actual store loop.
	while(!atomicCompareExchangeWeakAcquireRelease(a, value, ref))
	{
		atomicThreadPause();
	}
}
// Acquire
static inline AtomicAbaPointer atomicLoadAcquire(const AtomicAbaNode &a) { return atomicLoadRelaxed(a); }
// CAS
static inline bool atomicCompareExchangeStrongAcquireRelease(AtomicAbaNode &a, AtomicAbaPointer &expectedValue, AtomicAbaPointer newValue) 
{ 
	return atomicCompareExchangeWeakAcquireRelease(a, expectedValue, newValue);
}

FB_END_PACKAGE1()
