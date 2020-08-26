#include "Precompiled.h"
#include "StringImp.h"

#include "fb/container/LinearHashSet.h"
#include "fb/container/PodVector.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/Cstrlen.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/GlobalFixedAllocateFunctions.h"
#include "fb/lang/GlobalMemoryOperators.h"
#include "fb/lang/hash/Hash.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/Types.h"
#include "fb/memory/AtomicLinearAllocator.h"
#include "fb/memory/GlobalFixedAllocator.h"
#include "fb/memory/stats/DebugStats.h"
#include "fb/profiling/ScopedProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/Config.h"
#include "fb/string/StringRef.h"

#include <cstring>

#define FB_STRING_REGRESSION_MODE FB_FALSE

FB_PACKAGE1(string)


/* Set the FB_STRING_DEBUG_STATS_ENABLED to true, and look at the debug stats. Then deduce the proper cache size from 
 * that. Current number is based on Trine 4 build (editor needs more). Probably not optimal for everything, but close 
 * enough */
#if FB_ENGINE_FOR_TOOLS == FB_TRUE
	static const SizeType expectedStringCacheSize = 1 << 11;
#else
	static const SizeType expectedStringCacheSize = 1 << 18;
#endif

/* String shorter than this have separate stats collected */
#define FB_STRING_STRINGIMP_STAT_MAX_STRING_LEN 512

/* Debug stats use StaticStrings, which makes tracking StringImp usage with debug stats somewhat problematic. The 
 * first StringImp to be constructed must not be part of any of StringImp debug stats. Having the scopeName defined 
 * here, before FB_DSTAT lines, instead of perhaps more logical place inside stringimpdetail, makes sure that is 
 * always the case. In practice it should be relatively low probability that no Static or DynamicString is used before 
 * static initialization here, but sure it happened anyway */
FB_STATIC_CONST_STRING(scopeName, "fb::string::StringImp");

FB_DSTAT(fb::string::StringImp, inPlaceCreatedAmount);
FB_DSTAT(fb::string::StringImp, inPlaceCreatedBytes);
FB_DSTAT(fb::string::StringImp, inPlaceCreateFailAmount);
FB_DSTAT(fb::string::StringImp, inPlaceCreateFailBytes);
FB_DSTAT(fb::string::StringImp, stringStaticCreatedAmount);
FB_DSTAT(fb::string::StringImp, stringStaticCreatedMemoryAmount);
FB_DSTAT(fb::string::StringImp, stringInstanceAmount);
FB_DSTAT(fb::string::StringImp, stringMemoryTotalAmount);
FB_DSTAT(fb::string::StringImp, stringMemoryOverheadBytes);
FB_DSTAT(fb::string::StringImp, averageStringSizeRounded);
FB_DEBUG_STATS_ARRAY_VAR_NO_LIMIT(fb::string::StringImp, stringsOfLength, FB_STRING_STRINGIMP_STAT_MAX_STRING_LEN + 1);

#if FB_STRING_DEBUG_STATS_ENABLED == FB_TRUE

	namespace stringimpdetail
	{
		#define FB_STRING_IMP_DSTAT_INIT(p_varname) \
			{ \
				FB_DSTAT_GET(p_varname); \
				FB_DSTAT_SET(p_varname, 0); \
			}

		static bool initStringImpStats()
		{
			static bool firstRun = true;
			static bool initialized = false;
			if (firstRun)
			{
				firstRun = false;
				FB_STRING_IMP_DSTAT_INIT(inPlaceCreatedAmount);
				FB_STRING_IMP_DSTAT_INIT(inPlaceCreatedBytes);
				FB_STRING_IMP_DSTAT_INIT(inPlaceCreateFailAmount);
				FB_STRING_IMP_DSTAT_INIT(inPlaceCreateFailBytes);
				FB_STRING_IMP_DSTAT_INIT(stringStaticCreatedAmount);
				FB_STRING_IMP_DSTAT_INIT(stringStaticCreatedMemoryAmount);
				FB_STRING_IMP_DSTAT_INIT(stringInstanceAmount);
				FB_STRING_IMP_DSTAT_INIT(stringMemoryTotalAmount);
				FB_STRING_IMP_DSTAT_INIT(stringMemoryOverheadBytes);
				FB_STRING_IMP_DSTAT_INIT(averageStringSizeRounded);

				FB_DEBUG_STATS_ARRAY_GET(stringsOfLength, 0);
				FB_DEBUG_STATS_ARRAY_SET(stringsOfLength, 0, 0);
				initialized = true;
			}
			return initialized;
		}

		static bool stringImpDebugStatsAvailable()
		{
			return initStringImpStats();
		}
	}

#else

namespace stringimpdetail
{

	static constexpr bool stringImpDebugStatsAvailable()
	{
		return false;
	}
}

#endif

FB_STACK_SET_CLASS(StringImp);

fb_static_assert(StringImpHelper::RefCountBytes == 4);
fb_static_assert(StringImpHelper::RefCountBytes == sizeof(lang::AtomicInt32));

// got tired of the bugs due to string and (non-string) buffer pointers getting mixed up.
typedef char * StringBufferPtr;

char *stringBufferPointerToStringPointer(StringBufferPtr buf)
{
	return reinterpret_cast<char *>(buf) + StringImpHelper::ShiftBytes;
}


StringBufferPtr stringPointerToStringBufferPointer(char *str)
{
	return reinterpret_cast<StringBufferPtr>(str - StringImpHelper::ShiftBytes);
}


static uint32_t getNextID()
{
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	static uint32_t id = 0;
	++id;
	return id;
#else
	return 0;
#endif
}

// Returns estimated refcounts as there can be multiple modify operations overlapping

static inline int32_t modifyReferenceCountAtomicInc(char *pointer)
{
	lang::AtomicInt32 *refCount = reinterpret_cast<lang::AtomicInt32 *> (pointer - StringImpHelper::RefcountOffset);
	int32_t oldRefcount = lang::atomicIncRelease(*refCount);

	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(oldRefcount >= 0);
	#endif

	int32_t newRefcount = oldRefcount + 1;

	// Questionable, but this still sort of makes sense. Avoids overflowing 16bit refcount.
	if (newRefcount >= StringImpHelper::refcountStaticConversionBoundary)
		StringImpHelper::setStatic(pointer, true);

	return newRefcount;
}


static inline int32_t modifyReferenceCountAtomicDec(char *pointer)
{
	lang::AtomicInt32 *refCount = reinterpret_cast<lang::AtomicInt32 *> (pointer - StringImpHelper::RefcountOffset);
	int32_t oldRefcount = lang::atomicDecAcquire(*refCount);

	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(oldRefcount > 0);
	#endif

	return oldRefcount - 1;
}


static inline int32_t getReferenceCountAtomic(char *pointer)
{
	lang::AtomicInt32 *refCount = reinterpret_cast<lang::AtomicInt32 *> (pointer - StringImpHelper::RefcountOffset);
	int32_t currentRefcount = lang::atomicLoadAcquire(*refCount);

	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(currentRefcount >= 0);
	#endif

	return (int32_t) currentRefcount;
}


static inline int32_t modifyReferenceCountInc(char *pointer)
{
	int32_t *refCount = reinterpret_cast<int32_t *> (pointer - StringImpHelper::RefcountOffset);
	int32_t oldRefcount = *refCount;
	++(*refCount);

#if FB_STRING_REGRESSION_MODE == FB_TRUE
	fb_assert(oldRefcount >= 0);
#endif

	int32_t newRefcount = oldRefcount + 1;

	// Questionable, but this still sort of makes sense. Avoids overflowing 16bit refcount.
	if (newRefcount >= StringImpHelper::refcountStaticConversionBoundary)
		StringImpHelper::setStatic(pointer, true);

	return newRefcount;
}


static inline int32_t modifyReferenceCountDec(char *pointer)
{
	int32_t *refCount = reinterpret_cast<int32_t *> (pointer - StringImpHelper::RefcountOffset);
	int32_t oldRefcount = *refCount;
	--(*refCount);

#if FB_STRING_REGRESSION_MODE == FB_TRUE
	fb_assert(oldRefcount > 0);
#endif

	return oldRefcount - 1;
}


static inline int32_t getReferenceCount(char *pointer)
{
	int32_t *refCount = reinterpret_cast<int32_t *> (pointer - StringImpHelper::RefcountOffset);
	int32_t currentRefcount = *refCount;

#if FB_STRING_REGRESSION_MODE == FB_TRUE
	fb_assert(currentRefcount >= 0);
#endif

	return currentRefcount;
}


// Pools

struct HashStringWrapper
{
	char *data = nullptr;
	SizeType length = 0;

	HashStringWrapper()
	{
	}

	HashStringWrapper(char *d, SizeType l)
		: data(d)
		, length(l)
	{
	}

	bool operator==(const HashStringWrapper &sw) const
	{
		if (length != sw.length)
			return false;

		return lang::MemCompare::equals(data, sw.data, length);
	}

	uint32_t getHash() const
	{
		return getHashValue(data, length, 0);
	}
};


struct HashStringFunctor
{
	uint32_t operator() (const HashStringWrapper &w) const
	{
		return w.getHash();
	}
};

typedef LinearHashSet<HashStringWrapper, HashStringFunctor> StringCache;
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	typedef LinearHashSet<uint32_t> StringIDCache;
#endif

class StringImplData
{
public:
	FB_ADD_GLOBAL_CLASS_MEMORY_OVERLOADS(StringImplData)

		StringImplData()
		: stringCache(nullptr)
		#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			, stringIDCache(nullptr)
		#endif
		/* Starting T4 build generates about 175 thousand static created strings using total of 14 MBs. Probably most 
		 * of those are file names, and as they are relatively speaking long, probably safe to assume that editor will 
		 * need about double due to files in builds folder (or more, if doing builds for many platforms) */
		#if FB_ENGINE_FOR_TOOLS == FB_TRUE
			/* We use osAllocate so we can not leak memory, tools case is usually simple enough */
		#elif FB_EDITOR_ENABLED == FB_FALSE
			, staticCreatedAllocator(getSystemHeapAllocator(), 8 * 1024 * 1024, 2, true)
		#else
			, staticCreatedAllocator(getSystemHeapAllocator(), 8 * 1024 * 1024, 4, true)
		#endif
	{
	}


	~StringImplData()
	{
#if FB_ENGINE_FOR_TOOLS == FB_TRUE
		for (StringCache::Iterator iter = stringCache->getBegin(); iter != stringCache->getEnd(); ++iter)
		{
			char *strPointer = (*iter).data;
			if (StringImpHelper::isStatic(strPointer) && !StringImpHelper::isInPlace(strPointer))
			{
				StringBufferPtr ptrToFree = stringPointerToStringBufferPointer(strPointer);
				lang::osFree(ptrToFree);
			}
		}

		delete stringCache;
		stringCache = nullptr;
#else
		/* Leak string cache. Otherwise every static StaticString and DynamicString will access crap memory in destructor */
		/* Change DynamicStrings to StaticString. Otherwise they will try to use non-existent mutex in destructor */
		uint32_t counter = 0;
		for (StringCache::Iterator iter = stringCache->getBegin(); iter != stringCache->getEnd(); ++iter)
		{
			++counter;
			char* strPointer = (*iter).data;
			if (!StringImpHelper::isStatic(strPointer))
				StringImpHelper::setStatic(strPointer, true);
		}
		fb_assert(counter == stringCache->getSize());
#endif
	}


	Mutex stringImpMutex;
	StringCache *stringCache;
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	StringIDCache *stringIDCache;
#endif

#if FB_ENGINE_FOR_TOOLS == FB_FALSE
	memory::AtomicLinearAllocator staticCreatedAllocator;
#endif
};


StringImplData &getStringImplData()
{
	static StringImplData stringImplData;
	return stringImplData;
}

#if FB_ENGINE_FOR_TOOLS == FB_TRUE
void StringImpHelper::initializeData()
{
	getStringImplData();
}
#endif

// Global cache functions

void initStringCacheIdNecessary()
{
	StringImplData &data = getStringImplData();
	if (data.stringCache != nullptr)
		return;

	data.stringCache = new StringCache();
	data.stringCache->reserve(expectedStringCacheSize);
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	data.stringIDCache = new StringIDCache();
	data.stringIDCache->reserve(expectedStringCacheSize);
#endif
}
		
StringBufferPtr findCachedPointer(const char *ptr, SizeType length)
{
	StringImplData &data = getStringImplData();
	initStringCacheIdNecessary();

	HashStringWrapper w((char*) ptr, length);
	StringCache::Iterator it = data.stringCache->find(w);
	if (it == data.stringCache->getEnd())
		return 0;

	return stringPointerToStringBufferPointer((*it).data);
}

void addCachedPointer(char *cachePointer, SizeType length)
{
	StringImplData &data = getStringImplData();
	initStringCacheIdNecessary();
	char *stringPtr = stringBufferPointerToStringPointer(cachePointer);
	HashStringWrapper w(stringPtr, length);
	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(data.stringCache->find(w) == data.stringCache->getEnd());
	#endif

	data.stringCache->insert(w);

#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	/* In place constructed strings won't get removed, so they don't need ID */
	if (StringImpHelper::isInPlace(stringPtr))
		return;
	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(data.stringIDCache->find(StringImpHelper::getId(stringPtr)) == data.stringIDCache->getEnd());
	#endif

	data.stringIDCache->insert(StringImpHelper::getId(stringPtr));
#endif

	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(data.stringCache->find(w) != data.stringCache->getEnd());
	#endif
}


void removeCachedPointer(char *cachePointer, uint32_t pointerId, SizeType length)
{
	StringImplData &data = getStringImplData();
	initStringCacheIdNecessary();

#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	{
		StringIDCache::Iterator it = data.stringIDCache->find(pointerId);
		fb_assert(it != data.stringIDCache->getEnd());
		data.stringIDCache->erase(it);
	}
#endif
	{
		HashStringWrapper w(stringBufferPointerToStringPointer(cachePointer), length);
		StringCache::Iterator it = data.stringCache->find(w);
		fb_assert(it != data.stringCache->getEnd());
		data.stringCache->erase(it);
	}
}


#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
bool hasCachedPointer(uint32_t pointerID)
{
	StringImplData &data = getStringImplData();
	fb_assert(data.stringCache != NULL);
	StringIDCache::Iterator it = data.stringIDCache->find(pointerID);
	return it != data.stringIDCache->getEnd();
}
#endif

// Pool functions

void resetProperties(StringBufferPtr ptr, uint32_t pointerID, SizeType length, bool staticFlag, const char *originalString)
{
	memset(ptr, 0, StringImpHelper::OverheadBytes);
	char *tempPtr = stringBufferPointerToStringPointer(ptr);
	StringImpHelper::setId(tempPtr, pointerID);
	StringImpHelper::setReferenceCount(tempPtr, 0);
	StringImpHelper::setLength(tempPtr, length);
	StringImpHelper::setStatic(tempPtr, staticFlag);

	// And copy the actual string
	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(cstrlen(originalString) >= length && "Given string is shorter than specified");
	#endif
	memcpy(tempPtr, originalString, length);
	// And null
	tempPtr += length;
	*tempPtr = '\0';
}


StringBufferPtr addStringToPool(const char *str, SizeType length, bool staticFlag, StringBufferPtr inPlacePtr, bool &wasFound)
{
	// Global pointer cache
	StringBufferPtr cacheResult = findCachedPointer(str, length);
	if (cacheResult)
	{
		wasFound = true;
		if (stringimpdetail::stringImpDebugStatsAvailable() && inPlacePtr)
		{
			FB_DSTAT_INC(inPlaceCreateFailAmount);
			FB_DSTAT_ADD(inPlaceCreateFailBytes, length + StringImpHelper::OverheadBytes);
		}
		return cacheResult;
	}

	wasFound = false;

	/* We only want to use inPlacePtr, if such string doesn't exists yet. Thus this code is after above search */
	if (inPlacePtr)
	{
		if (stringimpdetail::stringImpDebugStatsAvailable())
		{
			FB_DSTAT_INC(inPlaceCreatedAmount);
			FB_DSTAT_ADD(inPlaceCreatedBytes, length + StringImpHelper::OverheadBytes);
		}
		fb_assert(StringImpHelper::isInPlace(str) && "In place construction failed due to crappy data");
		fb_assert(StringImpHelper::isStatic(str) && "In place construction failed due to crappy data");
		fb_assert(StringImpHelper::getLength(str) == cstrlen(str) && "In place construction failed due to crappy data");
		addCachedPointer(inPlacePtr, StringImpHelper::getLength(str));
		return inPlacePtr;
	}

	// New string debug stats
	if (stringimpdetail::stringImpDebugStatsAvailable())
	{
		if (length < FB_STRING_STRINGIMP_STAT_MAX_STRING_LEN)
		{
			FB_DEBUG_STATS_ARRAY_INC(stringsOfLength, length);
		}
		else
		{
			/* Put number of large strings to zero slot */
			FB_DEBUG_STATS_ARRAY_INC(stringsOfLength, 0);
		}
		FB_DSTAT_ADD(stringMemoryTotalAmount, length + StringImpHelper::OverheadBytes);
		FB_DSTAT_ADD(stringMemoryOverheadBytes, StringImpHelper::OverheadBytes);
		FB_DSTAT_INC(stringInstanceAmount);
		if (FB_DEBUG_STATS_GET(stringInstanceAmount) > 0)
		{
			FB_DEBUG_STATS_SET(averageStringSizeRounded, (FB_DEBUG_STATS_GET(stringMemoryTotalAmount) - FB_DEBUG_STATS_GET(stringMemoryOverheadBytes)) / FB_DEBUG_STATS_GET(stringInstanceAmount));
		}
		else
		{
			FB_DEBUG_STATS_SET(averageStringSizeRounded, 0);
		}
		if (staticFlag)
		{
			FB_DSTAT_INC(stringStaticCreatedAmount);
			FB_DSTAT_ADD(stringStaticCreatedMemoryAmount, length + StringImpHelper::OverheadBytes);
		}
	}

	// No match, create a new one.
	StringBufferPtr ptr = nullptr;
	if (!staticFlag)
	{
		ptr = (StringBufferPtr) lang::allocateFixed(length + StringImpHelper::OverheadBytes);
		/* Use this for brute-force, eyeball profiling of Dynamic vs. StaticStrings */
		//static uint32_t count = 0;
		//++count;
		///* Loan the just allocated pointer to print only the part of the string actually used */
		//strncpy(ptr, str, length);
		//ptr[length] = '\0';
		//FB_PRINTF("Created non-static DynamicString (%d) %s\n", count, ptr);
	}
	else
	{
		/* Static allocations are done from separate allocator mostly to avoid spamming rather huge amount of 
		 * allocation tracker spam on start. Of course, this should also reduce overhead in general, but there's no 
		 * measurable performance benefit in real code */
		StringImplData &data = getStringImplData();
#if FB_ENGINE_FOR_TOOLS == FB_TRUE
		ptr = (StringBufferPtr)lang::osAllocate(length + StringImpHelper::OverheadBytes);
#endif
	}

	// And reset properties.
	resetProperties(ptr, getNextID(), length, staticFlag, str);

	addCachedPointer(ptr, length);
	return ptr;
}

void tryRemoveStringFromPool(char *str, uint32_t pointerId, SizeType length)
{
	// We are (should be) inside the creation mutex. We might come here due to atomic decrement overlapping with new string creation
	// meaning str might actually have an active refcount.
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	if (!hasCachedPointer(pointerId))
	{
		/* Someone managed to delete the pointer before we had a change */
		/* Note: this only happens, if string gets re-referenced after ref count was zeroed, but before we have a 
		 * change to complete deletion. Then the ref count got decreased again and another thread did the deletion. */
		return;
	}

	fb_assert(StringImpHelper::getId(str) == pointerId);

	int32_t currentRefcount = getReferenceCountAtomic(str);
	fb_assert(currentRefcount >= 0);
	if (currentRefcount)
	{
		// Don't remove if someone is already using it.
		return;
	}

#else
	fb_assertf(getReferenceCount(str) == 0, "Reference count should be zero, was %d", getReferenceCount(str));
#endif

	if (StringImpHelper::isStatic(str))
	{
		/* Don't remove, someone has made it static. */
		fb_assert(0 && "This should not happen. If someone has made it static, it should have at least that one reference, so should not get here at all or return above");
		return;
	}

	removeCachedPointer(stringPointerToStringBufferPointer(str), pointerId, length);
	fb_assert(!StringImpHelper::isStatic(str));

	// New string debug stats
	if (stringimpdetail::stringImpDebugStatsAvailable())
	{
		if (length < FB_STRING_STRINGIMP_STAT_MAX_STRING_LEN)
		{
			FB_DEBUG_STATS_ARRAY_DEC(stringsOfLength, length);
		}
		else
		{
			/* Large strings in zero slot */
			FB_DEBUG_STATS_ARRAY_DEC(stringsOfLength, 0);
		}
		FB_DSTAT_SUB(stringMemoryTotalAmount, length + StringImpHelper::OverheadBytes);
		FB_DSTAT_SUB(stringMemoryOverheadBytes, StringImpHelper::OverheadBytes);
		FB_DEBUG_STATS_DEC(stringInstanceAmount);
		if (FB_DEBUG_STATS_GET(stringInstanceAmount) > 0)
		{
			FB_DEBUG_STATS_SET(averageStringSizeRounded, (FB_DEBUG_STATS_GET(stringMemoryTotalAmount) - FB_DEBUG_STATS_GET(stringMemoryOverheadBytes)) / FB_DEBUG_STATS_GET(stringInstanceAmount));
		}
		else
		{
			FB_DEBUG_STATS_SET(averageStringSizeRounded, 0);
		}
	}

	lang::freeFixed(stringPointerToStringBufferPointer(str), length + StringImpHelper::OverheadBytes);
}


// this requires a bit of a detour to get past static initialization order problems...
Mutex &getStringImpMutex()
{
	static Mutex &stringImpMutex = getStringImplData().stringImpMutex;
	return stringImpMutex;
}

StringImp::StringImp(const char *ptr, SizeType length, bool isStatic)
	: pointer(getEmptyStringPointer())
{
	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(cstrlen(ptr) >= length && "Suspicious string construction taking place");
	#endif
	create(ptr, length, isStatic, nullptr);
}

#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
// [ID][RefCount][StringLength][StaticFlag][Actual string]
char StringImp::emptyStringData[StringImpHelper::ShiftBytes + 1] = { /* ID */ 0, 0, 0, 0, /* RefCount */ 0, 0, 0, 0, /* StringLength */ 0, 0, /* StaticFlag */ 1, /* Actual string */ '\0' };
#else
// [RefCount][StringLength][StaticFlag][Actual string]
char StringImp::emptyStringData[StringImpHelper::ShiftBytes + 1] = { /* RefCount */ 0, 0, 0, 0, /* StringLength */ 0, 0, /* StaticFlag */ 1, /* Actual string */ '\0' };
#endif

StringImp::StringImp(char *ptr)
	: pointer(getEmptyStringPointer())
{
	const char *strPtr = stringBufferPointerToStringPointer(ptr);
	SizeType length = StringImpHelper::getLength(strPtr);
	create(strPtr, length, true, ptr);
}


StringImp::StringImp(const StringImp &other)
	: pointer(getEmptyStringPointer())
{
	copyFrom(other);
}


void StringImp::unsafeCopy(const StringImp &other)
{
	fb_assert(isStatic() && other.isStatic());
	pointer = other.pointer;
}


void StringImp::operator= (const StringImp &other)
{
	copyFrom(other);
}


bool StringImp::operator<(const StringImp &other) const
{
	return strcmp(pointer, other.getPointer()) < 0;
}

bool StringImp::operator>(const StringImp &other) const
{
	return strcmp(pointer, other.getPointer()) > 0;
}

bool StringImp::operator<=(const StringImp &other) const
{
	return pointer == other.pointer || (strcmp(pointer, other.getPointer()) < 0);
}


bool StringImp::operator>=(const StringImp &other) const
{
	return pointer == other.pointer || (strcmp(pointer, other.getPointer()) > 0);
}

void StringImp::create(const char *ptr, SizeType length, bool staticFlag, StringBufferPtr isInPlacePtr)
{
	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(ptr);
	#endif
	if (length == 0)
	{
		pointer = getEmptyStringPointer();
		return;
	}

	FB_ZONE("StringImp::create");

	// Mutex
	MutexGuard guard(getStringImpMutex());

	bool wasFound;

	// Get the pointer (shifted)
	pointer = stringBufferPointerToStringPointer(addStringToPool(ptr, length, staticFlag, isInPlacePtr, wasFound));

	// Refcount
	if (!this->isStatic())
	{
		if (staticFlag)
		{
			StringImpHelper::setStatic(pointer, true);
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			if (getReferenceCountAtomic(pointer) == 0)
				modifyReferenceCountAtomicInc(pointer);
#else
			if (getReferenceCount(pointer) == 0)
				modifyReferenceCountInc(pointer);
#endif
		}
		else
		{
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			#if FB_STRING_REGRESSION_MODE == FB_TRUE
				int32_t refCount = modifyReferenceCountAtomicInc(pointer);
				fb_assert(refCount >= 1);
			#elif FB_STRING_REGRESSION_MODE == FB_FALSE
				modifyReferenceCountAtomicInc(pointer);
			#endif
#else
			#if FB_STRING_REGRESSION_MODE == FB_TRUE
				int32_t refCount = modifyReferenceCountInc(pointer);
				fb_assert(refCount >= 1);
			#elif FB_STRING_REGRESSION_MODE == FB_FALSE
				modifyReferenceCountInc(pointer);
			#endif
#endif
		}
	}

	#if FB_STRING_REGRESSION_MODE == FB_TRUE
		fb_assert(getLength() == length);
	#endif
}


void StringImp::copyFrom(const StringImp &other)
{
	// We can't do the code below in case we are assigning the same pointer!
	// It results of pointer being freed and still being used
	if (pointer == other.pointer)
		return;

	if (other.isStatic())
	{
		// Even when other pointer is static, we might not be
		if (!isStatic())
			reset();

		pointer = other.pointer;
		return;
	}

	// Remove old ref
#if FB_STRING_IMP_USE_ATOMICS == FB_FALSE
	MutexGuard guard(getStringImpMutex());
#endif
	if (!isStatic())
	{
		uint32_t id = StringImpHelper::getId(pointer);
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
		int32_t refCount = modifyReferenceCountAtomicDec(pointer);
#else
		int32_t refCount = modifyReferenceCountDec(pointer);
#endif
		if (refCount == 0)
		{
			/* Even when refcount is 0, someone else could have created the same string before we hit the mutex and
			 * start removing it. This is fine, but have to check in remove code if the pointer is still the same and
			 * it still has 0 refcount. */
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			MutexGuard guard(getStringImpMutex());
#endif
			tryRemoveStringFromPool(pointer, id, getLength());
		}
	}

	pointer = other.pointer;
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	modifyReferenceCountAtomicInc(pointer);
#else
	modifyReferenceCountInc(pointer);
#endif
}


void StringImp::reset()
{
	if (isStatic())
	{
		pointer = getEmptyStringPointer();
		return;
	}

	FB_ZONE("StringImp::reset");

	uint32_t id = StringImpHelper::getId(pointer);
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	int32_t refCount = modifyReferenceCountAtomicDec(pointer);
#else
	MutexGuard guard(getStringImpMutex());
	/* Check again, as this may have changed before we got the mutex */
	if (isStatic())
	{
		pointer = getEmptyStringPointer();
		return;
	}
	int32_t refCount = modifyReferenceCountDec(pointer);
#endif
	if (refCount == 0)
	{
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
		/* Even when refcount is 0, someone else could have created the same string before we hit the mutex and
		 * start removing it. This is fine, but have to check in remove code if the pointer is still the same and
		 * it still has 0 refcount. */
		MutexGuard guard(getStringImpMutex());
#endif
		tryRemoveStringFromPool(pointer, id, getLength());
	}

	pointer = getEmptyStringPointer();
}


FB_END_PACKAGE1()
