#pragma once

#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/IsTriviallyCopyable.h"
#include "fb/lang/platform/ByteOrder.h"
#include "fb/lang/platform/Decl.h"
#include "fb/lang/platform/ForceInline.h"
#include "fb/lang/Types.h"

/* These wrapper functions are here mostly to avoid the need to include <memory> in a (precompiled) header.
 *
 * Performance should be about equal to just calling memcpy and co. Interfaces should be identical, apart from few 
 * minor things (equals in MemCompare, no return value in MemCopy). It was tested in Space by adding cumulative 
 * ScopedProfiler to EntityDamageAsync::islandUpdate() and spawning a ship with console command:
 *     spaceBaseNetManagerInst:devLoadShipFromMissionId("spaceship_armed_republic_interceptor")
 *
 * If in doubt, setting FB_MEMTOOLS_ENABLED to FB_FALSE can be done (maybe should be done in FinalRelease anyway?).
 */

#define FB_MEMTOOLS_ENABLED FB_TRUE
/* The templated versions actually seem to beat compiler in some cases, as they can assume things about alignment */
#define FB_MEMTOOLS_TEMPLATED_ENABLED FB_TRUE

/* Avoid including <memory> */
extern "C" void *memcpy(void *, const void *, size_t);
extern "C" int memcmp(const void * ptr1, const void * ptr2, size_t num);
extern "C" void *memset(void * ptr, int value, size_t num);

#if FB_MEMTOOLS_ENABLED == FB_FALSE
	#define FB_MEMTOOLS_MEMCPY_ENABLED FB_FALSE
	#define FB_MEMTOOLS_MEMSET_ENABLED FB_FALSE
	#define FB_MEMTOOLS_MEMCMP_ENABLED FB_FALSE
#else
	#define FB_MEMTOOLS_MEMCPY_ENABLED FB_TRUE
	#define FB_MEMTOOLS_MEMSET_ENABLED FB_TRUE
	#define FB_MEMTOOLS_MEMCMP_ENABLED FB_TRUE
#endif


#if FB_MEMTOOLS_ENABLED == FB_TRUE && FB_BYTE_ORDER == FB_BIG_ENDIAN
	#error Multibyte optimizations don't work in big endian environment
#endif

FB_PACKAGE1(lang)

class MemCompare
{
public:
#if FB_MEMTOOLS_TEMPLATED_ENABLED == FB_TRUE
	template <typename T>
	static FB_FORCEINLINE bool equals(const T *first, const T *second, uint64_t size)
	{
		const uint8_t *bytePtr1 = reinterpret_cast<const uint8_t*>(first);
		const uint8_t *bytePtr2 = reinterpret_cast<const uint8_t*>(second);
		if (alignof(T) % 8 == 0)
		{
			const uint64_t *ptr1 = reinterpret_cast<const uint64_t*>(first);
			const uint64_t *ptr2 = reinterpret_cast<const uint64_t*>(second);
			while (size >= 8)
			{
				if (*ptr1 != *ptr2)
					return false;

				++ptr1;
				++ptr2;
				size -= 8;
			}
			bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
			bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
		}
		else if (alignof(T) % 4 == 0)
		{
			const uint32_t *ptr1 = reinterpret_cast<const uint32_t*>(first);
			const uint32_t *ptr2 = reinterpret_cast<const uint32_t*>(second);
			while (size >= 4)
			{
				if (*ptr1 != *ptr2)
					return false;

				++ptr1;
				++ptr2;
				size -= 4;
			}
			bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
			bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
		}
		for (uint64_t i = 0; i < size; ++i)
		{
			if (bytePtr1[i] != bytePtr2[i])
				return false;
		}
		return true;
	}

	template <typename T>
	static FB_FORCEINLINE int32_t order(const T *first, const T *second, uint64_t size)
	{
		const uint8_t *bytePtr1 = reinterpret_cast<const uint8_t*>(first);
		const uint8_t *bytePtr2 = reinterpret_cast<const uint8_t*>(second);
		if (alignof(T) % 8 == 0)
		{
			const uint64_t *ptr1 = reinterpret_cast<const uint64_t*>(first);
			const uint64_t *ptr2 = reinterpret_cast<const uint64_t*>(second);
			while (size >= 8)
			{
				if (*ptr1 < *ptr2)
					return -1;
				else if (*ptr1 > *ptr2)
					return 1;

				++ptr1;
				++ptr2;
				size -= 8;
			}
			bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
			bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
		}
		else if (alignof(T) % 4 == 0)
		{
			const uint32_t *ptr1 = reinterpret_cast<const uint32_t*>(first);
			const uint32_t *ptr2 = reinterpret_cast<const uint32_t*>(second);
			while (size >= 4)
			{
				if (*ptr1 < *ptr2)
					return -1;
				else if (*ptr1 > *ptr2)
					return 1;

				++ptr1;
				++ptr2;
				size -= 4;
			}
			bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
			bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
		}
		for (uint64_t i = 0; i < size; ++i)
		{
			if (bytePtr1[i] < bytePtr2[i])
				return -1;
			else if (bytePtr1[i] > bytePtr2[i])
				return 1;
		}
		return 0;
	}
#endif

	static FB_FORCEINLINE bool equals(const void *first, const void *second, uint64_t size)
	{
		#if FB_MEMTOOLS_MEMCMP_ENABLED == FB_TRUE
			const uint8_t *bytePtr1 = reinterpret_cast<const uint8_t*>(first);
			const uint8_t *bytePtr2 = reinterpret_cast<const uint8_t*>(second);
			uintptr_t firstPtrValue = uintptr_t(first);
			uintptr_t secondPtrValue = uintptr_t(second);
			if ((firstPtrValue & 0x7) == 0 && (secondPtrValue & 0x7) == 0)
			{
				/* Aligned to 8 bytes */
				const uint64_t *ptr1 = reinterpret_cast<const uint64_t*>(first);
				const uint64_t *ptr2 = reinterpret_cast<const uint64_t*>(second);
				while (size >= 8)
				{
					if (*ptr1 != *ptr2)
						return false;

					++ptr1;
					++ptr2;
					size -= 8;
				}
				bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
				bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
			}
			for (uint64_t i = 0; i < size; ++i)
			{
				if (bytePtr1[i] != bytePtr2[i])
					return false;
			}
			return true;
		#else
			return memcmp(first, second, size) == 0;
		#endif
	}

	static FB_FORCEINLINE int32_t order(const void *first, const void *second, uint64_t size)
	{
		#if FB_MEMTOOLS_MEMCMP_ENABLED == FB_TRUE
			const uint8_t *bytePtr1 = reinterpret_cast<const uint8_t*>(first);
			const uint8_t *bytePtr2 = reinterpret_cast<const uint8_t*>(second);
			uintptr_t firstPtrValue = uintptr_t(first);
			uintptr_t secondPtrValue = uintptr_t(second);
			if ((firstPtrValue & 0x7) == 0 && (secondPtrValue & 0x7) == 0)
			{
				/* Aligned to 8 bytes */
				const uint64_t *ptr1 = reinterpret_cast<const uint64_t*>(first);
				const uint64_t *ptr2 = reinterpret_cast<const uint64_t*>(second);
				while (size >= 8)
				{
					if (*ptr1 < *ptr2)
						return -1;
					else if (*ptr1 > *ptr2)
						return 1;

					++ptr1;
					++ptr2;
					size -= 8;
				}
				bytePtr1 = reinterpret_cast<const uint8_t*>(ptr1);
				bytePtr2 = reinterpret_cast<const uint8_t*>(ptr2);
			}
			for (uint64_t i = 0; i < size; ++i)
			{
				if (bytePtr1[i] < bytePtr2[i])
					return -1;
				else if (bytePtr1[i] > bytePtr2[i])
					return 1;
			}
			return 0;
		#else
			return int32_t(memcmp(first, second, size));
		#endif
	}
};


class MemCopy
{
public:
#if FB_MEMTOOLS_TEMPLATED_ENABLED == FB_TRUE
	template <typename T>
	static FB_FORCEINLINE void copy(T * FB_RESTRICT destination, const T * FB_RESTRICT source, uint64_t sizeInBytes)
	{
		fb_static_assert(lang::IsTriviallyCopyable<T>::value && "One shouldn't be using MemCopy for non-trivially copyable types");
		fb_assert(sizeInBytes % sizeof(T) == 0 && "Mismatching object type and copy size");
		for (uint64_t i = 0, num = sizeInBytes / sizeof(T); i < num; ++i)
			destination[i] = source[i];
	}
#endif
	static FB_FORCEINLINE void copy(void * FB_RESTRICT destination, const void * FB_RESTRICT source, uint64_t size)
	{
		memcpy(destination, source, size);
	}

	static FB_FORCEINLINE void copyVoid(void * FB_RESTRICT destination, const void * FB_RESTRICT source, uint64_t size)
	{
		memcpy(destination, source, size);
	}
};


class MemSet
{
public:
	static FB_FORCEINLINE void set(void *destination, uint8_t value, uint64_t size)
	{
		memset(destination, value, size);
	}
};


FB_END_PACKAGE1()
