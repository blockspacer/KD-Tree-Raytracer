#pragma once

#include <stddef.h>
#include "Alignment.h"
#include "fb/container/ArraySlice.h"

FB_PACKAGE1(lang)

/// Low level component for packing several allocations into one (to reduce fragmentation). 
/// Configuration before allocate() has to match to pointers queried after, including the allocation order!
/// Internal pointer is destroyed with class unless dismiss()'ed.
class CombinedAllocation
{
public:
	CombinedAllocation();
	~CombinedAllocation();

	/// Configure needed pointers with this before allocate()
	template<typename T>
	void addPointer(SizeType amount = 1, size_t alignment = DefaultAlignment);
	/// Configure needed pointers with this before allocate()
	template<typename T>
	void addArray(SizeType arraySize, size_t alignment = DefaultAlignment);

	/// Configure needed points first!
	void allocate();
	/// Use given buffer as allocation. Remember to Dismiss the pointer afterwards if you don't want it to get delete[]'d!
	void allocate(char *buffer, size_t bufferSize, size_t alignment = DefaultAlignment);

	/// Get next pointer from already allocate()'d internal data.
	template<typename T>
	T *getNextPointer(size_t alignment = DefaultAlignment);
	/// Get next pointer from already allocate()'d internal data, no default constructor is called. 
	/// Useful if the type doesn't have default constructor and you know what is placement new.
	template<typename T>
	T *getNextPointerNoDefaultConstructor(size_t alignment = DefaultAlignment);

	/// Get next array pointer from already allocate()'d internal data.
	template<typename T>
	T *getNextArray(SizeType arraySize, size_t alignment = DefaultAlignment);
	/// Get next array pointer from already allocate()'d internal data, no default constructors are called. 
	/// Useful if the type doesn't have default constructor and you know what is placement new.
	template<typename T>
	T *getNextArrayNoDefaultConstructor(SizeType arraySize, size_t alignment = DefaultAlignment);

	/// Free pointer (call destructor).  Optional call if you know what you are doing(tm).
	template<typename T>
	void freePointer(T *pointer);

	/// Free array (call destructors). Optional call if you know what you are doing(tm).
	template<typename T>
	void freeArray(T *pointer, SizeType arraySize);

	/// Resets internal pointer so that it won't get destroyed with class. It is users responsibility to delete[] returned pointer.
	char *dismiss();
	void reset();

	// Utility functions for getting ArraySlices
	template<typename T>
	ArraySlice<T> getNextSlice(SizeType arraySize, size_t alignment = DefaultAlignment)
	{
		return sliceFromPointer(getNextArray<T>(arraySize, alignment), arraySize);
	}
	template<typename T>
	ArraySlice<T> getNextSliceNoDefaultConstructor(SizeType arraySize, size_t alignment = DefaultAlignment)
	{
		return sliceFromPointer(getNextArrayNoDefaultConstructor<T>(arraySize, alignment), arraySize);
	}

	size_t getRequestedMemory() const { return requestedSize; }
	size_t getMemoryToAllocate() const { return requestedSize + maxRequestedAlignment; }
	size_t getReservedMemory() const { return requestedSize; }

	const char *getData() const { return pointer; }

private:
	// Not defined
	CombinedAllocation(const CombinedAllocation &other);
	void operator = (const CombinedAllocation &other);

	void addPointerImp(size_t size, SizeType amount, size_t alignment);
	void addArrayImp(size_t size, SizeType amount, size_t alignment);
	char *getPointerImp(size_t size, size_t alignment);
	char *getArrayImp(size_t size, SizeType amount, size_t alignment);

	void updateAlignmentOffset();

	char *pointer = nullptr;
	size_t requestedSize = 0;
	size_t maxRequestedAlignment = 0;
	size_t alignmentOffset = 0;
	size_t queriedSize = 0;
};

#include "CombinedAllocationInline.h"

FB_END_PACKAGE1()
