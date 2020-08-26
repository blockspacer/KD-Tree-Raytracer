#include "Precompiled.h"
#include "CombinedAllocation.h"
#include "AlignmentFunctions.h"

FB_PACKAGE1(lang)

CombinedAllocation::CombinedAllocation()
{
}

CombinedAllocation::~CombinedAllocation()
{
	delete[] pointer;
}

void CombinedAllocation::allocate()
{
	fb_assert(requestedSize);
	fb_assert(!queriedSize);

	pointer = new char[getMemoryToAllocate()];

	updateAlignmentOffset();
}

void CombinedAllocation::allocate(char *buffer, size_t bufferSize, size_t alignment)
{
	maxRequestedAlignment = lang::max(maxRequestedAlignment, alignment);
	requestedSize = bufferSize;
	fb_assert(!queriedSize);

	pointer = buffer;

	updateAlignmentOffset();
}

void CombinedAllocation::updateAlignmentOffset()
{
	uintptr_t alignedPointer = alignValue(uintptr_t(pointer), uintptr_t(maxRequestedAlignment));
	alignmentOffset = alignedPointer - uintptr_t(pointer);
}

char *CombinedAllocation::dismiss()
{
	char *tmp = pointer;
	pointer = nullptr;

	return tmp;
}

void CombinedAllocation::reset()
{
	delete[] pointer;
	pointer = nullptr;
	requestedSize = 0;
	queriedSize = 0;
	maxRequestedAlignment = 0;
	alignmentOffset = 0;
}

void CombinedAllocation::addPointerImp(size_t size, SizeType amount, size_t alignment)
{
	fb_assert(!queriedSize);

	maxRequestedAlignment = lang::max(maxRequestedAlignment, alignment);
	requestedSize = alignValue(requestedSize, alignment);
	requestedSize += size * amount;
}

void CombinedAllocation::addArrayImp(size_t size, SizeType amount, size_t alignment)
{
	fb_assert(!queriedSize);

	maxRequestedAlignment = lang::max(maxRequestedAlignment, alignment);
	requestedSize = alignValue(requestedSize, alignment);
	requestedSize += size * amount;
}

char *CombinedAllocation::getPointerImp(size_t size, size_t alignment)
{
	fb_assert(alignment <= maxRequestedAlignment);
	queriedSize = alignValue(queriedSize, alignment);

	char *result = &pointer[alignmentOffset + queriedSize];
	queriedSize += size;

	fb_assert(alignment == 0 || (uintptr_t(result) % alignment) == 0);
	fb_assert(queriedSize <= requestedSize);
	return result;
}

char *CombinedAllocation::getArrayImp(size_t size, SizeType amount, size_t alignment)
{
	fb_assert(alignment <= maxRequestedAlignment);
	queriedSize = alignValue(queriedSize, alignment);

	char *result = &pointer[alignmentOffset + queriedSize];
	queriedSize += size * amount;

	fb_assert(alignment == 0 || (uintptr_t(result) % alignment) == 0);
	fb_assert(queriedSize <= requestedSize);
	return result;
}

FB_END_PACKAGE1()
