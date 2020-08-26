#include "Precompiled.h"
#include "Config.h"
#include "TempAllocator.h"

#include "fb/lang/Alignment.h"
#include "fb/lang/AlignmentFunctions.h"

FB_PACKAGE1(memory)

static const SizeType DefaultAlignment = lang::GeneralAlignmentFast;

void TempAllocator::defrag()
{
	for (SizeType i = 1; i < MaxAllocations ; ++i)
	{
		// Move all inactive (size 0) pointers closer to the right place
		// (being right after the previous allocation)
		if (allocationSize[i] == 0)
		{
			allocationOffset[i] = allocationOffset[i - 1] + allocationSize[i - 1];

			// However, those can never be further than 
		}
	}

	// Regression
	/*
	for (int i = 0; i < MaxAllocations - 1; ++i)
	{
		int previousEnd = allocationOffset[i] + allocationSize[i];

		for (int j = i + 1; j < MaxAllocations; ++j)
		{
			if (allocationOffset[j] < previousEnd)
			{
				abort();
			}
		}
	}
	*/
}

TempAllocator::TempAllocator(char *buffer, SizeType bufferSize)
:	memoryBuffer(buffer)
,	memoryBufferSize(bufferSize)
,	freeMemoryBuffer(false)
{
	for (SizeType i = 0; i < MaxAllocations; ++i)
	{
		allocationOffset[i] = 0;
		allocationSize[i] = 0;
	}
}

TempAllocator::TempAllocator(SizeType bufferSize)
:	memoryBuffer(0)
,	memoryBufferSize(0)
,	freeMemoryBuffer(true)
{
	memoryBuffer = new char[bufferSize];
	memoryBufferSize = bufferSize;

	for (SizeType i = 0; i < MaxAllocations; ++i)
	{
		allocationOffset[i] = 0;
		allocationSize[i] = 0;
	}
}

TempAllocator::~TempAllocator()
{
	if (freeMemoryBuffer)
		delete[] memoryBuffer;
}

char *TempAllocator::allocate(SizeType size)
{
	// Not really standard allocator conforming but oh well
	fb_assert(size);
	// Make sure we stay within alignment limits
	size = (SizeType) lang::alignValue(size, DefaultAlignment);

	// Find a free slot - we don't even try to be a smart about it

	for (SizeType i = 0; i < MaxAllocations; ++i)
	{
		if (allocationSize[i] != 0)
			continue;

		SizeType currentOffset = allocationOffset[i];
		SizeType possibleSpace = 0xFFFFFFFF;

		// Distance to next reserved pointer
		for (SizeType j = i + 1; j < MaxAllocations; ++j)
		{
			if (allocationSize[j])
			{
				possibleSpace = allocationOffset[j] - currentOffset;
				break;
			}
		}

		// No hits, so all the way to the end
		if (possibleSpace == 0xFFFFFFFF)
			possibleSpace = memoryBufferSize - allocationOffset[i];

		if (possibleSpace >= size)
		{
			allocationSize[i] = size;
			defrag();

			return &memoryBuffer[currentOffset];
		}
	}

	// No time to deal with these atm
	//fb_assert(0 && !"TempAllocator ran out of space, or got too fragmented");
	return 0;
}

void TempAllocator::deallocate(char *ptr)
{
	// This is fine
	if (!ptr)
		return;
	if (ptr < memoryBuffer || ptr >= memoryBuffer + memoryBufferSize)
		return;

	SizeType offset = (SizeType) (ptr - memoryBuffer);
	fb_assert(offset < memoryBufferSize);

	for (SizeType i = 0; i < MaxAllocations; ++i)
	{
		if (allocationOffset[i] == offset)
		{
			allocationSize[i] = 0;
			defrag();

			return;
		}
	}
}

FB_END_PACKAGE1()
