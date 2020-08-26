#include "Precompiled.h"
#include "AllocationDebugger.h"

#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/MemoryOperatorsConfig.h"
#include "fb/container/LinearPodMap.h"
#include "fb/container/LinearPodSet.h"
#include "fb/lang/IAllocationDebuggerDump.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/stacktrace/StackTrace.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/ScopedTimer.h"


FB_PACKAGE1(lang)


class AllocationDebugger::Impl
{
public:
#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_FALSE
	static const SizeType captureCallStackDepth = 16;
#else
	/* FiendishAllocator tends to explode the amount of paths, as it disables pools for small allocations. Must battle it with less depth */
	static const SizeType captureCallStackDepth = 8;
#endif
	static const SizeType invalidIndex = 0xFFFFFFFF;

	typedef CallStackCapture<captureCallStackDepth> CapturedCallStack;

	struct AllocationTrackerData
	{
		uint64_t allocationSize;
		SizeType trackedAllocationPathIndex;
		/* This is just a hint. Allocation never travel to higher indexes, so this can be used as starting point for search */
		SizeType allocationNodeIndex;
#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE
		uint64_t forwardSafeZone[numForwardSafeZoneBlocks];
#endif
	};

		static AllocationTrackerData& getAllocationTrackerData(void *pointer)
	{
		return *reinterpret_cast<AllocationTrackerData*>(pointer);
	}



	/* Data for one allocation. Makes up a linked list  */
	struct AllocationListNode
	{
		void *allocationPointer;
		uint64_t timeStamp;
	};

	struct TrackedAllocationPath
	{
		static const SizeType initialAllocationSize = 32;

		TrackedAllocationPath()
			: allocationNodes((AllocationListNode*)lang::osAllocate(sizeof(AllocationListNode) * initialAllocationSize))
		{
		}

		void allocDone(AllocationTrackerData &allocationTrackerData, void *pointer, size_t size, uint64_t timeStamp)
		{
			const uint64_t numLiveAllocations = numAllocations - numDeallocations;
			fb_expensive_assert(numLiveAllocations <= allocationNodeCapacity);
			if (numLiveAllocations == allocationNodeCapacity)
			{
				allocationNodes = (AllocationListNode*)lang::osReallocate(allocationNodes, sizeof(AllocationListNode) * allocationNodeCapacity * 2);
				fb_assert(allocationNodes != nullptr);
				allocationNodeCapacity *= 2;
			}
			/* Surely we don't have over 4 billion live allocations */
			allocationTrackerData.allocationNodeIndex = SizeType(numLiveAllocations);
			allocationNodes[numLiveAllocations].allocationPointer = pointer;
			allocationNodes[numLiveAllocations].timeStamp = timeStamp;
			++numAllocations;
			sizeOfAllocations += size;
			maxNumLiveAllocations = lang::max(maxNumLiveAllocations, numAllocations - numDeallocations);
			maxSizeOfLiveAllocations = lang::max(maxSizeOfLiveAllocations, sizeOfAllocations - sizeOfDeallocations);
		}

		void deallocDone(const AllocationTrackerData &allocationTrackerData, void *pointer)
		{
			const uint64_t numLiveAllocations = numAllocations - numDeallocations;
			/* Search for the pointer starting from the back */
			for (uint64_t i = lang::min<uint64_t>(numLiveAllocations - 1, allocationTrackerData.allocationNodeIndex); i < numLiveAllocations; --i)
			{
				if (allocationNodes[i].allocationPointer == pointer)
				{
					allocationNodes[i] = allocationNodes[numLiveAllocations - 1];
					++numDeallocations;
					sizeOfDeallocations += allocationTrackerData.allocationSize;
					return;
				}
			}
			fb_assert(0 && "No allocation found. Double free?");
		}

		void reallocDone(AllocationTrackerData &allocationTrackerData, void *oldPointer, void *newPointer, size_t newSize, uint64_t timeStamp)
		{
			/* Handle like dealloc + alloc. It's not clear how reallocs should actually be counted. Now place of 
			 * original allocation is used for stack trace even after realloc (handled by caller really) */
			if (oldPointer != nullptr)
				deallocDone(allocationTrackerData, oldPointer);

			allocDone(allocationTrackerData, newPointer, newSize, timeStamp);
		}

		CapturedCallStack callStack;
		/* Individual allocations and their time stamps aren't actually reported outside currently */
		AllocationListNode *allocationNodes = nullptr;
		uint64_t numAllocations = 0;
		uint64_t numDeallocations = 0;
		uint64_t maxNumLiveAllocations = 0;
		uint64_t sizeOfAllocations = 0;
		uint64_t sizeOfDeallocations = 0;
		uint64_t maxSizeOfLiveAllocations = 0;
		SizeType allocationNodeCapacity = initialAllocationSize;
	};


	struct AllocationPathTracker
	{
		AllocationPathTracker() { }

		void allocDone(const CapturedCallStack &callStack, void *pointer, size_t size, uint64_t timeStamp)
		{
			AllocationTrackerData& allocationTrackerData = getAllocationTrackerData(pointer);
			allocationTrackerData.allocationSize = size;
#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
			MutexGuard guard(mutex);
			CallStackToIndexMap::Iterator iter = callStackToIndexMap.find(callStack);
			if (iter == callStackToIndexMap.getEnd())
			{
				// Assert slightly before the allocation path limit is reached as asserting may cause further allocations
				// Leaving space for 512 allocations for assert to use
				fb_assert_once(allocationPaths.getSize() + 512 != allocationPaths.getCapacity() && "Too many unique allocation paths. Grow numExpectedUniqueCallStacks.");
				TrackedAllocationPath& allocationPath = allocationPaths.pushBack();
				allocationPath.callStack = callStack;
				Pair<CallStackToIndexMap::Iterator, bool> result = callStackToIndexMap.insert(callStack, allocationPaths.getSize() - 1);
				iter = result.first;
			}
			allocationTrackerData.trackedAllocationPathIndex = iter.getValue();
			TrackedAllocationPath &allocation = allocationPaths[allocationTrackerData.trackedAllocationPathIndex];
			allocation.allocDone(allocationTrackerData, pointer, size, timeStamp);
#endif
		}

		void deallocDone(void *pointer)
		{
#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
			MutexGuard guard(mutex);
			AllocationTrackerData& allocationTrackerData = getAllocationTrackerData(pointer);
			TrackedAllocationPath &allocation = allocationPaths[allocationTrackerData.trackedAllocationPathIndex];
			allocation.deallocDone(allocationTrackerData, pointer);
			/* Poison the data */
			allocationTrackerData.trackedAllocationPathIndex = 0xFFFFFFFF;
#endif
		}

		void reallocDone(void *oldPointer, void *newPointer, size_t newSize, uint64_t timeStamp)
		{
#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
			MutexGuard guard(mutex);
			AllocationTrackerData& allocationTrackerData = getAllocationTrackerData(newPointer);
			TrackedAllocationPath &allocation = allocationPaths[allocationTrackerData.trackedAllocationPathIndex];
			allocation.reallocDone(allocationTrackerData, oldPointer, newPointer, newSize, timeStamp);
#endif
		}

#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
		Mutex mutex;
		/* About 130 thousand unique call stacks */
		static const SizeType numExpectedUniqueCallStacks = 1 << 17;
		StaticPodVector<TrackedAllocationPath, numExpectedUniqueCallStacks> allocationPaths;
		typedef StaticLinearPodMap<CapturedCallStack, SizeType, numExpectedUniqueCallStacks> CallStackToIndexMap;
		CallStackToIndexMap callStackToIndexMap;
#endif
	};


	class AllocationDebuggerDump : public IAllocationDebuggerDump
	{
	public:
		struct AllocationPathTrackerDump
		{
			StaticPodVector<StackFrame, captureCallStackDepth> stackFrames;
			uint64_t numAllocations;
			uint64_t numDeallocations;
			uint64_t maxNumLiveAllocations;
			uint64_t sizeOfAllocations;
			uint64_t sizeOfDeallocations;
			uint64_t maxSizeOfLiveAllocations;
		};

		
		virtual uint32_t getOverheadPerAllocation() const
		{
			return totalNumExtraBytes;
		}


		virtual SizeType getNumAllocationPathTrackers() const
		{
			return allocationPathTrackerDumps.getSize();
		}


		virtual SizeType getAPTNumStackFrames(SizeType index) const
		{
			return allocationPathTrackerDumps[index].stackFrames.getSize();
		}


		virtual SizeType getAPTRecommendedStackFramesToIgnore() const
		{
			return 2;
		}

		
		virtual const StackFrame &getAPTStackFrame(SizeType index, SizeType stackFrameIndex) const
		{
			return allocationPathTrackerDumps[index].stackFrames[stackFrameIndex];
		}

		
		virtual uint64_t getAPTNumAllocations(SizeType index) const
		{
			return allocationPathTrackerDumps[index].numAllocations;
		}


		virtual uint64_t getAPTNumDeallocations(SizeType index) const
		{
			return allocationPathTrackerDumps[index].numDeallocations;
		}


		virtual uint64_t getAPTMaxNumLiveAllocations(SizeType index) const
		{
			return allocationPathTrackerDumps[index].maxNumLiveAllocations;
		}


		virtual uint64_t getAPTAllocatedBytes(SizeType index) const
		{
			return allocationPathTrackerDumps[index].sizeOfAllocations;
		}


		virtual uint64_t getAPTDeallocatedBytes(SizeType index) const
		{
			return allocationPathTrackerDumps[index].sizeOfDeallocations;
		}


		virtual uint64_t getAPTMaxLiveAllocatedBytes(SizeType index) const
		{
			return allocationPathTrackerDumps[index].maxSizeOfLiveAllocations;
		}


		void copyAllocationPathTrackerData(AllocationPathTracker &tracker)
		{
#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
			MutexGuard guard(tracker.mutex);
			fb_static_assert(Mutex::isReentrant && "This will deadlock without re-entrancy, unless call stack resolving is non-allocating");
			allocationPathTrackerDumps.clear();
			for (const TrackedAllocationPath &path : tracker.allocationPaths)
			{
				AllocationPathTrackerDump &pathDump = allocationPathTrackerDumps.pushBack();
				for (SizeType i = 0; i < path.callStack.numCapturedFrames; ++i)
				{
					lang::StackFrame &frame = pathDump.stackFrames.pushBack();
					lang::resolveCallStackFrame(frame, path.callStack.capturedFrames[i]);
				}
				pathDump.numAllocations = path.numAllocations;
				pathDump.numDeallocations = path.numDeallocations;
				pathDump.maxNumLiveAllocations = path.maxNumLiveAllocations;
				pathDump.sizeOfAllocations = path.sizeOfAllocations;
				pathDump.sizeOfDeallocations = path.sizeOfDeallocations;
				pathDump.maxSizeOfLiveAllocations = path.maxSizeOfLiveAllocations;
			}
#endif
		}


#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
		StaticVector<AllocationPathTrackerDump, AllocationPathTracker::numExpectedUniqueCallStacks> allocationPathTrackerDumps;
#else
		StaticVector<AllocationPathTrackerDump, 1> allocationPathTrackerDumps;
#endif
	};


	struct AllocationNazifier
	{
	public:
		static const bool shouldOverwriteAllocated = true;
#if FB_ENABLE_FIENDISH_NAZI_ALLOCATOR == FB_TRUE
		static const bool shouldOverwriteFreed = false;
#else
		static const bool shouldOverwriteFreed = true;
#endif

		static const uint8_t safeZoneByte = 0xFB;
		static const uint64_t safeZoneBlock = uint64_t(safeZoneByte) + (uint64_t(safeZoneByte) << 8) + 
			(uint64_t(safeZoneByte) << 16) + (uint64_t(safeZoneByte) << 24) + (uint64_t(safeZoneByte) << 32) + 
			(uint64_t(safeZoneByte) << 40) + (uint64_t(safeZoneByte) << 48) + (uint64_t(safeZoneByte) << 56);
		static const uint64_t allocationOverwriteBlock = 0xACE5BA5E5DA151E5;
		static const uint64_t freeOverwriteBlock = 0xCCACCAA0CACCAA;

#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE
		static void writeSafeZones(void *pointer)
		{
			AllocationTrackerData &allocationTrackerData = getAllocationTrackerData(pointer);
			for (SizeType i = 0; i < numForwardSafeZoneBlocks; ++i)
				allocationTrackerData.forwardSafeZone[i] = safeZoneBlock;

			/* Get start of after safe zone */
			char *charPtr = reinterpret_cast<char*>(pointer);
			charPtr += totalNumPrefixBytes + allocationTrackerData.allocationSize;
			/* After safe zone may not be aligned */
			for (SizeType i = 0; i < numAfterSafeZoneBytes; ++i)
				charPtr[i] = safeZoneByte;
		}

		static void checkSafeZones(void *pointer)
		{
			AllocationTrackerData &allocationTrackerData = getAllocationTrackerData(pointer);
			for (SizeType i = 0; i < numForwardSafeZoneBlocks; ++i)
				fb_assert(allocationTrackerData.forwardSafeZone[i] == safeZoneBlock);

			/* Get start of after safe zone */
			uint8_t *charPtr = reinterpret_cast<uint8_t*>(pointer);
			charPtr += totalNumPrefixBytes + allocationTrackerData.allocationSize;
			/* After safe zone may not be aligned */
			for (SizeType i = 0; i < numAfterSafeZoneBytes; ++i)
				fb_assert(charPtr[i] == safeZoneByte);
		}

		/* This will overwrite at least part of after safe zone */
		static void overwriteAllocated(void *pointer)
		{
			if (!shouldOverwriteAllocated)
				return;

			AllocationTrackerData &allocationTrackerData = getAllocationTrackerData(pointer);
			char *charPtr = reinterpret_cast<char*>(pointer);
			charPtr += totalNumPrefixBytes;
			/* This will overwrite at least part of after safe zone */
			for (SizeType i = 0; i < allocationTrackerData.allocationSize + numAfterSafeZoneBytes; i += 8)
				*reinterpret_cast<uint64_t*>(charPtr + i) = allocationOverwriteBlock;
		}

		/* This will overwrite at least part of after safe zone */
		static void overwriteFreed(void *pointer)
		{
			if (!shouldOverwriteFreed)
				return;

			AllocationTrackerData &allocationTrackerData = getAllocationTrackerData(pointer);
			char *charPtr = reinterpret_cast<char*>(pointer);
			charPtr += totalNumPrefixBytes;
			for (SizeType i = 0; i < allocationTrackerData.allocationSize + numAfterSafeZoneBytes; i += 8)
				*reinterpret_cast<uint64_t*>(charPtr + i) = freeOverwriteBlock;
	}

#else
		static void writeSafeZones(void *pointer) { }
		static void checkSafeZones(void *pointer) { }
		static void overwriteAllocated(void *pointer) { }
		static void overwriteFreed(void *pointer) { }
#endif
	};

	Impl()
	{
	}

	~Impl()
	{
		aliveMagicMarker = 0;
	}


	AllocationPathTracker allocationTracker;
	AllocationDebuggerDump dump;

	static const uint32_t maxStatsLineLen = 512;
	ScopedTimer timer;
	static const uint64_t aliveMagicValue = 0x12345678ABCDEF09;
	uint64_t aliveMagicMarker = aliveMagicValue;
};


static AllocationDebugger::Impl &getSingletonImpl()
{
	static AllocationDebugger::Impl singletonImpl;
	return singletonImpl;
}


AllocationDebugger::AllocationDebugger()
	: impl(getSingletonImpl())
{
	static uint32_t counter = 0;
	++counter;
	fb_assert(counter == 1 && "There should be only one initialization of AllocationDebugger");
#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE
	static_assert(sizeof(Impl::AllocationTrackerData) == 32, "Unexpected size of AllocationTrackerData");
#else
	static_assert(sizeof(Impl::AllocationTrackerData) == 16, "Unexpected size of AllocationTrackerData");
#endif
}


AllocationDebugger::~AllocationDebugger()
{
}


void AllocationDebugger::addAllocation(void *pointer, size_t size)
{
	Impl::CapturedCallStack callStack;
#if FB_ENABLE_ALLOCATION_TRACKER == FB_TRUE
	FB_CAPTURE_STACK_TRACE(callStack);
#endif
	impl.allocationTracker.allocDone(callStack, pointer, size, impl.timer.getMicroseconds());
	Impl::AllocationNazifier::overwriteAllocated(pointer);
	Impl::AllocationNazifier::writeSafeZones(pointer);
}


void AllocationDebugger::removeAllocation(void *pointer)
{
	/* During static uninitialization, AllocationDebugger may be gone before last allocations are removed */
	if (impl.aliveMagicMarker != Impl::aliveMagicValue)
		return;

	Impl::AllocationNazifier::checkSafeZones(pointer);
	Impl::AllocationNazifier::overwriteFreed(pointer);
	impl.allocationTracker.deallocDone(pointer);
}


void AllocationDebugger::modifyAllocation(void *oldPointer, void *newPointer, size_t newSize)
{
#if FB_ENABLE_NAZI_ALLOCATOR == FB_TRUE
	/* Currently outside world just uses addAllocation and removeAllocation. This should be fixed to overwrite newly 
	 * allocated part of memory */
	fb_assert_once(0 && "Not fully implemented for Nazi allocator");
#endif
	if (oldPointer != nullptr)
	{
		Impl::AllocationNazifier::checkSafeZones(oldPointer);
		Impl::AllocationNazifier::overwriteFreed(oldPointer);
	}
	impl.allocationTracker.reallocDone(oldPointer, newPointer, newSize, impl.timer.getMicroseconds());
	Impl::AllocationNazifier::writeSafeZones(newPointer);
}


IAllocationDebuggerDump &AllocationDebugger::getDump()
{
	impl.dump.copyAllocationPathTrackerData(impl.allocationTracker);
	return impl.dump;
}


uint64_t AllocationDebugger::getTimeStamp() const
{
	return impl.timer.getMicroseconds();
}


AllocationDebugger &AllocationDebugger::getInstance()
{
	static AllocationDebugger allocationDebugger;
	return allocationDebugger;
}

FB_END_PACKAGE1()
