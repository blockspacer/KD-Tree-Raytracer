#include "Precompiled.h"
#include "AtomicLinearAllocator.h"

#include "IBlockAllocator.h"
#include "fb/lang/AlignmentFunctions.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/FBStaticAssert.h"

FB_PACKAGE1(memory)

AtomicLinearAllocator::AtomicLinearAllocator(IBlockAllocator *allocator, SizeType blockSizeInBytes_, uint32_t initialBlockAmount, bool expandDynamically_)
:	blockAllocator(allocator)
,	blockSizeInBytes(blockSizeInBytes_)
,	expandDynamically(true)
{
	atomicStoreRelaxed(currentBuffer, NULL);

	if (initialBlockAmount)
	{
		for (uint32_t i = 0; i < initialBlockAmount; ++i)
			allocateNewBlockImp();
	}

	// Delay this to first allocate wanted amount of blocks
	expandDynamically = expandDynamically_;
}

AtomicLinearAllocator::~AtomicLinearAllocator()
{
	Buffer *current = firstBuffer;
	while (current)
	{
		Buffer *next = (Buffer *) atomicLoadRelaxed(current->nextBuffer);
		if (blockAllocator)
			blockAllocator->deallocate(current, blockSizeInBytes);
		else
			lang::freeMemory(current);

		current = next;
	}
}

void AtomicLinearAllocator::reset()
{
	if (firstBuffer)
		atomicStoreRelaxed(firstBuffer->positionInBytes, 0);

	atomicStoreRelease(currentBuffer, firstBuffer);
}

void *AtomicLinearAllocator::allocate(uint64_t size64)
{
	fb_assert(size64 + OverheadInBytes < blockSizeInBytes);
	fb_static_assert(sizeof(blockSizeInBytes) == 4 && "blockSizeInBytes may be more than 32 bits can handle");
	uint32_t size = uint32_t(size64);
	for (;;)
	{
		Buffer *current = (Buffer*) atomicLoadAcquire(currentBuffer);
		if (!current)
		{
			mutex.enter();
			
			// Double-check, as another thread could have done this before we reach mutex
			if (!firstBuffer)
				getNextBlockImp();

			mutex.leave();
		}
		else
		{
			uint32_t previousOffset = atomicAddRelaxed(current->positionInBytes, size);
			if (previousOffset + size + OverheadInBytes > blockSizeInBytes)
			{
				mutex.enter();
			
				// Double-check, as another thread could have done this before we reach mutex
				previousOffset = atomicLoadRelaxed(current->positionInBytes);
				if (previousOffset + size + OverheadInBytes > blockSizeInBytes)
					getNextBlockImp();

				mutex.leave();
			}
			else
			{
				// All good
				return current->pointer + previousOffset;
			}
		}
	}
}

void AtomicLinearAllocator::deallocate(void *, uint64_t)
{
	// NOP
}

void *AtomicLinearAllocator::allocateAligned(uint64_t size, SizeType alignment)
{
	uintptr_t ptr = (uintptr_t) allocate(size + alignment);
	ptr = lang::alignValue(ptr, alignment);

	return (void*) ptr;
}

// Assumes we are protected by mutex by caller as needed
void AtomicLinearAllocator::getNextBlockImp()
{
	Buffer *current = (Buffer*) atomicLoadRelaxed(currentBuffer);
	if (!current)
	{
		allocateNewBlockImp();
		current = (Buffer*) atomicLoadRelaxed(currentBuffer);
	}
	else
	{
		Buffer *next = (Buffer*) atomicLoadRelaxed(current->nextBuffer);
		if (!next)
		{
			allocateNewBlockImp();
			next = (Buffer*) atomicLoadRelaxed(current->nextBuffer);
		}

		current = next;
	}

	fb_assert(current);
	atomicStoreRelaxed(current->positionInBytes, 0);
	atomicStoreRelease(currentBuffer, current);
}

// Assumes we are protected by mutex by caller as needed
void AtomicLinearAllocator::allocateNewBlockImp()
{
	fb_assert(expandDynamically);
	totalMemoryAllocatedInBytes += blockSizeInBytes;

	char *pointer = NULL;
	if (blockAllocator)
		pointer = (char*) blockAllocator->allocate(blockSizeInBytes);
	else
		pointer = (char*) lang::allocateMemory(blockSizeInBytes);

	Buffer *newBuffer = (Buffer*) pointer;
	atomicStoreRelaxed(newBuffer->nextBuffer, NULL);
	atomicStoreRelaxed(newBuffer->positionInBytes, 0);
	newBuffer->pointer = pointer + OverheadInBytes;

	if (!firstBuffer)
	{
		firstBuffer = newBuffer;
		atomicStoreRelease(currentBuffer, newBuffer);
	}
	else
	{
		Buffer *current = (Buffer*) atomicLoadRelaxed(currentBuffer);
		fb_assert(current);
		Buffer *currentNext = (Buffer*) atomicLoadRelaxed(current->nextBuffer);

		// Insert as next buffer. currentBuffer should always be present if we are not empty.
		atomicStoreRelaxed(newBuffer->nextBuffer, currentNext);
		atomicStoreRelaxed(current->nextBuffer, newBuffer);
	}
}

FB_END_PACKAGE1()
