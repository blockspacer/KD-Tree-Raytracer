#include "Precompiled.h"
#include "SystemAllocator.h"

#include "fb/lang/AlignmentFunctions.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/MemoryOperatorsConfig.h"
#include "fb/memory/LargePageAllocator.h"

FB_PACKAGE1(memory)

enum { SystemAllocatorBlockSize = 2 * 1024 * 1024 };

void *SystemAllocator::allocateImp(uint64_t size)
{
	atomicAddRelaxed(totalSizeAllocated, size);

#if FB_USE_LARGE_PAGES == FB_TRUE
	void *buffer = memory::LargePageAllocator::getInstance().allocate(size);
	if (buffer != nullptr)
		return buffer;
#endif

	return lang::osAllocate((size_t)size);
}

SystemAllocator::SystemAllocator(uint64_t initialMemoryAmount, uint32_t flags)
	: flags(flags)
{
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
	initialMemoryAmount = SystemAllocatorBlockSize;
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE
	initialMemoryAmount = FB_ALIGN_VALUE(initialMemoryAmount, SystemAllocatorBlockSize);
#endif

	cachedPointer = (uint8_t *) allocateImp(initialMemoryAmount);
	if (cachedPointer)
		cachedTotalMemoryAmount = initialMemoryAmount;
}

SystemAllocator::~SystemAllocator()
{
	// NOP
}

SizeType SystemAllocator::getBlockSize() const
{
	return SystemAllocatorBlockSize;
}

void *SystemAllocator::allocate(uint64_t size)
{
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
	atomicAddRelaxed(totalSizeAllocated, size);
	return lang::allocateMemory(size);
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE
	size = FB_ALIGN_VALUE(size, SystemAllocatorBlockSize);
	MutexGuard guard(mutex);

	if (cachedTotalMemoryAmount - cachedUsedMemoryAmount >= size)
	{
		void *result = cachedPointer + cachedUsedMemoryAmount;
		cachedUsedMemoryAmount += size;
		return result;
	}

	if (flags & FlagAllocateOverInitialLimit)
	{
		void *result = allocateImp(size);
		return result;
	}
	return nullptr;
#endif
}

void SystemAllocator::deallocate(void *pointer, uint64_t size)
{
	//fb_assert(0 && !"Deallocation not supported. Memory should go to static pools only.");
}

uint64_t SystemAllocator::getCurrentAllocationBytes() const
{
	return atomicLoadRelaxed(totalSizeAllocated);
}

FB_END_PACKAGE1()
