#include "Precompiled.h"
#include "ImpByteElementArray.h"

#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/platform/Decl.h"

#include <cstring> // For memcpy and memmove

FB_PACKAGE1(container)

//#define USE_GLOBAL_HEAP

static inline uint32_t getNextPow2(uint32_t value)
{
	FB_UNUSED_NAMED_VAR(uint32_t, originalValue) = value;
	// If we go outside our range
	uint32_t overflowSize = value >= 0x80000000 ? 0xffffffff : 0;
	// Skip 1 and 2 element arrays as they generate a lot of overhead
	value = value < 4 ? 4 : value;

	// In case we are just below the next pow2, make sure we jump over it
	// (doesnt do anything if already pow2)
	value += value/2;

	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;

	value = overflowSize ? overflowSize : value;
	fb_assert(value >= originalValue);
	return value;
}

uint32_t ImpByteElementArray::impGetNextPow2(uint32_t size)
{
	return getNextPow2(size);
}

ImpByteElementArray::ImpByteElementArray(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve)
	: intPointer(uintptr_t(staticPointer) | StaticPointerBit)
	, size(0)
	, capacity(staticSizeInElements)
{
	intPointer |= (assertOnReserve) ? ReserveAssertPointerBit : 0;

	fb_assert(staticPointer && staticSizeInElements);
	fb_assert((uintptr_t(staticPointer) & PointerDataMask) == 0);
}

void *ImpByteElementArray::impGetNewPointer(uint32_t elementSizeInBytes, uint32_t &elementAmount)
{
	fb_assert(!shouldAssertOnReserve());
	elementAmount = getNextPow2(elementAmount);
	return impGetNewPointerExact(elementSizeInBytes, elementAmount);
}

void *ImpByteElementArray::impGetNewPointerExact(uint32_t elementSizeInBytes, uint32_t &elementAmount)
{
	uint64_t newCapacityInBytes = uint64_t(elementSizeInBytes) * elementAmount;
#ifdef USE_GLOBAL_HEAP
		void *pointer = realloc (nullptr, newCapacityInBytes);
	#else
		void *pointer = lang::reallocateFixed(nullptr, 0, newCapacityInBytes);
	#endif

	fb_assert((uintptr_t(pointer) & PointerDataMask) == 0);
	return pointer;
}

void ImpByteElementArray::impSwapPointer(uint32_t elementSizeInBytes, void *pointer, uint32_t newCapacity)
{
	uint32_t localSize = getSize();
	impReset(elementSizeInBytes);

	intPointer = (uintptr_t) pointer;
	capacity = newCapacity;
	size = localSize;
#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
	cachedAllocationByteSize = uint64_t(newCapacity) * elementSizeInBytes;
#endif
}

void ImpByteElementArray::impSwapToStaticPointer(uint32_t elementSizeInBytes, void *pointer, uint32_t pointerSizeInBytes, bool assertOnReserve)
{
	fb_assert(getSize() == 0);
	// Do we already have a static pointer?
	fb_assert(!shouldAssertOnReserve());

	impReset(elementSizeInBytes);

	intPointer = (uintptr_t) pointer;
	capacity = pointerSizeInBytes / elementSizeInBytes;
	intPointer |= StaticPointerBit;
	intPointer |= (assertOnReserve) ? ReserveAssertPointerBit : 0;

#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
	cachedAllocationByteSize = uint64_t(capacity) * elementSizeInBytes;
#endif
}

void ImpByteElementArray::impZeroPointer()
{
	fb_assert(!shouldAssertOnReserve());
	intPointer = 0;
	size = 0;
	capacity = 0;

#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
	cachedAllocationByteSize = 0;
#endif
}

void ImpByteElementArray::impReset(uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);

	char *pointer = getBytePointer();
	if (pointer && !isStaticAllocation())
	{
		#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
			fb_assert(uint64_t(getCapacity()) * elementSizeInBytes == cachedAllocationByteSize);
		#endif
	
		#ifdef USE_GLOBAL_HEAP
			free(pointer);
		#else
			lang::freeFixed(pointer, uint64_t(getCapacity()) * elementSizeInBytes);
		#endif
		intPointer = 0;
		capacity = 0;

		#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
			cachedAllocationByteSize = 0;
		#endif
	}

	size = 0;
}

#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
	void ImpByteElementArray::validate(uint32_t elementSizeInBytes) const
	{
		cachedElementSize = cachedElementSize ? cachedElementSize : elementSizeInBytes;
		fb_assert(cachedElementSize == elementSizeInBytes);
		fb_assert(getSize() <= getCapacity());
		fb_assert(isStaticAllocation() || (cachedAllocationByteSize == uint64_t(getCapacity()) * elementSizeInBytes));
	}
#endif

ImpByteElementArray::ImpByteElementArray()
{
}

ImpByteElementArray::~ImpByteElementArray()
{
	// We don't have enough information to destruct pointer, so assert user has done it manually.
	fb_assert(isStaticAllocation() || !getBytePointer());
}

void ImpByteElementArray::impSwapOrdered(ImpByteElementArray &other, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	fb_assert(this != &other);

	// ToDo:
	// This is somewhat annoying when dealing with possible static pointers on either side.
	// This crappy cop-out will convert both sides to dynamic which is just plain rubbish

	// At the very least, do in-place copy for case where both instances have static allocation and enough space for each others data.

	if (isStaticAllocation())
		impConvertToDynamic(elementSizeInBytes);

	if (other.isStaticAllocation())
		other.impConvertToDynamic(elementSizeInBytes);

	// Plain swap

	uintptr_t tmpIntPointer = other.intPointer;
	uint32_t tmpSize = other.size;
	uint32_t tmpCapacity = other.capacity;

	other.intPointer = intPointer;
	other.size = size;
	other.capacity = capacity;

	intPointer = tmpIntPointer;
	size = tmpSize;
	capacity = tmpCapacity;

	#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
		uint64_t tempCachedAllocationByteSize = other.cachedAllocationByteSize;
		other.cachedAllocationByteSize = cachedAllocationByteSize;
		cachedAllocationByteSize = tempCachedAllocationByteSize;
	#endif
}

void ImpByteElementArray::impMoveOrdered(ImpByteElementArray &other, uint32_t elementSizeInBytes)
{
	if (this == &other)
		return;

	validate(elementSizeInBytes);
	fb_assert(this != &other);

	if (isStaticAllocation())
	{
		if (getCapacity() >= other.getSize())
		{
			// If enough space, just do plain copy and quit.
			// Other still in valid state.
			char *bytePointer = getBytePointer();
			memcpy(bytePointer, other.getBytePointer(), other.getSize() * elementSizeInBytes);
			size = other.getSize();
			return;
		}

		// If not enough space, reset local data to prevent it being copied over.
		intPointer = 0;
		size = 0;
		capacity = 0;

		#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
				cachedAllocationByteSize = 0;
		#endif
	}

	if (other.isStaticAllocation())
	{
		if (getCapacity() >= other.getSize())
		{
			// If enough space, just do plain copy and quit.
			// Other still in valid state.
			char *bytePointer = getBytePointer();
			memcpy(bytePointer, other.getBytePointer(), other.getSize() * elementSizeInBytes);
			size = other.getSize();
			return;
		}

		other.impConvertToDynamic(elementSizeInBytes);
	}

	// Plain pointer swap
	impPointerSwap(other);
}

void ImpByteElementArray::impPointerSwap(ImpByteElementArray &other)
{
	uintptr_t tmpIntPointer = other.intPointer;
	uint32_t tmpSize = other.size;
	uint32_t tmpCapacity = other.capacity;

	other.intPointer = intPointer;
	other.size = size;
	other.capacity = capacity;

	intPointer = tmpIntPointer;
	size = tmpSize;
	capacity = tmpCapacity;

	#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
		uint64_t tempCachedAllocationByteSize = other.cachedAllocationByteSize;
		other.cachedAllocationByteSize = cachedAllocationByteSize;
		cachedAllocationByteSize = tempCachedAllocationByteSize;
	#endif
}

void ImpByteElementArray::impConvertToDynamic(uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	fb_assert(isStaticAllocation());

	// We can safely copy old state to stack and 'leak' it as it's static
	const char *oldBytePointer = getBytePointer();
	uint32_t oldSizeInElements = getSize();

	intPointer = 0;
	size = 0;
	capacity = 0;

	#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
		cachedAllocationByteSize = 0;
	#endif

	impInsertArrayOrdered(0, oldBytePointer, oldSizeInElements, elementSizeInBytes);
}

void ImpByteElementArray::impCopyFromUnordered(const ImpByteElementArray &other, uint32_t elementSizeInBytes, const void *optionalDefaultValues)
{
	if (this == &other)
		return;

	validate(elementSizeInBytes);
	other.validate(elementSizeInBytes);

	if (other.isEmpty())
	{
		clear();
		return;
	}

	uint32_t currentCapacityInElements = getCapacity();
	uint32_t otherCapacityInElements = other.getCapacity();
	if (otherCapacityInElements > currentCapacityInElements)
		impReserveInElementsExactUnordered(elementSizeInBytes, otherCapacityInElements, optionalDefaultValues);

	fb_assert(getCapacity() >= other.getCapacity());

	char *pointer = getBytePointer();
	memcpy(pointer, other.getBytePointer(), uint64_t(otherCapacityInElements) * elementSizeInBytes);
	size =  other.getSize();
}

void ImpByteElementArray::impCopyFromOrdered(const ImpByteElementArray &other, uint32_t elementSizeInBytes)
{
	if (this == &other)
		return;

	validate(elementSizeInBytes);
	other.validate(elementSizeInBytes);

	if (other.isEmpty())
	{
		clear();
		impTrimMemoryOrdered(elementSizeInBytes);
		return;
	}

	uint32_t currentCapacityInElements = getCapacity();
	uint32_t otherSizeInElements = other.getSize();
	if (otherSizeInElements > currentCapacityInElements)
		impReserveInElementsExactOrdered(elementSizeInBytes, otherSizeInElements);
	
	fb_assert(getCapacity() >= otherSizeInElements);

	char *pointer = getBytePointer();
	memcpy(pointer, other.getBytePointer(), uint64_t(otherSizeInElements) * elementSizeInBytes);
	size = otherSizeInElements;
}

void ImpByteElementArray::impReserveInElementsExactUnordered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements, const void *optionalDefaultValues)
{
	validate(elementSizeInBytes);
	fb_assert(newCapacityInElements >= getSize());

	uint32_t currentCapacityInElements = getCapacity();
	if (newCapacityInElements == currentCapacityInElements)
		return;

	if (!isStaticAllocation())
	{
		#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
			fb_assert(cachedAllocationByteSize == uint64_t(currentCapacityInElements) * elementSizeInBytes);
		#endif

		fb_assert(!shouldAssertOnReserve());
		uint64_t newCapacityInBytes = uint64_t(elementSizeInBytes) * newCapacityInElements;

		#ifdef USE_GLOBAL_HEAP
			intPointer = (uintptr_t) realloc (getBytePointer(), newCapacityInBytes);
		#else
			intPointer = (uintptr_t) lang::reallocateFixed(getBytePointer(), uint64_t(currentCapacityInElements) * elementSizeInBytes, newCapacityInBytes);
		#endif
	}
	else
	{
		// No point to replace our static allocation with a smaller dynamic one
		if (newCapacityInElements < currentCapacityInElements)
			return;

		const char *oldBytePointer = getBytePointer();
		fb_assert(oldBytePointer);

		/*
		#ifdef USE_GLOBAL_HEAP
			intPointer = (uintptr_t) realloc (nullptr, newCapacityInBytes);
		#else
			intPointer = (uintptr_t) reallocateMemory<true> (nullptr, 0, newCapacityInBytes);
		#endif
		*/

		intPointer = (uintptr_t) impGetNewPointer(elementSizeInBytes, newCapacityInElements);

		// Copy over the old static data.
		memcpy(getBytePointer(), oldBytePointer, uint64_t(getSize()) * elementSizeInBytes);
	}

	fb_assert((intPointer & PointerDataMask) == 0);

	if (optionalDefaultValues && (newCapacityInElements > currentCapacityInElements))
	{
		for (uint32_t i = currentCapacityInElements; i < newCapacityInElements; ++i)
			impSetIndexUnordered(i, optionalDefaultValues, elementSizeInBytes);
	}

	capacity = newCapacityInElements;
	#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
		cachedAllocationByteSize = uint64_t(newCapacityInElements) * elementSizeInBytes;
	#endif
}

void ImpByteElementArray::impReserveInElementsRoundedUnordered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements, const void *optionalDefaultValues)
{
	newCapacityInElements = getNextPow2(newCapacityInElements);
	impReserveInElementsExactUnordered(elementSizeInBytes, newCapacityInElements, optionalDefaultValues);
}

void ImpByteElementArray::impReserveInElementsExactOrdered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements)
{
	validate(elementSizeInBytes);
	if (newCapacityInElements <= getSize())
		return;

	// ToDo:
	// Can be optimised to copy over only getSize() elements, instead of getCapacity().

	impReserveInElementsExactUnordered(elementSizeInBytes, newCapacityInElements, nullptr);
}

void ImpByteElementArray::impReserveInElementsRoundedOrdered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements)
{
	validate(elementSizeInBytes);
	if (newCapacityInElements <= getSize())
		return;

	// ToDo:
	// Can be optimised to copy over only getSize() elements, instead of getCapacity().

	impReserveInElementsRoundedUnordered(elementSizeInBytes, newCapacityInElements, nullptr);
}

void ImpByteElementArray::impGrowContainerUnordered(uint32_t elementSizeInBytes, uint32_t elementAmount, const void *optionalDefaultValues)
{
	validate(elementSizeInBytes);
	fb_assert(elementSizeInBytes > 0 && elementAmount > 0);

	uint32_t newCapacityInElements = getCapacity() + elementAmount;
	newCapacityInElements = getNextPow2(newCapacityInElements);

	impReserveInElementsExactUnordered(elementSizeInBytes, newCapacityInElements, optionalDefaultValues);
}

void ImpByteElementArray::impGrowContainerOrdered(uint32_t elementSizeInBytes, uint32_t elementAmount)
{
	validate(elementSizeInBytes);
	fb_assert(elementSizeInBytes && elementAmount);

	uint32_t newCapacityInElements = getCapacity() + elementAmount;
	newCapacityInElements = getNextPow2(newCapacityInElements);

	impReserveInElementsExactOrdered(elementSizeInBytes, newCapacityInElements);
}

void ImpByteElementArray::impResizeUnordered(uint32_t elementSizeInBytes, uint32_t newSizeInElements, const void *optionalDefaultValues)
{
	validate(elementSizeInBytes);

	uint32_t currentSize = getSize();
	if (newSizeInElements <= currentSize)
	{
		size = newSizeInElements;
		return;
	}

	uint32_t currentCapacityInElements = getCapacity();
	if (newSizeInElements > currentCapacityInElements)
		impReserveInElementsExactUnordered(elementSizeInBytes, newSizeInElements, optionalDefaultValues);

	fb_assert(newSizeInElements <= getCapacity());
	size = newSizeInElements;
}

void ImpByteElementArray::impResizeOrdered(uint32_t elementSizeInBytes, uint32_t newSizeInElements, const void *optionalDefaultValues)
{
	validate(elementSizeInBytes);

	uint32_t currentSizeInElements = getSize();
	if (newSizeInElements <= currentSizeInElements)
	{
		size = newSizeInElements;
		return;
	}

	uint32_t currentCapacityInElements = getCapacity();
	if (newSizeInElements > currentCapacityInElements)
		impReserveInElementsExactOrdered(elementSizeInBytes, newSizeInElements);

	if (optionalDefaultValues && (newSizeInElements > currentSizeInElements))
	{
		bool useDefaultImp = true;

		if(elementSizeInBytes < 8u)
		{
			// Could also specialise 1/2/4/8 byte copies with actual values in them 
			uint64_t zero = 0;
			if(memcmp(optionalDefaultValues, &zero, elementSizeInBytes) == 0)
			{
				char *bytePointer = getBytePointer();
				memset(bytePointer + (currentSizeInElements * elementSizeInBytes), 0, (newSizeInElements - currentSizeInElements) * elementSizeInBytes);
				useDefaultImp = false;
			}
		}

		if(useDefaultImp)
		{
			// Initialising with unordered call as we are touching area ouside getSize()
			for (uint32_t i = currentSizeInElements; i < newSizeInElements; ++i)
				impSetIndexUnordered(i, optionalDefaultValues, elementSizeInBytes);
		}
	}

	fb_assert(newSizeInElements <= getCapacity());
	size = newSizeInElements;
}

char *ImpByteElementArray::impPushOrdered(uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);

	uint32_t currentSizeInElements = getSize();
	uint32_t currentCapacityInElements = getCapacity();
	if (currentSizeInElements == currentCapacityInElements)
		impGrowContainerOrdered(elementSizeInBytes, 1);

	size = currentSizeInElements + 1;
	return getBytePointer() + (uint64_t(currentSizeInElements) * elementSizeInBytes);
}

void ImpByteElementArray::impPushOrdered(const void *elementPointer, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);

	uint32_t currentSizeInElements = getSize();
	uint32_t currentCapacityInElements = getCapacity();
	if (currentSizeInElements == currentCapacityInElements)
		impGrowContainerOrdered(elementSizeInBytes, 1);

	memcpy(getBytePointer() + (uint64_t(currentSizeInElements) * elementSizeInBytes), elementPointer, elementSizeInBytes);
	size = currentSizeInElements + 1;
}

void ImpByteElementArray::impPrepareInsertOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	uint32_t currentSizeInElements = getSize();
	fb_assert(insertIndex <= currentSizeInElements);

	uint32_t currentCapacityInElements = getCapacity();
	if (currentSizeInElements == currentCapacityInElements)
	{
		impGrowContainerOrdered(elementSizeInBytes, 1);
		currentCapacityInElements = getCapacity();
	}

	if (insertIndex < currentSizeInElements) // Insert to middle, so move existing elements up
	{
		uint32_t moveElementAmount = currentSizeInElements - insertIndex;
		uint64_t startOffset = uint64_t(elementSizeInBytes) * insertIndex;
		fb_assert(startOffset + elementSizeInBytes + elementSizeInBytes <= uint64_t(currentCapacityInElements) * elementSizeInBytes);

		char *bytePointer = getBytePointer();
		memmove(bytePointer + startOffset + elementSizeInBytes, bytePointer + startOffset, uint64_t(moveElementAmount) * elementSizeInBytes);
	}

	size = currentSizeInElements + 1;
}

void ImpByteElementArray::impInsertOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementSizeInBytes)
{
	impPrepareInsertOrdered(insertIndex, elementPointer, elementSizeInBytes);
	impSetIndexOrdered(insertIndex, elementPointer, elementSizeInBytes);
}

void ImpByteElementArray::impInsertArrayOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementAmount, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	if (elementAmount == 0)
		return;

	fb_assert(elementPointer != nullptr);
	uint32_t currentSize = getSize();
	fb_assert(insertIndex <= currentSize);

	uint32_t currentCapacityInElements = getCapacity();
	if (currentSize + elementAmount > currentCapacityInElements)
	{
		impGrowContainerOrdered(elementSizeInBytes, currentSize + elementAmount - currentCapacityInElements);
		currentCapacityInElements = getCapacity();
	}

	if (insertIndex < currentSize) // Insert to middle, so move existing elements up
	{
		uint32_t moveElementAmount = currentSize - insertIndex + (elementAmount - 1);
		moveElementAmount = insertIndex + moveElementAmount > currentSize ? currentSize - insertIndex : moveElementAmount;

		uint64_t startOffset = uint64_t(elementSizeInBytes) * insertIndex;
		fb_assert(startOffset + elementSizeInBytes + (uint64_t(elementSizeInBytes) * elementAmount) <= uint64_t(currentCapacityInElements) * elementSizeInBytes);

		char *bytePointer = getBytePointer();
		memmove(bytePointer + startOffset + (uint64_t(elementSizeInBytes) * elementAmount), bytePointer + startOffset, uint64_t(moveElementAmount) * elementSizeInBytes);
	}

	memcpy(getBytePointer() + (uint64_t(elementSizeInBytes) * insertIndex), elementPointer, uint64_t(elementAmount) * elementSizeInBytes);
	size = currentSize + elementAmount;
}

void ImpByteElementArray::impEraseOrdered(uint32_t eraseIndex, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	uint32_t currentSize = getSize();
	fb_assert(currentSize && eraseIndex < currentSize);

	if (eraseIndex < (currentSize - 1)) // Removing from the middle, so move following elements down
	{
		uint32_t moveElementAmount = currentSize - eraseIndex - 1;
		uint64_t dstOffset = uint64_t(elementSizeInBytes) * eraseIndex;
		uint64_t srcOffset = dstOffset + elementSizeInBytes;

		char *bytePointer = getBytePointer();
		memmove(bytePointer + dstOffset, bytePointer + srcOffset, uint64_t(moveElementAmount) * elementSizeInBytes);
	}

	size = currentSize - 1;
}

void ImpByteElementArray::impEraseRangeOrdered(uint32_t eraseIndex, uint32_t eraseElementAmount, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	if (!eraseElementAmount)
		return;

	uint32_t currentSize = getSize();
	fb_assertf((currentSize && eraseElementAmount && eraseElementAmount) && ((eraseIndex + eraseElementAmount) <= currentSize), "%d + %d <= %d", eraseIndex, eraseElementAmount, currentSize);

	if (eraseIndex < (currentSize - eraseElementAmount)) // Removing from the middle, so move following elements down
	{
		uint32_t moveElementAmount = currentSize - eraseIndex - eraseElementAmount;
		uint64_t dstOffset = uint64_t(elementSizeInBytes) * eraseIndex;
		uint64_t srcOffset = uint64_t(elementSizeInBytes) * (eraseIndex + eraseElementAmount);
		
		char *bytePointer = getBytePointer();
		memmove(bytePointer + dstOffset, bytePointer + srcOffset, uint64_t(moveElementAmount) * elementSizeInBytes);
	}

	size = currentSize - eraseElementAmount;
}

void ImpByteElementArray::impSwapOutOrdered(uint32_t eraseIndex, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);

	uint32_t currentSizeInElements = getSize();
	fb_assert(eraseIndex < currentSizeInElements);

	if (eraseIndex < currentSizeInElements - 1)
	{
		char *bytePointer = getBytePointer();
		memmove(bytePointer + (uint64_t(eraseIndex) * elementSizeInBytes), bytePointer + ((currentSizeInElements - 1) * uint64_t(elementSizeInBytes)), elementSizeInBytes);
	}

	size = currentSizeInElements - 1;
}

void ImpByteElementArray::impSetIndexUnordered(uint32_t elementIndex, const void *elementPointer, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	fb_assert(elementIndex < getCapacity());
	uint64_t insertOffsetInBytes = uint64_t(elementIndex) * elementSizeInBytes;
	fb_assert(insertOffsetInBytes + elementSizeInBytes <= uint64_t(getCapacity()) * elementSizeInBytes);

	char *bytePointer = getBytePointer();
	memcpy(bytePointer + insertOffsetInBytes, elementPointer, elementSizeInBytes);
}

void ImpByteElementArray::impSetIndexOrdered(uint32_t elementIndex, const void *elementPointer, uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);

	fb_assert(elementIndex < getSize() && elementPointer && elementSizeInBytes);
	uint64_t insertOffsetInBytes = uint64_t(elementIndex) * elementSizeInBytes;
	fb_assert(insertOffsetInBytes + elementSizeInBytes <= uint64_t(getSize()) * elementSizeInBytes);

	char *bytePointer = getBytePointer();
	memcpy(bytePointer + insertOffsetInBytes, elementPointer, elementSizeInBytes);
}

void ImpByteElementArray::impTrimMemoryOrdered(uint32_t elementSizeInBytes)
{
	validate(elementSizeInBytes);
	if (isStaticAllocation())
		return;

	uint32_t currentSizeInElements = getSize();
	if (!currentSizeInElements)
	{
		impReset(elementSizeInBytes);
		return;
	}

	uint32_t currentCapacityInElements = getCapacity();
	uint32_t newCapacityInElements = getNextPow2(currentSizeInElements);

	// Make sure we only scale down. Our next pow2 could be bigger.
	if (currentCapacityInElements <= newCapacityInElements)
		return;

	fb_assert(currentCapacityInElements > newCapacityInElements);
	impReserveInElementsExactOrdered(elementSizeInBytes, newCapacityInElements);
}

FB_END_PACKAGE1()
