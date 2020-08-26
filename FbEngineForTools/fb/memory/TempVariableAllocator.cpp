#include "Precompiled.h"
#include "TempVariableAllocator.h"

#include "Config.h"
#include "fb/lang/Alignment.h"
#include "fb/lang/AlignmentFunctions.h"
#include "fb/lang/logger/LoggingMacros.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/util/CreateTemporaryHeapString.h"


#include <string.h>
#include <stdio.h>

FB_PACKAGE1(memory)

static const int TempVariableAllocator_DefaultAlignment = lang::GeneralAlignmentFast;

void TempVariableAllocator::init()
{
	allocations = new AllocationInfo[getMaxAllocations()];
	numAllocations = 0;
	freeChunks = new AllocationInfo[getMaxFreeChunks()];
	numFreeChunks = 0;

	// Init all memory to a single free chunk
	numFreeChunks = 1;
	AllocationInfo &i = freeChunks[0];
	i.offset = 0;
	i.size = memoryBufferSize;
}

void TempVariableAllocator::removeFromArray(AllocationInfo *chunks, SizeType bestIndex, SizeType &numChunks)
{
	const void *from = &chunks[bestIndex + 1];
	void *to = &chunks[bestIndex];
	SizeType copyAmount = numChunks - bestIndex - 1;
	--numChunks;

	if (copyAmount)
		memmove(to, from, copyAmount * sizeof(AllocationInfo));
}

void TempVariableAllocator::addToArray(AllocationInfo *chunks, SizeType insertIndex, SizeType &numChunks, AllocationInfo value)
{
	const void *from = &chunks[insertIndex];
	void *to = &chunks[insertIndex + 1];
	SizeType copyAmount = numChunks - insertIndex;
	++numChunks;

	if (copyAmount)
		memmove(to, from, copyAmount * sizeof(AllocationInfo));

	chunks[insertIndex] = value;
}

void TempVariableAllocator::mergeAllocation(SizeType index)
{
	AllocationInfo ai = freeChunks[index];
	size_t offset = ai.offset;
	size_t size = ai.size;

	// How far down can we merge
	SizeType downIndex = index;
	for (SizeType i = index - 1; i < index; --i)
	{
		AllocationInfo a = freeChunks[i];
		if (a.offset + a.size != offset)
			break;

		offset = a.offset;
		size += a.size;
		downIndex = i;
	}

	// How far up can we merge
	SizeType upIndex = index;
	for (SizeType i = index + 1; i < numFreeChunks; ++i)
	{
		AllocationInfo a = freeChunks[i];
		if (offset + size != a.offset)
			break;

		size += a.size;
		upIndex = i;
	}

	// Nothing to do?
	if (downIndex == upIndex)
		return;

	// Save our merged pointer
	{
		AllocationInfo &ai2 = freeChunks[downIndex];
		ai2.offset = offset;
		ai2.size = size;
	}

	// And copy the remaining array over removed values
	const void *from = &freeChunks[upIndex + 1];
	void *to = &freeChunks[downIndex + 1];
	SizeType copyAmount = numFreeChunks - upIndex - 1;
	numFreeChunks -= upIndex - downIndex;
	if (copyAmount)
		memmove(to, from, copyAmount * sizeof(AllocationInfo));
}

void TempVariableAllocator::debug()
{
	/* Uncomment for debug */
	/*
	int biggestFree = 0;
	int freeSpace = 0;

	int previousPosition = 0;
	for (SizeType i = 0; i < numFreeChunks; ++i)
	{
		AllocationInfo &ai = freeChunks[i];
		freeSpace += (int) ai.size;
		if ((int) ai.size > biggestFree)
			biggestFree = (int) ai.size;

		fb_assert((int) ai.offset >= previousPosition);
		previousPosition = (int) (ai.offset + ai.size);
	}

	previousPosition = 0;
	for (SizeType i = 0; i < numAllocations; ++i)
	{
		AllocationInfo &ai = allocations[i];
		fb_assert((int) ai.offset >= previousPosition);
		previousPosition = (int) (ai.offset + ai.size);
	}
	FB_PRINTF("Allocator stas - %d allocations, %d free chunks, %d KB biggest free chunk, %d KB space left\r\n", a, f, biggestFree/1024, freeSpace/1024);
	*/
}

SizeType TempVariableAllocator::getMaxAllocations() const
{
	return maxAllocations;
}

SizeType TempVariableAllocator::getMaxFreeChunks() const
{
	return maxAllocations * 2;
}

SizeType TempVariableAllocator::getAllocationIndex(char *ptr) const
{
	fb_expensive_assert(doesOwnPointer(ptr));
	SizeType offset = SizeType(ptr - memoryBuffer);

	if (!numAllocations)
		return 0xFFFFFFFF;

	// Binary seach for index
	SizeType minRange = 0;
	SizeType maxRange = numAllocations;
	SizeType tryIndex = maxRange / 2;
	for (;;)
	{
		AllocationInfo ai = allocations[tryIndex];
		if (ai.offset == offset)
			return tryIndex;

		if (offset < ai.offset)
		{
			maxRange = tryIndex;
			tryIndex = (maxRange + minRange) / 2;
		}
		else
		{
			minRange = tryIndex + 1;
			tryIndex = (maxRange + minRange) / 2;
		}
	}
}

SizeType TempVariableAllocator::getIndex(AllocationInfo *chunks, SizeType allocationsParam, SizeType offset) const
{
	if (!allocationsParam)
		return 0;

	// Binary seach for index
	SizeType minRange = 0;
	SizeType maxRange = allocationsParam;
	SizeType tryIndex = maxRange / 2;
	for (;;)
	{
		AllocationInfo ai = chunks[tryIndex];
		//if (ai.offset == offset)
		//	return tryIndex;
		
		if (offset < ai.offset)
		{
			// We can't go lower, so the right index is the first
			if (tryIndex == 0)
				return tryIndex;

			maxRange = tryIndex;
			tryIndex = (maxRange + minRange) / 2;
		}
		else
		{
			// We can't go higher, so the right index is the last
			if (tryIndex == allocationsParam - 1)
				return allocationsParam;

			// We are bigger than current, if we are also lower than next
			if (offset < chunks[tryIndex + 1].offset)
				return tryIndex + 1; // Match!

			// Carry on
			minRange = tryIndex + 1;
			tryIndex = (maxRange + minRange) / 2;
		}
	}
}

FB_STATIC_CONST_STRING(undefinedStr, "undefined");

TempVariableAllocator::TempVariableAllocator(SizeType bufferSize, SizeType maxAllocations)
	: maxAllocations(maxAllocations)
	, idString(undefinedStr)
{
	memoryBuffer = new char[bufferSize];
	memoryBufferSize = bufferSize;
	freeMemoryBuffer = true;

	init();
}

TempVariableAllocator::TempVariableAllocator(void *buffer, SizeType bufferSize, SizeType maxAllocations)
	: maxAllocations(maxAllocations)
	, idString(undefinedStr)
{
	memoryBuffer = (char*) buffer;
	memoryBufferSize = bufferSize;
	freeMemoryBuffer = false;

	init();
}

TempVariableAllocator::~TempVariableAllocator()
{
	if (freeMemoryBuffer)
		delete[] memoryBuffer;
	delete[] allocations;
	delete[] freeChunks;
}

bool TempVariableAllocator::doesOwnPointer(char *ptr) const
{
	if ((ptr >= memoryBuffer) && (ptr < (memoryBuffer + memoryBufferSize)))
		return true;

	return false;
}

char *TempVariableAllocator::allocate(SizeType size)
{
	// Make sure we stay within alignment limits
	size = (SizeType) lang::alignValue(size, TempVariableAllocator_DefaultAlignment);

	fb_assert(numAllocations < getMaxAllocations());
	if (numAllocations < getMaxAllocations())
	{
		// We need to find the best match from free list (least wasted space)

		SizeType bestIndex = 0xFFFFFFFF;
		SizeType bestSize = 0;
		for (SizeType i = 0; i < numFreeChunks; ++i)
		{
			AllocationInfo &ai = freeChunks[i];
			if ((int)ai.size < size) // Too small
				continue;
			
			// Store, if smaller

			if ((SizeType)ai.size < bestSize || bestSize == 0)
			{
				bestIndex = i;
				bestSize = (SizeType) ai.size;
			}
		}

		if (bestIndex != 0xFFFFFFFF)
		{
			AllocationInfo &ai = freeChunks[bestIndex];
			char *resultPointer = memoryBuffer + ai.offset;

			//FB_PRINTF("Allocating %d bytes to offset %u\r\n", size, ai.offset);

			{
				// Add allocation to sorted list
				AllocationInfo ai2;
				ai2.offset = ai.offset;
				ai2.size = size;

				SizeType insertIndex = getIndex(allocations, numAllocations, (SizeType)ai2.offset);
				addToArray(allocations, insertIndex, numAllocations, ai2);
			}

			// Modify remaining free size
			ai.offset += size;
			ai.size -= size;

			// Remove, if necessary
			if (ai.size == 0)
				removeFromArray(freeChunks, bestIndex, numFreeChunks);

			debug();
			return resultPointer;
		}
	}

	if (warnOnOutOfSpace)
	{
		// Get stats and dump info
		FB_LOG_WARNING(FB_MSG("TempVariableAllocator ", idString, " ran out of space! Stats follow ->"));
		printStats();
	}

	return 0;
}

void TempVariableAllocator::deallocate(char *ptr)
{
	// This is fine
	if (!ptr)
		return;

	fb_assert(doesOwnPointer(ptr));
	int offset = (int) (ptr - memoryBuffer);
	SizeType allocationIndex = getAllocationIndex(ptr);

	if (allocationIndex != 0xFFFFFFFF)
	{
		AllocationInfo ai = allocations[allocationIndex];

		// Remove found allocation (and keep info on stack)
		removeFromArray(allocations, allocationIndex, numAllocations);

		// First check if we can merge new free space to existing free chunk (defrag)
		// ToDo: We could do a binary search for the position here
		SizeType previousIndex = numFreeChunks - 1;
		SizeType listIndex = getIndex(freeChunks, numFreeChunks, (SizeType)ai.offset);
		int minIndex = (int)listIndex - 2;
		int maxIndex = (int)listIndex + 2;
		if (minIndex < 0)
			minIndex = 0;
		if (maxIndex >= (int)numFreeChunks)
			maxIndex = (int)numFreeChunks;
		
		// We need to do a binary search to free list as well !

		//for (int j = 0; j < numFreeChunks; ++j)
		for (SizeType j = (SizeType)minIndex; j < (SizeType)maxIndex; ++j)
		{
			AllocationInfo &ai2 = freeChunks[j];
			size_t endPosition = ai2.offset + ai2.size;

			// Match, pointer right before current free chunk
			if (ai2.offset == ai.offset + ai.size)
			{
				ai2.offset -= ai.size;
				ai2.size += ai.size;

				mergeAllocation(j);
				debug();
				return;
			}
			// Match, pointer right after current free chunk
			else if (endPosition == ai.offset)
			{
				ai2.size += ai.size;

				mergeAllocation(j);
				debug();
				return;
			}
			// No match, we went past the wanted offset
			else if ((int) endPosition > offset)
			{
				previousIndex = j - 1;
				break;
			}
		}

		// Create a new free list entry
		fb_assert(numFreeChunks < getMaxFreeChunks());
		SizeType insertIndex = previousIndex + 1;
		addToArray(freeChunks, insertIndex, numFreeChunks, ai);

		debug();
		return;
	}

	fb_assert(0 && !"TempVariableAllocator - deallocated pointer not found from the pool.");
}

char *TempVariableAllocator::reallocate(char *ptr, SizeType size)
{
	if (!ptr || !size)
		return allocate(size);

	fb_assert(doesOwnPointer(ptr));
	SizeType allocationIndex = getAllocationIndex(ptr);
	if (allocationIndex != 0xFFFFFFFF)
	{
		// Slow !
		SizeType oldSize = (SizeType) allocations[allocationIndex].size;

		char *newPtr = allocate(size);
		SizeType copySize = oldSize;
		if (size < copySize)
			copySize = size;

		memcpy(newPtr, ptr, copySize);
		deallocate(ptr);
		return newPtr;
	}

	fb_assert(!"Something wrong here ..");
	return allocate(size);
}

void TempVariableAllocator::setIdString(const DynamicString &newIdString)
{
	newIdString.convertToStatic();
	this->idString = newIdString;
}

void TempVariableAllocator::setWarnOnOutOfSpace(bool value)
{
	warnOnOutOfSpace = value;
}

void TempVariableAllocator::printStats()
{
	#if FB_BUILD != FB_FINAL_RELEASE
		SizeType previousPosition = 0;
		SizeType freeSpace = 0;
		SizeType biggestFree = 0;
		for (SizeType i = 0; i < numFreeChunks; ++i)
		{
			AllocationInfo &ai = freeChunks[i];
			freeSpace += (SizeType) ai.size;
			if ( ai.size > biggestFree)
				biggestFree = (SizeType) ai.size;

			fb_assert(ai.offset >= previousPosition);
			previousPosition = (SizeType) (ai.offset + ai.size);
		}

		FB_LOG_INFO(FB_FMT("Stats for %s allocator - %d allocations, %d MB space used, %d KB biggest free chunk", idString.getPointer(), numAllocations, freeSpace, biggestFree));
	#endif
}
FB_END_PACKAGE1()
