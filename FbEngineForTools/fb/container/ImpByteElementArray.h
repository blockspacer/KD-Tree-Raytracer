#pragma once

#include "fb/lang/FBAssert.h"
#include "fb/lang/Types.h"
#include "fb/lang/platform/ForceInline.h"

#ifndef FB_VECTOR_ASSERT
#define FB_VECTOR_ASSERT(...) fb_assert(__VA_ARGS__)
#endif

#ifndef FB_VECTOR_ASSERTF
#define FB_VECTOR_ASSERTF(p_pred,p_fmt,...) fb_assertf(!!(p_pred), p_fmt, ## __VA_ARGS__)
#endif

FB_PACKAGE1(container)

#define FB_IMPBYTELEMENTARRAY_REGRESSION_MODE FB_FALSE

// Very simple byte element array which offers some basic functionality.
// Use as non-template base class. 
// Things like element size is not stored here for size reasons, deriving template can give that as parameter.
// Does not enforce any safety, but provides Ordered/Unordered versions of many functions. Ordered assumes that all elements are linearly packed to the start of buffer.
// Allows somewhat faster copies, and more extensive asserts without copy-pasting them to every possible container.
struct ImpByteElementArray
{
protected:
	static const uintptr_t PointerMask = sizeof(char*) == 8 ? 0xfffffffffffffffc : 0xfffffffc;
	static const uintptr_t StaticPointerBit = 1;
	static const uintptr_t ReserveAssertPointerBit = 2;
	static const uintptr_t PointerDataMask = StaticPointerBit | ReserveAssertPointerBit;

	// If StaticPointerBit has been set, assumes pointer is in user allocated space and should not be freed
	uintptr_t intPointer = 0;
	uint32_t size = 0;
	uint32_t capacity = 0;

	ImpByteElementArray(char *staticPointer, uint32_t staticSizeInElements, bool assertOnReserve);

	void *impGetNewPointer(uint32_t elementSizeInBytes, uint32_t &elementAmount);
	void *impGetNewPointerExact(uint32_t elementSizeInBytes, uint32_t &elementAmount);
	void impSwapPointer(uint32_t elementSizeInBytes, void *pointer, uint32_t newCapacity);
	void impSwapToStaticPointer(uint32_t elementSizeInBytes, void *pointer, uint32_t pointerSizeInBytes, bool assertOnReserve);

	// Note: Deriving class has to call this in destructor, as plain destructor doesn't have enough information for it.
	void impReset(uint32_t elementSizeInBytes);

	#if FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_TRUE
		mutable uint32_t cachedElementSize = 0;
		mutable uint64_t cachedAllocationByteSize = 0;
		void validate(uint32_t elementSizeInBytes) const;
	#elif FB_IMPBYTELEMENTARRAY_REGRESSION_MODE == FB_FALSE
		void validate(uint32_t elementSizeInBytes) const {}
	#endif

	ImpByteElementArray();
	~ImpByteElementArray();

	static uint32_t impGetNextPow2(uint32_t size);

public:
	static SizeType getMaxCapacity() { return 0xffffffff; }
	typedef fb::SizeType SizeType;

	FB_FORCEINLINE bool isStaticAllocation() const { return (intPointer & StaticPointerBit) != 0; }
	FB_FORCEINLINE bool shouldAssertOnReserve() const { return (intPointer & ReserveAssertPointerBit) != 0; }
	FB_FORCEINLINE bool isEmpty() const { return size == 0; }
	FB_FORCEINLINE uint32_t getIndexCount() const { return size; }
	FB_FORCEINLINE uint32_t getElementCount() const { return size; }
	FB_FORCEINLINE uint32_t getSize() const { return size; }
	FB_FORCEINLINE uint32_t getCapacity() const { return capacity; }
	FB_FORCEINLINE const char *getBytePointer() const { return (const char*) (intPointer & PointerMask); }
	FB_FORCEINLINE char *getBytePointer() { return (char*) (intPointer & PointerMask); }

	// Never decreases capacity (or resets any data)
	void clear()
	{
		size = 0;
	}

	// Resets internal pointer and leaks the memory.  Use during static destruction or somesuch.
	void leakMemory()
	{
		intPointer = 0;
		size = 0;
		capacity = 0;
	}

protected:
	void impSwapOrdered(ImpByteElementArray &other, uint32_t elementSizeInBytes);
	void impMoveOrdered(ImpByteElementArray &other, uint32_t elementSizeInBytes);
	void impPointerSwap(ImpByteElementArray &other);
	void impZeroPointer();

	// Convert static pointer to dynamic one
	void impConvertToDynamic(uint32_t elementSizeInBytes);

	// Create copy using other.getCapacity() elements (as we can't assume all elements are in order)
	void impCopyFromUnordered(const ImpByteElementArray &other, uint32_t elementSizeInBytes, const void *optionalDefaultValues);
	// Create copy using other.getSize() elements 
	void impCopyFromOrdered(const ImpByteElementArray &other, uint32_t elementSizeInBytes);

	// Change getCapacity() to newCapacityInElements elements. For unordered case, optionally initialise memory to given values.
	void impReserveInElementsExactUnordered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements, const void *optionalDefaultValues);
	void impReserveInElementsRoundedUnordered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements, const void *optionalDefaultValues);
	void impReserveInElementsExactOrdered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements);
	void impReserveInElementsRoundedOrdered(uint32_t elementSizeInBytes, uint32_t newCapacityInElements);

	// Helper for growing container. Reserves space for at least elementAmount elements (doubling the size using pow2, most likely).
	void impGrowContainerUnordered(uint32_t elementSizeInBytes, uint32_t elementAmount, const void *optionalDefaultValues);
	void impGrowContainerOrdered(uint32_t elementSizeInBytes, uint32_t elementAmount);

	// Resize to wanted new size. Increases capacity as needed, does not free memory.
	// Init new space (from old capacity to new capacity) with optionalDefaultValues if pointer present, otherwise leave memory uninitialised.
	void impResizeUnordered(uint32_t elementSizeInBytes, uint32_t newSizeInElements, const void *optionalDefaultValues);
	// Resize to wanted new size. Increases capacity as needed, does not free memory.
	// Init new space (from old size to new size) with optionalDefaultValues if pointer present, otherwise leave memory uninitialised.
	void impResizeOrdered(uint32_t elementSizeInBytes, uint32_t newSizeInElements, const void *optionalDefaultValues);

	// Increases size, increases capacity if needed.
	char *impPushOrdered(uint32_t elementSizeInBytes);
	void impPushOrdered(const void *elementPointer, uint32_t elementSizeInBytes);

	// Prepare array for inserting element to given index (in case we want to inline the copying)
	// insertIndex has to be <= getSize()
	// Increases size, increases capacity if needed.
	void impPrepareInsertOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementSizeInBytes);

	// Insert element to given index. If insertIndex < getSize(), move existing data up to make space.
	// insertIndex has to be <= getSize()
	// Increases size, increases capacity if needed.
	void impInsertOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementSizeInBytes);

	// Vector-style insert element to given index. If insertIndex < getSize(), move existing data up to make space.
	// insertIndex has to be <= getSize()
	// Increases size, increases capacity if needed.
	void impInsertArrayOrdered(uint32_t insertIndex, const void *elementPointer, uint32_t elementAmount, uint32_t elementSizeInBytes);

	// Vector-style erase which removes given index. If eraseIndex < (getSize() - 1), move existing data down one element.
	// eraseIndex has to be < getSize()
	// Decreases size, never decreases capacity.
	void impEraseOrdered(uint32_t eraseIndex, uint32_t elementSizeInBytes);

	// Erase which removes given index range. If eraseIndex < (getSize() - eraseElementAmount), move existing data down.
	// eraseIndex has to be < getSize()
	// eraseIndex+eraseElementAmount has to be <= getSize()
	// Decreases size, never decreases capacity.
	void impEraseRangeOrdered(uint32_t eraseIndex, uint32_t eraseElementAmount, uint32_t elementSizeInBytes);

	void impSwapOutOrdered(uint32_t eraseIndex, uint32_t elementSizeInBytes);

	// Raw set. Does NOT increase size.
	void impSetIndexUnordered(uint32_t elementIndex, const void *elementPointer, uint32_t elementSizeInBytes);
	void impSetIndexOrdered(uint32_t elementIndex, const void *elementPointer, uint32_t elementSizeInBytes);

	// Trim capacity to >= getSize().
	// If size is zero, free the memory completely (assuming it's not a static pointer)
	void impTrimMemoryOrdered(uint32_t elementSizeInBytes);

private:
	// Not defined, don't copy by value (using these). See explicit copyFrom.
	ImpByteElementArray(const ImpByteElementArray &);
	void operator = (const ImpByteElementArray &);
};

FB_END_PACKAGE1()
