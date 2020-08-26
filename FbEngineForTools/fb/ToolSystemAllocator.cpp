#include "Precompiled.h"

#include "fb/memory/SystemAllocator.h"

FB_PACKAGE0()

memory::IBlockAllocator *getSystemHeapAllocator()
{
	uint32_t systemAllocationFlags = fb::memory::SystemAllocator::FlagAllocateOverInitialLimit | fb::memory::SystemAllocator::FlagThreadSafe;
	uint64_t systemAllocationInitialSize = 1024 * 1024;

	static fb::memory::SystemAllocator systemAllocator(systemAllocationInitialSize, systemAllocationFlags);
	return &systemAllocator;
}

FB_END_PACKAGE0()