#include "Precompiled.h"
#include "GlobalFixedAllocator.h"

#include "fb/lang/GlobalFixedAllocateFunctions.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/MemoryOperatorsConfig.h"
#include "fb/memory/HeapAllocator.h"
#include "fb/memory/IBlockAllocator.h"
#include "fb/memory/LargePageAllocator.h"
#include "fb/memory/MultiSizePool.h"
#include "fb/memory/SystemAllocator.h"
#include "fb/profiling/AllocationProfiler.h"
#include "fb/profiling/ZoneProfiler.h"
#include "fb/string/HeapString.h"

FB_PACKAGE0()
	typedef memory::MultiSizePool PoolType;
FB_END_PACKAGE0()

FB_PACKAGE2(memory, fixedallocator)

	namespace {
		memory::MultiSizePool *multiPool = NULL;
		static const uint32_t MagicInitNumber = 0xbabe2bed;
		static const uint32_t MagicDestructNumber = 0xfade2bed;
		static uint32_t initCheckValue = 0;

		// To detect problems with single thread allocators
		typedef DWORD ThreadHandle;
		ThreadHandle getCurrentThreadHandle() { return GetCurrentThreadId(); }
		// According to MSDN, "most likely" a valid assumption. Worst case is ignored single thread check.
		#define FB_NO_THREAD_VALUE 0
		static ThreadHandle mainThreadHandle = FB_NO_THREAD_VALUE;

		void initImp()
		{
			FB_ZONE_START();

			// Assuming that the first entry will be from main core
			if (initCheckValue != MagicInitNumber)
			{
				fb_assert(!multiPool);

				PoolType::InitParams initParams;
				initParams.allocator = fb::getSystemHeapAllocator();
				/* Call to getSystemHeapAllocator causes GlobalFixedAllocator to get initialized. I didn't track it too closely, but WTF probably happens. */
				if (initCheckValue == MagicInitNumber)
					return;

				initParams.backupAllocator = new memory::HeapAllocator();
				initParams.oversizeAllocator = initParams.backupAllocator;
				initParams.maxAllocationSizeInBytes = 2048 * 2;
				initParams.alignmentInBytes = 8;
				initParams.growMode = memory::MultiSizePool::GrowModeLinear;

				multiPool = new PoolType(initParams,PoolType::FlagThreadSafe);

				initCheckValue = MagicInitNumber;
			}
		}
	}

	void setCurrentThreadAsMain()
	{
		mainThreadHandle = getCurrentThreadHandle();
	}
	
	void destruct()
	{
		initCheckValue = MagicDestructNumber;
	}

	uint64_t getCurrentAllocationBytes()
	{
		return multiPool ? multiPool->getCurrentAllocationBytes() : 0;
	}

FB_END_PACKAGE2()

FB_PACKAGE1(lang)

	void *globalFixedAllocate(size_t size)
	{
		profiling::AllocationProfiler::pushAllocationEvent(size, "globalFixedAllocate");
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
		void *ptr = fb::lang::allocateMemory(size);
		fb_assert(ptr && "Failed to allocate memory");
		return ptr;
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE
		if (memory::fixedallocator::initCheckValue != memory::fixedallocator::MagicInitNumber)
		{
			if (memory::fixedallocator::initCheckValue == memory::fixedallocator::MagicDestructNumber)
				return lang::osAllocate(size);
			else
				memory::fixedallocator::initImp();
		}

		void *ptr = memory::fixedallocator::multiPool->PoolType::allocate(size);
		fb_assert(ptr && "Failed to allocate memory");
		return ptr;
#endif
	}

	void globalFixedDeallocate(size_t size, void *ptr)
	{
		profiling::AllocationProfiler::pushFreeEvent(size, "globalFixedAllocate");
#if FB_POOL_ALLOCATORS_DISABLED == FB_TRUE
		lang::freeMemory(ptr);
#elif FB_POOL_ALLOCATORS_DISABLED == FB_FALSE
		if (memory::fixedallocator::initCheckValue != memory::fixedallocator::MagicInitNumber)
			return;

		memory::fixedallocator::multiPool->PoolType::deallocate(ptr, size);
#endif
	}


FB_END_PACKAGE1()
