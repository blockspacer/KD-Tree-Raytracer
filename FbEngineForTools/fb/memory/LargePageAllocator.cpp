#include "Precompiled.h"
#include "LargePageAllocator.h"

#include "fb/container/PodVector.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/DebugHelp.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/logger/LoggingMacros.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/profiling/ScopedProfiler.h"
#include "fb/string/HeapString.h"

FB_PACKAGE1(memory)

class LargePageAllocator::Impl
{
public:
	Impl()
	{
		/* Initialize large page support */
		/* Get privilege to lock pages in memory */
		HANDLE token = nullptr;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
		{
			TempString msg(FB_FUNCTION ": OpenProcessToken() failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			msg << "\n";
			FB_PRINTF(msg.getPointer());
			return;
		}

		LUID luid;
		if (!LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid))
		{
			TempString msg(FB_FUNCTION ": LookupPrivilegeValue() failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			return;
		}
		/* Actually set privilege to allow page locking */
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
		{
			TempString msg(FB_FUNCTION ": AdjustTokenPrivileges() failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			msg << "\n";
			FB_PRINTF(msg.getPointer());
			return;
		}

		/* Get large page minimum size and physical memory amount from OS. Use them to set sensible defaults */
		largePageMinimumSize = GetLargePageMinimum();
		unsigned long long physicallyInstalledMemoryInKilobytes = 0;
		if (!GetPhysicallyInstalledSystemMemory(&physicallyInstalledMemoryInKilobytes))
		{
			TempString msg(FB_FUNCTION ": GetPhysicallyInstalledSystemMemory() failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			msg << "\n";
			FB_PRINTF(msg.getPointer());
			return;
		}
		physicalMemoryAmount = physicallyInstalledMemoryInKilobytes * 1024;
		/* One eight of physical memory is pretty nice value even on 16 GB computer. With larger memory (like 64 GBs), 
		 * it may be a bit low for some things, but generally speaking such things don't happen, so it doesn't matter 
		 * anyway. Besides, VirtualAlloc performance gets really bad, so it probably makes sense to up allocation 
		 * chunk size in that case too */
		maxAmountOfAllocatedBytes = physicalMemoryAmount / 8;
	}


	static Impl &getInstance()
	{
		static Impl instance;
		return instance;
	}

	static const uint64_t megabyte = 1024 * 1024;
	/* 16 gigabytes should be enough for everyone */
	static const uint64_t maxAmountOfBytes = 16 * 1024 * megabyte;
	/* If minimum size is actually larger, that's ok too */
	static const uint32_t presumedLargePageMinimumSize = 2 * megabyte;
	/* About 8000 individual allocations */
	static const uint32_t maxNumAllocations = maxAmountOfBytes / presumedLargePageMinimumSize;
	/* Internally allocate at least a chunk at a time */
	static const uint32_t allocationChunkSize = 256 * megabyte;
	/* VirtualAlloc can be slow, so we allocate bigger chunks at a time. This should be enough to keep track of 
	 * half-used chunks. We really shouldn't have more than few */
	static const uint32_t maxNumberOfTempAllocations = maxAmountOfBytes / allocationChunkSize;
	/* How many consecutive fails to tolerate before giving up */
	static const uint32_t totalFailThreshold = 5;
	/* If allocation of this size or smaller fails on ERROR_NO_SYSTEM_RESOURCES, stop trying */
	static const uint32_t badFailSizeThreshold = 16 * megabyte;
	/* If allocation takes this many milliseconds, it is counted as a failure */
	static const uint32_t appallinglySlowThresholdMs = 100;

	Mutex mutex;
	uint64_t largePageMinimumSize = 0xFFFFFFFFFFFFFFFF;
	uint64_t physicalMemoryAmount = 0;
	uint64_t maxAmountOfAllocatedBytes = 0;
	uint32_t consecutiveFailedAllocations = 0;
	uint32_t enabled = 1;
	lang::AtomicUInt64 allocatedBytes;

	struct AllocationInfo
	{
		uint64_t size = 0;
		uint8_t *buffer = nullptr;
	};

	/* About 8000 AllocationInfos, each 16 bytes, so about 128 kBs. We can reserve that much statically */
	StaticPodVector<AllocationInfo, maxNumAllocations> allocations;
	/* If we have more than 256 half-used allocations, someone is doing something wrong */
	StaticPodVector<AllocationInfo, maxNumberOfTempAllocations> partialChunks;
};


LargePageAllocator::LargePageAllocator()
	: impl(Impl::getInstance())
{

}

LargePageAllocator::~LargePageAllocator()
{

}

bool LargePageAllocator::isAvailable() const
{
	return impl.largePageMinimumSize < 0xFFFFFFFFFFFFFFFF;
}

void LargePageAllocator::setEnabled(bool enabled)
{
	MutexGuard guard(impl.mutex);
	impl.enabled = enabled ? 1U : 0U;
}

uint64_t LargePageAllocator::getLargePageMinimumSize() const
{
	return impl.largePageMinimumSize;
}

void LargePageAllocator::setMaxAmountOfAllocatedBytes(uint64_t newAmount)
{
	/* This shouldn't be spammed anywhere */
	fb_single_thread_assert();
	fb_main_thread_assert();
	MutexGuard guard(impl.mutex);
	impl.maxAmountOfAllocatedBytes = newAmount;
}

void *LargePageAllocator::allocate(uint64_t sizeInBytes)
{
	/* Don't bother with small ones. This also handles not-working case */
	if (sizeInBytes < impl.largePageMinimumSize || !impl.enabled)
		return nullptr;

	/* Size of allocation must be multiple of largePageMinimum */
	if (sizeInBytes % impl.largePageMinimumSize != 0)
	{
		/* Don't allow huge waste */
		uint64_t wastedBytes = impl.largePageMinimumSize - (sizeInBytes % impl.largePageMinimumSize);
		if (wastedBytes > impl.largePageMinimumSize / 2 && sizeInBytes < impl.largePageMinimumSize * 2)
			return nullptr;

		sizeInBytes += wastedBytes;
	}

	/* Micro-optimization: pre-check available space before taking a mutex. Since maxAmountOfAllocatedBytes shouldn't 
	 * change often (if at all) and there shouldn't be many, if any deallocations, this shouldn't give false positives 
	 * often. Both values are only changed when mutex is held, so another check after mutex is taken makes sure we 
	 * won't go over the allowed values. */
	if (atomicLoadRelaxed(impl.allocatedBytes) + sizeInBytes > impl.maxAmountOfAllocatedBytes)
		return nullptr;

	/* VirtualAlloc can sometimes get really slow. Use a timer to detect such moments and limit the damage */
	ScopedTimer timer;

	MutexGuard guard(impl.mutex);
	/* Check again */
	if (atomicLoadRelaxed(impl.allocatedBytes) + sizeInBytes > impl.maxAmountOfAllocatedBytes)
		return nullptr;

	/* Check if we are fully booked. This really should never happen. */
	if (impl.allocations.getSize() == impl.allocations.getCapacity())
		return nullptr;

	/* If we have failed over and over again, just give up */
	if (impl.consecutiveFailedAllocations > Impl::totalFailThreshold)
	{
		FB_PRINTF("LargePageAllocator turning off due to %d concecutive failed allocations. Total allocated bytes: %d kB\n", impl.consecutiveFailedAllocations, uint32_t(atomicLoadRelaxed(impl.allocatedBytes) >> 10));
		impl.largePageMinimumSize = 0xFFFFFFFFFFFFFFFF;
		return nullptr;
	}

	/* Do allocation */
	Impl::AllocationInfo info;
	info.size = sizeInBytes;
	if (sizeInBytes < Impl::allocationChunkSize)
	{
		for (SizeType i = 0, num = impl.partialChunks.getSize(); i < num; ++i)
		{
			Impl::AllocationInfo &partialChunk = impl.partialChunks[i];
			if (partialChunk.size >= sizeInBytes)
			{
				info.buffer = partialChunk.buffer;
				partialChunk.buffer += sizeInBytes;
				partialChunk.size -= sizeInBytes;
				fb_assert(partialChunk.size < Impl::allocationChunkSize && "Bad math somewhere");
				if (partialChunk.size == 0)
					impl.partialChunks.swapOutIndex(i);

				break;
			}
		}
	}
	else
	{
		/* Large block, allocate directly */
		//profiling::ScopedProfiler sp(FB_FUNCTION  ": huge allocation");
		info.buffer = reinterpret_cast<uint8_t*>(VirtualAlloc(nullptr, (size_t)sizeInBytes, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE));
		if (info.buffer == nullptr)
		{
			++impl.consecutiveFailedAllocations;
			DWORD errorCode = GetLastError();
			TempString msg;
			msg.doSprintf(FB_FUNCTION ": Allocation of size %d kB failed. ", uint32_t(sizeInBytes / 1024));
			DebugHelp::addGetLastErrorCode(msg);
			if (errorCode == ERROR_PRIVILEGE_NOT_HELD)
			{
				msg << ". You need to enable \"Local Policies/User Rights Assignment/Lock pages in memory\" policy with Local Security Policy editor in order to allocate large pages\n";
				/* Turn it off */
				impl.largePageMinimumSize = 0xFFFFFFFFFFFFFFFF;
			}
			else if (errorCode == ERROR_NO_SYSTEM_RESOURCES)
			{
				if (sizeInBytes <= Impl::badFailSizeThreshold)
				{
					msg << ". LargePageAllocator turning off due to insufficient system resources on allocation. Total allocated bytes: "
						<< uint32_t(atomicLoadRelaxed(impl.allocatedBytes) >> 10) << " kB\n";
					impl.largePageMinimumSize = 0xFFFFFFFFFFFFFFFF;
				}
			}
			FB_PRINTF(msg.getPointer());
			return nullptr;
		}
		if (timer.getMilliseconds() < Impl::appallinglySlowThresholdMs)
			impl.consecutiveFailedAllocations = 0;
		else
			++impl.consecutiveFailedAllocations;

	}

	if (info.buffer == nullptr)
	{
		/* Allocate new chunk */
		//profiling::ScopedProfiler sp(FB_FUNCTION  ": new chunk");
		/* If there's no more room for partial chunks, just give up */
		if (impl.partialChunks.getSize() == impl.partialChunks.getCapacity())
			return nullptr;

		Impl::AllocationInfo &partialChunk = impl.partialChunks.pushBack();
		partialChunk.size = Impl::allocationChunkSize;
		partialChunk.buffer = reinterpret_cast<uint8_t*>(VirtualAlloc(nullptr, Impl::allocationChunkSize, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE));
		if (partialChunk.buffer != nullptr)
		{
			if (timer.getMilliseconds() < Impl::appallinglySlowThresholdMs)
				impl.consecutiveFailedAllocations = 0;
			else
				++impl.consecutiveFailedAllocations;

			info.buffer = partialChunk.buffer;
			info.size = sizeInBytes;
			partialChunk.buffer += sizeInBytes;
			partialChunk.size -= sizeInBytes;
			fb_assert(partialChunk.size < Impl::allocationChunkSize && "Failing math abound");
		}
		else
		{
			++impl.consecutiveFailedAllocations;
			impl.partialChunks.popBack();

			DWORD errorCode = GetLastError();
			TempString msg;
			msg.doSprintf(FB_FUNCTION ": Allocation of size %d kB failed. ", uint32_t(sizeInBytes / 1024));
			DebugHelp::addGetLastErrorCode(msg);
			if (errorCode == ERROR_PRIVILEGE_NOT_HELD)
			{
				msg << ". You need to enable \"Local Policies/User Rights Assignment/Lock pages in memory\" policy with Local Security Policy editor in order to allocate large pages\n";
			}
			else if (errorCode == ERROR_NO_SYSTEM_RESOURCES)
			{
				msg << ". LargePageAllocator turning off due to insufficient system resources on allocation. Total allocated bytes: " 
					<< uint32_t(atomicLoadRelaxed(impl.allocatedBytes) >> 10) << " kB\n";
				impl.largePageMinimumSize = 0xFFFFFFFFFFFFFFFF;
			}

			FB_PRINTF(msg.getPointer());
			return nullptr;
		}
	}

	impl.allocations.pushBack(info);
	atomicAddRelaxed(impl.allocatedBytes, sizeInBytes);
	return info.buffer;
}

void LargePageAllocator::deallocate(void *buffer)
{
	/* Note: Generally speaking, deallocation doesn't really work. There's no coordination with chunk allocation, so 
	 * if this was used in anything but simplistic cases, memory would fragment badly. Luckily, performance of 
	 * VirtualAlloc will tank much before that becomes a real problem */
	if (buffer == nullptr)
		return;

	MutexGuard guard(impl.mutex);
	/* Presume latests allocations will be deallocated first */
	for (SizeType num = impl.allocations.getSize(), i = num - 1; i < num; ++i)
	{
		if (impl.allocations[i].buffer != buffer)
			continue;

		atomicSubRelaxed(impl.allocatedBytes, impl.allocations[i].size);
		impl.allocations.swapOutIndex(i);
		if (VirtualFree(buffer, 0, MEM_RELEASE) == 0)
		{
			/* Failure. This probably shouldn't happen unless there's double free. There shouldn't be, unless there's a
			 * problem with LPA itself */
			TempString msg(FB_FUNCTION ": VirtualFree MEM_RELEASE failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			msg << "\n";
			FB_PRINTF(msg.getPointer());
		}
		return;
	}
	FB_PRINTF(FB_FUNCTION ": Could not find pointer that was given to deallocate");
}

uint64_t LargePageAllocator::getAmountOfAllocatedBytes() const
{
	return atomicLoadRelaxed(impl.allocatedBytes);
}


LargePageAllocator &LargePageAllocator::getInstance()
{
	static LargePageAllocator instance;
	return instance;
}

FB_END_PACKAGE1()
