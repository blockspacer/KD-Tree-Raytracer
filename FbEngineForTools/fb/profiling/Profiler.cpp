#include "Precompiled.h"
#include "Profiler.h"

#include "fb/algorithm/Sort.h"
#include "fb/container/LinearMap.h"
#include "fb/container/PodVector.h"
#include "fb/container/Vector.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/CallStackListener.h"
#include "fb/lang/FBPrintf.h"
#include "fb/lang/FBSingleThreadAssert.h"
#include "fb/lang/IntTypes.h"
#include "fb/lang/platform/LineFeed.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/string/HeapString.h"

#if FB_COMPILER == FB_MSC
#pragma warning( push )
#pragma warning( disable: 6246 )
#endif

// Without atomics, these will generate data races
#define FB_PROFILE_DISABLE_GLOBALS FB_TRUE

#define FB_PROFILE_DISABLE_ALLOCATION_TRACKING FB_TRUE

extern const char *g_profiler_latest_alloc_function_name;
extern const char *g_profiler_latest_alloc_class_name;

#include "fb/lang/IncludeWindows.h"

// Hacky internal defines -- in here to test stuff without complete rebuild
//#define DISABLE_PROFILING
//#define LOG_ALL_ALLOCATIONS
//#define LOG_ALL_ALLOCATIONS_FILE "c:\\allocations.log"

// Keep track of individual pointers to better understand where memory is being spent
//#define TRACK_INDIVIDUAL_POINTERS

FB_PACKAGE1(profiling)

static int64_t getCurrentFrequency()
{
	LARGE_INTEGER r;
	r.QuadPart = 0;
	QueryPerformanceFrequency(&r);
	return r.QuadPart / 1000;
}

namespace {

	struct MemoryHotspot
	{
		HeapString str;
		size_t memoryAllocatedOverall;
		size_t pointersAllocatedOverall;
		size_t memoryFreedOverall;
		size_t pointersFreedOverall;

		MemoryHotspot()
		:	memoryAllocatedOverall(0)
		,	pointersAllocatedOverall(0)
		,	memoryFreedOverall(0)
		,	pointersFreedOverall(0)
		{
		}

		bool operator < (const MemoryHotspot &s) const
		{
			return memoryAllocatedOverall > s.memoryAllocatedOverall;
		}
	};

	bool sortMemoryHotspotPointers(const MemoryHotspot &s1, const MemoryHotspot &s2)
	{
		return s1.pointersAllocatedOverall > s2.pointersAllocatedOverall;
	}

	typedef uint64_t TimeType;


	static ScopedTimer timer;

	const char *getPrettyFuctionName(const char *str)
	{
		#if FB_COMPILER == FB_MSC
			size_t length = strlen(str);
			for (size_t i = length - 2; i > 0; --i)
			{
				if (str[i] == ':')
					return str + i + 1;
			}
		#endif

		return str;
	}

	struct Entry;
	struct EntryTimeSorter
	{
		bool operator () (const Entry *a, const Entry *b) const;
	};

	// Note - same entry (or rather, entry with identical classname/functionName)
	// can be on several places in callstack!
	struct Entry
	{
		// Entry information
		const char *className;
		const char *functionName;

		// Tree information
		Entry *parent;
		typedef PodVector<Entry *> ChildList;
		ChildList childs;

		// Timing stats
		TimeType totalTime;
		TimeType mostRecentTime;
		TimeType maxTime;
		int callCount;
		TimeType enterTime;

		// Memory stats
		size_t memoryAllocatedAmount;
		size_t memoryFreedAmount;
		uint32_t pointerAllocatedAmount;
		uint32_t pointerFreedAmount;

		// Refcount for recursion support
		int callRefcount;
		int breakpointCounter;

		Entry(const char *className_, const char *functionName_)
		:	className(className_),
			functionName(functionName_),
			parent(0),
			enterTime(0),
			callRefcount(0),
			breakpointCounter(0)
		{
			clearTiming();
			clearMemory();
		}

		void enter()
		{
			if (breakpointCounter)
			{
				fb_assert(0 && "Manually set profiler breakpoint hit.");
			}

			//fb_expensive_assert(callCount == 0);
			callRefcount = 1;
			enterTime = timer.getMilliseconds();
		}

		void leave()
		{
			TimeType currentTime = timer.getMilliseconds();
			//fb_expensive_assert(currentTime >= enterTime);

			TimeType currentSample = currentTime - enterTime;
			totalTime += currentSample;
			++callCount;

			mostRecentTime = currentSample;
			if (currentSample > maxTime)
				maxTime = currentSample;
		}

		void clearTiming()
		{
			totalTime = 0;
			mostRecentTime = 0;
			maxTime = 0;
			callCount = 0;

			for (SizeType i = 0; i < childs.getSize(); ++i)
				childs[i]->clearTiming();
		}

		void clearMemory()
		{
			memoryAllocatedAmount = 0;
			memoryFreedAmount = 0;
			//memoryPeakAmount = 0;
			pointerAllocatedAmount = 0;
			pointerFreedAmount = 0;

			for (SizeType i = 0; i < childs.getSize(); ++i)
				childs[i]->clearMemory();
		}

		void getInclusiveMemory(size_t &memoryAllocatedAmountInclusive, size_t &memoryFreedAmountInclusive, int &pointerAllocatedAmountInclusive, int &pointerFreedAmountInclusive)
		{
			memoryAllocatedAmountInclusive += memoryAllocatedAmount;
			memoryFreedAmountInclusive += memoryFreedAmount;
			pointerAllocatedAmountInclusive += pointerAllocatedAmount;
			pointerFreedAmountInclusive  += pointerFreedAmount;

			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *e = childs[i];
				if (e)
					e->getInclusiveMemory(memoryAllocatedAmountInclusive, memoryFreedAmountInclusive, pointerAllocatedAmountInclusive, pointerFreedAmountInclusive);
			}
		}

		Entry *getEntry(const char *classNameToFind, const char *functionNameToFind)
		{
			// This is a tree, so having flat children list is slow.
			// Having a hashmap/such of childer would be overkill on most cases, tho.
			// Instead, we could do a binary seach if keeping this sorted alphabetically ..
			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *e = childs[i];
				if ((e->className == classNameToFind) && (e->functionName == functionNameToFind))
				{
					e->enter();
					return e;
				}
			}

			// Not found, add a new entry
			Entry *e = new Entry(classNameToFind, functionNameToFind);
			childs.pushBack(e);
			e->parent = this;
			e->enter();

			return e;
		}

		void operator =(const Entry &other)
		{
			className = other.className;
			functionName = other.functionName;
			parent = 0;
			childs.clear();
			totalTime = other.totalTime;
			mostRecentTime = other.mostRecentTime;
			maxTime = other.maxTime;
			callCount = other.callCount;
			enterTime = other.enterTime;
			memoryAllocatedAmount = other.memoryAllocatedAmount;
			memoryFreedAmount = other.memoryFreedAmount;
			//memoryPeakAmount = other.memoryPeakAmount;
			pointerAllocatedAmount = other.pointerAllocatedAmount;
			pointerFreedAmount = other.pointerFreedAmount;
			callRefcount = other.callRefcount;

			childs.reserve(other.childs.getSize());
			for (SizeType i = 0; i < other.childs.getSize(); ++i)
			{
				Entry *eo = other.childs[i];
				Entry *ec = new Entry(eo->className, eo->functionName);
				*ec = *eo;

				childs[i] = ec;
			}
		}


		const HeapString& getEmptySpaces(SizeType numSpaces)
		{
			static Vector<HeapString> strings;
			/* Specially handle "negative" amounts. */
			if (numSpaces > 0xFFFF)
				return HeapString::empty;

			if (strings.getSize() > numSpaces)
				return strings[numSpaces];

			strings.reserve(numSpaces);
			if (strings.isEmpty())
				strings.pushBack(HeapString::empty);

			while (strings.getSize() <= numSpaces)
			{
				strings.pushBack(HeapString(strings.getBack()));
				strings.getBack() += " ";
			}
			return strings[numSpaces];
		}


		// Stats
		void outputData(HeapString &string, SizeType indent, bool curlyScopedFormat)
		{
			//fb_single_thread_assert();

			const SizeType statPosition1 = 150;
			const SizeType statPosition2 = statPosition1 + 20;
			const SizeType statPosition3 = statPosition2 + 20;
			const SizeType statPosition4 = statPosition3 + 20;
			const SizeType indentSpaces = indent * 3;
			TempString line;

			if (indent == 0)
			{
			}
			else
			{
				if (indent > 2 && callCount == 0)
					return;

#ifdef TRIMMED_LOG
				int msec = (int) (totalTime / getCurrentFrequency());
				if (indent > 3 && msec == 0 && memoryAllocatedAmount < 1024)
					return;
#endif
				float msecF = (float(totalTime) / (getCurrentFrequency()));

				line += getEmptySpaces(indentSpaces);

				if (className)
				{
					line += className;
					line += ":";
				}
				line += getPrettyFuctionName(functionName);

				line += getEmptySpaces(statPosition1 - line.getLength());

				line.appendFloat(msecF, 1);
				line += " (";
				line += callCount;
				line += ")";

				line += getEmptySpaces(statPosition2 - line.getLength());

				float avgTime = msecF;
				if (callCount > 0)
					avgTime = msecF / callCount;
				line.appendFloat(avgTime, 1);
				line += " (";
				float maxTimeMsec = (float(maxTime) / getCurrentFrequency());
				line.appendFloat(maxTimeMsec, 1);
				line += ")";

				line += getEmptySpaces(statPosition3 - line.getLength());

				// Make memory stats inclusive like timing ..
				size_t memoryAllocatedAmountInclusive = 0;
				size_t memoryFreedAmountInclusive = 0;
				int pointerAllocatedAmountInclusive = 0;
				int pointerFreedAmountInclusive = 0;
				getInclusiveMemory(memoryAllocatedAmountInclusive, memoryFreedAmountInclusive, pointerAllocatedAmountInclusive, pointerFreedAmountInclusive);

				line += (int) (memoryAllocatedAmountInclusive / 1024);
				line += " (";
				line += (int) (memoryFreedAmountInclusive / 1024);
				line += ")";

				line += getEmptySpaces(statPosition4 - line.getLength());

				line += pointerAllocatedAmountInclusive;
				line += " (";
				line += pointerFreedAmountInclusive;
				line += ")";
			}

			line += FB_PLATFORM_LF;
			string += line;

			// Sort childs based on time

			if (curlyScopedFormat)
			{
				string += "{" FB_PLATFORM_LF;
			}

			algorithm::sort(childs.getBegin(), childs.getEnd(), EntryTimeSorter());
			SizeType reserveHint = lang::max(string.getLength() + (childs.getSize() * 1024), string.getLength() * 2);
			if (string.getCapacity() < reserveHint)
				string.reserve(reserveHint);

			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *e = childs[i];
				e->outputData(string, indent + 1, curlyScopedFormat);
			}

			if (curlyScopedFormat)
			{
				string += "}" FB_PLATFORM_LF;
			}
		}

		void iterateFunctionTimings(LinearMap<HeapString, TimeType> &functionList, Entry &e)
		{
			TimeType elapsedTime = totalTime;

			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *entry = childs[i];
				elapsedTime -= entry->totalTime;
			}

			HeapString name;
			if (className)
			{
				name += className;
				name += ":";
			}
			name += getPrettyFuctionName(functionName);

			functionList[name] += elapsedTime;
			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *entry = childs[i];
				entry->iterateFunctionTimings(functionList, *entry);
			}
		}

		void iterateFunctionMemory(LinearMap<HeapString, MemoryHotspot> &funcList, Entry &e)
		{
			HeapString str;
			if (e.className && strlen(e.className) > 2)
			{
				str = e.className;
				str += ":";
			}

			str += getPrettyFuctionName(e.functionName);

			MemoryHotspot &mh = funcList[str];
			mh.memoryAllocatedOverall += e.memoryAllocatedAmount;
			mh.pointersAllocatedOverall += e.pointerAllocatedAmount;
			mh.memoryFreedOverall += e.memoryFreedAmount;
			mh.pointersFreedOverall += e.pointerFreedAmount;

			for (SizeType i = 0; i < e.childs.getSize(); ++i)
			{
				Entry *en = e.childs[i];
				iterateFunctionMemory(funcList, *en);
			}
		}


		void iterateClassMemory(LinearMap<HeapString, MemoryHotspot> &classList, Entry &e, const HeapString &activeClass)
		{
			HeapString str = activeClass;
			if (e.className && strlen(e.className) > 2)
				str = e.className;

			MemoryHotspot &mh = classList[str];
			mh.memoryAllocatedOverall += e.memoryAllocatedAmount;
			mh.pointersAllocatedOverall += e.pointerAllocatedAmount;
			mh.memoryFreedOverall += e.memoryFreedAmount;
			mh.pointersFreedOverall += e.pointerFreedAmount;

			for (SizeType i = 0; i < e.childs.getSize(); ++i)
			{
				Entry *en = e.childs[i];
				iterateClassMemory(classList, *en, str);
			}
		}

		struct MostRecentTimeEntryInternal
		{
			const char *className;
			const char *functionName;
			int64_t tempSum;
		};

		void getMostRecentTimes(MostRecentTimeEntryInternal *entries, SizeType numEntries)
		{
			if (className && functionName)
			{
				for (SizeType i = 0; i < numEntries; i++)
				{
					if (entries[i].className != 0 && strcmp(className, entries[i].className) == 0
						&& strcmp(getPrettyFuctionName(functionName), entries[i].functionName) == 0)
					{
						entries[i].tempSum += mostRecentTime;
						mostRecentTime = 0;
					}
				}
			}
			else if (functionName)
			{
				for (SizeType i = 0; i < numEntries; i++)
				{
					if (strcmp(getPrettyFuctionName(functionName), entries[i].functionName) == 0)
					{
						entries[i].tempSum += mostRecentTime;
						mostRecentTime = 0;
					}
				}
			}

			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *en = childs[i];
				en->getMostRecentTimes(entries, numEntries);
			}
		}

		void resetData()
		{
			totalTime = 0;
			maxTime = 0;
			callCount = 0;
			//enterTime = 0;

			for (SizeType i = 0; i < childs.getSize(); ++i)
			{
				Entry *e = childs[i];
				e->resetData();
			}
		}
	};

	bool EntryTimeSorter::operator () (const Entry *a, const Entry *b) const
	{
		return a->totalTime > b->totalTime;
	}

	struct PointerInformation
	{
		void *address;
		size_t size;
		const char *id;
		Entry *entry;

		PointerInformation()
		:	address(0)
		,	size(0)
		,	id(0)
		,	entry(0)
		{
		}
	};

	struct PointerStats
	{
		int allocations;
		size_t allocatedMemory;
		Entry *e;
		const char *className;

		PointerStats()
		:	allocations(0)
		,	allocatedMemory(0)
		,	e(0)
		,	className(0)
		{
		}
	};

	typedef LinearMap<void *, PointerInformation> PointerList;

	struct PointerStatsSorter
	{
		bool operator () (const PointerStats &a, const PointerStats &b) const
		{
			return a.allocatedMemory > b.allocatedMemory;
		}
	};

	struct State
	{
		Entry root;

		#ifdef TRACK_INDIVIDUAL_POINTERS
			PointerList pointerList;
		#endif

		State()
		:	root(0, "root")
		{
		}
	};

	typedef LinearMap<DynamicString, State *> StateList;

	struct ThreadState
	{
		State *currentState;
		Entry *currentEntry;
		bool allocationFlag;

		ThreadState()
		:	currentState(0)
		,	currentEntry(0)
		,	allocationFlag(false)
		{
		}
	};

	thread_local ThreadState *threadState = nullptr;
	thread_local bool temporaryDisableFlag = false;
	int instanceAmount = 0;

	struct ThreadStateCollection
	{
		Mutex mutex;
		typedef PodVector<State *> StateArray;
		StateArray stateList;

		ThreadStateCollection()
		{
		}

		~ThreadStateCollection()
		{
		}

		void addState(State *s)
		{
			MutexGuard m(mutex);
			stateList.pushBack(s);
		}

		const StateArray &lockStateList()
		{
			mutex.enter();
			return stateList;
		}

		void unlockStateList()
		{
			mutex.leave();
		}
	};

	ThreadStateCollection *threadStateCollection = nullptr;

	ThreadState *getThreadState()
	{
		ThreadState *s = threadState;
		if (s == nullptr)
		{
			temporaryDisableFlag = true;

			s = new ThreadState();
			s->currentState = new State();
			s->currentEntry = &s->currentState->root;
			threadState = s;
			threadStateCollection->addState(s->currentState);

			// Release the block
			temporaryDisableFlag = false;
		}

		return s;
	}

	void initThreadState()
	{
		threadStateCollection = new ThreadStateCollection();
	}
}

struct Profiler::Data: public lang::CallStackListener
{
	bool printDebugOutput;
	task::Scheduler *scheduler;

	#ifdef LOG_ALL_ALLOCATIONS
		FILE *fp;
	#endif

	Data()
	:	printDebugOutput(true)
	,	scheduler(0)
	{
		#ifdef LOG_ALL_ALLOCATIONS
			fp = fopen(LOG_ALL_ALLOCATIONS_FILE, "wb");
		#endif

		initThreadState();

		lang::addStackListener(this);
#if FB_PROFILE_DISABLE_ALLOCATION_TRACKING == FB_FALSE
		lang::LowlevelMemoryDebugger::addMemoryListener(this);
#endif

		fb_assert(instanceAmount == 0 && "Due to TLS, only one profiler instance supported!");
		++instanceAmount;
	}

	~Data()
	{
		--instanceAmount;
		lang::removeStackListener(this);
		//delete currentState;

		#ifdef LOG_ALL_ALLOCATIONS
			if (fp)
				fclose(fp);
		#endif
	}

	void pushFunction(const char *className, const char *functionName)
	{
		if (temporaryDisableFlag)
			return;

		#ifdef DISABLE_PROFILING
			return;
		#endif

		ThreadState *FB_RESTRICT s = getThreadState();
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");

		// Recursion support
		if ((s->currentEntry->className == className) && (s->currentEntry->functionName == functionName))
		{
			++s->currentEntry->callCount;
			++s->currentEntry->callRefcount;
			return;
		}

		s->currentEntry = s->currentEntry->getEntry(className, functionName);
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");
	}

	void popFunction(const char *className, const char *functionName)
	{
		if (temporaryDisableFlag)
			return;

		#ifdef DISABLE_PROFILING
			return;
		#endif

		ThreadState *FB_RESTRICT s = getThreadState();
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");

		// Recursion support
		if (--s->currentEntry->callRefcount > 0)
			return;

		s->currentEntry->leave();
		s->currentEntry = s->currentEntry->parent;

		//fb_assert(currentEntry);
		className = s->currentEntry->className;
		functionName = s->currentEntry->functionName;
	}

	void addAllocation(void *pointer, size_t size, uint32_t allocationType)
	{
		if (temporaryDisableFlag)
			return;

		ThreadState *FB_RESTRICT s = getThreadState();
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");

		#ifdef DISABLE_PROFILING
			return;
		#endif

		// Avoid logging profiler memory to here
		if (s->allocationFlag)
			return;
		s->allocationFlag = true;

		{
			#ifndef FB_PROFILE_DISABLE_GLOBALS
				// TEMP: ...
				// HACK: ...
				g_profiler_latest_alloc_class_name = s->currentEntry->className;
				g_profiler_latest_alloc_function_name = s->currentEntry->functionName;
			#endif

			s->currentEntry->memoryAllocatedAmount += size;
			++(s->currentEntry->pointerAllocatedAmount);
		}

		#ifdef TRACK_INDIVIDUAL_POINTERS
			PointerInformation i;
			i.address = pointer;
			i.size = size;
			i.id = id;
			i.entry = s->currentEntry;

			s->currentState->pointerList[pointer] = i;
		#endif

		#ifdef LOG_ALL_ALLOCATIONS
			#ifndef FB_LANG_CALLSTACK
				#error "Logging allocations without callstack is prolly not intended."
			#endif

			if (fp)
			{
				const char *a = "";
				const char *b = "";

				fprintf(fp, "0x%x, %d (", pointer, size);
				Entry *e = s->currentEntry;
				while (e)
				{
					if (e->className)
						a = e->className;
					if (e->functionName)
						b = e->functionName;
					fprintf(fp, "%s:%s, ", a, b);
					e = e->parent;
				}

				fprintf(fp, ")\r\n");
				fflush(fp);
			}
		#endif

		s->allocationFlag = false;
	}

	void removeAllocation(void *pointer, size_t size, uint32_t allocationType)
	{
		if (temporaryDisableFlag)
			return;

		ThreadState *FB_RESTRICT s = getThreadState();
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");

		#ifdef DISABLE_PROFILING
			return;
		#endif

		// Avoid logging profiler memory to here
		if (s->allocationFlag)
			return;
		s->allocationFlag = true;

		{
			#ifndef FB_PROFILE_DISABLE_GLOBALS
				// TEMP: ...
				// HACK: ...
				g_profiler_latest_alloc_class_name = s->currentEntry->className;
				g_profiler_latest_alloc_function_name = s->currentEntry->functionName;
			#endif

			s->currentEntry->memoryFreedAmount += size;
			++(s->currentEntry->pointerFreedAmount);
		}

		#ifdef TRACK_INDIVIDUAL_POINTERS
			PointerList::Iterator it = s->currentState->pointerList.find(pointer);
			if (it != s->currentState->pointerList.getEnd())
			{
				s->currentState->pointerList.erase(it);
			}
		#endif

		s->allocationFlag = false;
	}

	void modifyAllocation(void *oldPointer, size_t oldSize, void *newPointer, size_t newSize, uint32_t allocationType)
	{
		if (temporaryDisableFlag)
			return;

		ThreadState *FB_RESTRICT s = getThreadState();
		fb_expensive_assert(s->currentEntry && "Refactored regression assert triggered.");

		// Avoid logging profiler memory to here
		if (s->allocationFlag)
			return;
		s->allocationFlag = true;

		{
			#ifndef FB_PROFILE_DISABLE_GLOBALS
				// TEMP: ...
				// HACK: ...
				g_profiler_latest_alloc_class_name = s->currentEntry->className;
				g_profiler_latest_alloc_function_name = s->currentEntry->functionName;
			#endif

			s->currentEntry->memoryAllocatedAmount += newSize;
			s->currentEntry->memoryFreedAmount += newSize;

			++(s->currentEntry->pointerAllocatedAmount);
			++(s->currentEntry->pointerFreedAmount);
		}

		s->allocationFlag = false;
	}

	const char* getStatsString()
	{
		/* Not implemented */
		return "";
	}

	State *getWantedState(const DynamicString &stateName)
	{
		if (temporaryDisableFlag)
			return 0;

		State *dumpState = 0;
		/*
		StateList::Iterator it = stateList.find(stateName);
		if (it != stateList.getEnd())
			dumpState = it.getValue();
		else
			dumpState = currentState;
		*/

		return dumpState;
	}

	struct Hotspot
	{
		HeapString str;
		TimeType time;

		Hotspot()
		:	time(0)
		{
		}

		bool operator < (const Hotspot &s) const
		{
			return time > s.time;
		}
	};

	HeapString outputData(const DynamicString &stateName, bool curlyScopedFormat)
	{
		if (temporaryDisableFlag)
			return HeapString::empty;

		// Make sure we don't blow things up during state traversal
		temporaryDisableFlag = true;

		// At the moment this isn't guaranteed to be a safe call., as we'd need a global mutex to do that.
		// However, as this function is in practice called after program has been running for a while,
		// full stack graph should be in memory already - meaning worst case is small inconsistensies on profile data
		// (timing/memory during state collection).

		HeapString result;
		if (!curlyScopedFormat)
			result += FB_PLATFORM_LF "Call graph profiling (in milliseconds) --- overall time (call amount) --- avg (max time) -- allocated memory (freed memory) KB -- allocated pointers (freed pointers)";
		else
			result += FB_PLATFORM_LF "ProfilerDump" FB_PLATFORM_LF "{" FB_PLATFORM_LF;

		ThreadStateCollection::StateArray stateList = threadStateCollection->lockStateList();
		int index = 1;
		for (ThreadStateCollection::StateArray::Iterator it = stateList.getBegin(); it != stateList.getEnd(); ++it)
		{
			if (curlyScopedFormat)
			{
				if (index == 1)
					result += "Main Thread" FB_PLATFORM_LF "{" FB_PLATFORM_LF "Callstack";
				else if (index == 2)
					result += "Additional Threads" FB_PLATFORM_LF "{" FB_PLATFORM_LF;

				if (index >= 2)
					result += "Thread" FB_PLATFORM_LF "{" FB_PLATFORM_LF "Callstack";
			}
			else if (index > 1)
			{
				result += FB_PLATFORM_LF FB_PLATFORM_LF;
				result += "--- Next thread ---" FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
			}

			// Basic profiling info
			{
				(*it)->root.outputData(result, 0, curlyScopedFormat);
			}

			// Add hotspots ..
			{
				// First, we need to collect unique identifier timings (self with childs excluded)
				LinearMap<HeapString, TimeType> functionList;
				(*it)->root.iterateFunctionTimings(functionList, (*it)->root);

				// And then, put the results to vector and sort that
				Vector<Hotspot> hotspots;
				LinearMap<HeapString, TimeType>::Iterator funcIt = functionList.getBegin();
				for (; funcIt != functionList.getEnd(); ++funcIt)
				{
					Hotspot s;
					s.str = funcIt.getKey();
					s.time = funcIt.getValue();
					hotspots.pushBack(s);
				}

				algorithm::sort(hotspots.getBegin(), hotspots.getEnd());

				SizeType outputAmount = 50;
				if (hotspots.getSize() < outputAmount)
					outputAmount = hotspots.getSize();

				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += "    Hotspots (overall time, children excluded):" FB_PLATFORM_LF;

				if (curlyScopedFormat)
					result += "{" FB_PLATFORM_LF;

				for (SizeType i = 0; i < outputAmount; ++i)
				{
					result += "        ";
					SizeType msec = SizeType(hotspots[i].time / getCurrentFrequency());

					result += hotspots[i].str;
					result += " (";
					result += msec;
					result += " ms)";
					result += FB_PLATFORM_LF;
				}

				if (curlyScopedFormat)
					result += "}" FB_PLATFORM_LF;
			}

			// Add memory and pointer hotspots
			{
				// First, we need to collect unique tag memory (self with non-class childs included)
				LinearMap<HeapString, MemoryHotspot> classList;
				(*it)->root.iterateFunctionMemory(classList, (*it)->root);

				// And then, put the results to vector and sort that
				Vector<MemoryHotspot> hotspots;
				LinearMap<HeapString, MemoryHotspot>::Iterator classIt = classList.getBegin();
				for (; classIt != classList.getEnd(); ++classIt)
				{
					MemoryHotspot s = classIt.getValue();
					s.str = classIt.getKey();
					hotspots.pushBack(s);
				}

				algorithm::sort(hotspots.getBegin(), hotspots.getEnd());

				SizeType outputAmount = 50;
				if (hotspots.getSize() < outputAmount)
					outputAmount = hotspots.getSize();

				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += "    Tag memory allocations sorted by memory (self and non-class children included):" FB_PLATFORM_LF;
				if (curlyScopedFormat)
					result += "{" FB_PLATFORM_LF;

				SizeType s1 = 60;
				SizeType s2 = 85;
				SizeType s3 = 110;
				SizeType s4 = 135;

				result += "        ";
				SizeType l = result.getLength();
				//result += "Function";
				SizeType spaces = s1 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB allocated";

				spaces = s2 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB released";

				spaces = s3 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers allocated";

				spaces = s4 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers released";
				result += FB_PLATFORM_LF;

				for (SizeType i = 0; i < outputAmount; ++i)
				{
					result += "        ";
					SizeType kb1 = (SizeType) (hotspots[i].memoryAllocatedOverall / 1024);
					SizeType kb2 = (SizeType) (hotspots[i].memoryFreedOverall / 1024);
					SizeType len = result.getLength();

					result += hotspots[i].str;

					SizeType spaces2 = s1 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += "";
					result += kb1;

					spaces2 = s2 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += kb2;

					spaces2 = s3 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += SizeType(hotspots[i].pointersAllocatedOverall);

					spaces2 = s4 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += SizeType(hotspots[i].pointersFreedOverall);
					result += FB_PLATFORM_LF;
				}

				if (curlyScopedFormat)
					result += "}" FB_PLATFORM_LF;

				algorithm::sort(hotspots.getBegin(), hotspots.getEnd(), sortMemoryHotspotPointers);
				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += "    Tag memory allocations sorted by pointers (self and non-class children included):" FB_PLATFORM_LF;

				if (curlyScopedFormat)
					result += "{" FB_PLATFORM_LF;

				result += "        ";
				l = result.getLength();
				//result += "Function";
				spaces = s1 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers allocated";

				spaces = s2 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers released";

				spaces = s3 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB allocated";

				spaces = s4 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB released";
				result += FB_PLATFORM_LF;

				for (SizeType i = 0; i < outputAmount; ++i)
				{
					result += "        ";
					SizeType kb1 = (SizeType) (hotspots[i].memoryAllocatedOverall / 1024);
					SizeType kb2 = (SizeType) (hotspots[i].memoryFreedOverall / 1024);
					SizeType len = result.getLength();

					result += hotspots[i].str;

					spaces = s1 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersAllocatedOverall;

					spaces = s2 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersFreedOverall;

					spaces = s3 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += kb1;

					spaces = s4 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += kb2;

					result += FB_PLATFORM_LF;
				}

				if (curlyScopedFormat)
					result += "}" FB_PLATFORM_LF;
			}

			// Add class memory usages
			if (!curlyScopedFormat)
			{
				// First, we need to collect unique class memory (self with non-class childs included)
				LinearMap<HeapString, MemoryHotspot> classList;
				(*it)->root.iterateClassMemory(classList, (*it)->root, SmallTempString("Unspecified"));

				// And then, put the results to vector and sort that
				Vector<MemoryHotspot> hotspots;
				LinearMap<HeapString, MemoryHotspot>::Iterator classIt = classList.getBegin();
				for (; classIt != classList.getEnd(); ++classIt)
				{
					MemoryHotspot s = classIt.getValue();
					s.str = classIt.getKey();
					hotspots.pushBack(s);
				}

				algorithm::sort(hotspots.getBegin(), hotspots.getEnd());

				SizeType outputAmount = 50;
				if (hotspots.getSize() < outputAmount)
					outputAmount = hotspots.getSize();

				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += "    Class memory allocations sorted by memory (self and non-class children included):" FB_PLATFORM_LF;

				if (curlyScopedFormat)
					result += "{" FB_PLATFORM_LF;

				SizeType s1 = 40;
				SizeType s2 = 65;
				SizeType s3 = 90;
				SizeType s4 = 115;

				result += "        ";
				SizeType l = result.getLength();
				//result += "Class";
				SizeType spaces = s1 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB allocated";

				spaces = s2 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB released";

				spaces = s3 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers allocated";

				spaces = s4 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers released";
				result += FB_PLATFORM_LF;

				for (SizeType i = 0; i < outputAmount; ++i)
				{
					result += "        ";
					SizeType kb1 = (SizeType) (hotspots[i].memoryAllocatedOverall / 1024);
					SizeType kb2 = (SizeType) (hotspots[i].memoryFreedOverall / 1024);
					SizeType len = result.getLength();

					result += hotspots[i].str;

					SizeType spaces2 = s1 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += "";
					result += kb1;

					spaces2 = s2 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += kb2;

					spaces2 = s3 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersAllocatedOverall;

					spaces2 = s4 - (result.getLength() - len);
					spaces2 = spaces2 > 0 ? spaces2 : 0;
					for (SizeType j = 0; j < spaces2; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersFreedOverall;
					result += FB_PLATFORM_LF;
				}

				if (curlyScopedFormat)
					result += "}" FB_PLATFORM_LF;

				algorithm::sort(hotspots.getBegin(), hotspots.getEnd(), sortMemoryHotspotPointers);
				result += FB_PLATFORM_LF;
				result += FB_PLATFORM_LF;
				result += "    Class memory allocations sorted by pointers (self and non-class children included):" FB_PLATFORM_LF;

				if (curlyScopedFormat)
					result += "{" FB_PLATFORM_LF;

				result += "        ";
				l = result.getLength();
				//result += "Class";
				spaces = s1 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers allocated";

				spaces = s2 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "pointers released";

				spaces = s3 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB allocated";

				spaces = s4 - (result.getLength() - l);
				spaces = spaces > 0 ? spaces : 0;
				for (SizeType j = 0; j < spaces; ++j)
					result += " ";

				result += "KB released";
				result += FB_PLATFORM_LF;

				for (SizeType i = 0; i < outputAmount; ++i)
				{
					result += "        ";
					SizeType kb1 = (SizeType) (hotspots[i].memoryAllocatedOverall / 1024);
					SizeType kb2 = (SizeType) (hotspots[i].memoryFreedOverall / 1024);
					SizeType len = result.getLength();

					result += hotspots[i].str;

					spaces = s1 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersAllocatedOverall;

					spaces = s2 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += (SizeType)hotspots[i].pointersFreedOverall;

					spaces = s3 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += kb1;

					spaces = s4 - (result.getLength() - len);
					spaces = spaces > 0 ? spaces : 0;
					for (SizeType j = 0; j < spaces; ++j)
						result += " ";

					result += kb2;

					result += FB_PLATFORM_LF;
				}

				if (curlyScopedFormat)
					result += "}" FB_PLATFORM_LF;
			}

			// Track down most offending places for active allocations
			#ifdef TRACK_INDIVIDUAL_POINTERS
				if (!curlyScopedFormat)
				{
					LinearHashap<const char*, PointerStats> idStats;
					LinearHashap<const char *, PointerStats> classStats;
					PointerList &pointerList = (*it)->pointerList;
					for (PointerList::Iterator it = pointerList.getBegin(); it != pointerList.getEnd(); ++it)
					{
						PointerInformation &i = it.getValue();
						PointerStats &s = idStats[i.entry->functionName];
						s.allocations += 1;
						s.allocatedMemory += i.size;
						s.e = i.entry;

						// Iterate to closest class
						Entry *e = i.entry;
						while (e && e->className == 0)
							e = e->parent;

						if (e)
						{
							PointerStats &s = classStats[e->className];
							s.className = e->className;
							s.allocations += 1;
							s.allocatedMemory += i.size;
							s.e = e;
						}
					}


					Vector<PointerStats> ids;
					for (LinearMap<const char *, PointerStats>::Iterator it = idStats.getBegin(); it != idStats.getEnd(); ++it)
					{
						PointerStats &s = it.getValue();
						ids.pushBack(s);
					}

					Vector<PointerStats> classes;
					for (LinearMap<const char *, PointerStats>::Iterator it = classStats.getBegin(); it != classStats.getEnd(); ++it)
					{
						PointerStats &s = it.getValue();
						classes.pushBack(s);
					}

					algorithm::sort(ids.getBegin(), ids.getEnd(), PointerStatsSorter());
					algorithm::sort(classes.getBegin(), classes.getEnd(), PointerStatsSorter());

					result += FB_PLATFORM_LF FB_PLATFORM_LF "    Tags currently containing most allocations (KB, number of allocations):" FB_PLATFORM_LF;
					for (SizeType i = 0; i < ids.getSize(); ++i)
					{
						if (i >= 50)
							break;
						PointerStats &s = ids[i];
						int kb = (int) s.allocatedMemory / 1024;

						result += "        ";
						if (s.e->className)
						{
							result += s.e->className;
							result += ":";
						}

						result += getPrettyFuctionName(s.e->functionName);
						result += " (";
						string::util::convert::addNumberToString(result, kb);
						result += " KB, ";
						string::util::convert::addNumberToString(result, s.allocations);
						result += " allocations)" FB_PLATFORM_LF;
					}


					result += FB_PLATFORM_LF FB_PLATFORM_LF "    Classes currently containing most allocations (KB, number of allocations):" FB_PLATFORM_LF;
					for (SizeType i = 0; i < classes.getSize(); ++i)
					{
						if (i >= 50)
							break;
						PointerStats &s = classes[i];
						int kb = (int) s.allocatedMemory / 1024;

						result += "        ";
						result += s.className;
						result += " (";
						string::util::convert::addNumberToString(result, kb);
						result += " KB, ";
						string::util::convert::addNumberToString(result, s.allocations);
						result += " allocations)" FB_PLATFORM_LF;
					}
				}
			#endif

			if (curlyScopedFormat)
				result += "}" FB_PLATFORM_LF;

			++index;
		}

		if (curlyScopedFormat)
		{
			if (stateList.getSize() > 1)
				result += "}" FB_PLATFORM_LF;
			result += "}" FB_PLATFORM_LF;
		}

		threadStateCollection->unlockStateList();
		temporaryDisableFlag = false;

		return result;
	}

	bool modifyBreakpoint(const DynamicString *stackTrace, SizeType stackSize, int delta)
	{
		fb_assert(stackSize > 1);
		if (temporaryDisableFlag)
			return false;

		ThreadState *FB_RESTRICT s = getThreadState();

		// Skip root node if exists
		if (stackTrace[0] == "root")
		{
			++stackTrace;
			--stackSize;
		}

		// Find the correct spot
		Entry *entry = &s->currentState->root;
		HeapString funcStr;

		while (stackSize)
		{
			bool found = false;
			for (SizeType i = 0; i < entry->childs.getSize(); ++i)
			{
				Entry *e = entry->childs[i];

				if (e->className)
				{
					funcStr = e->className;
					funcStr += ":";
				}

				funcStr += getPrettyFuctionName(e->functionName);
				if (stackTrace[0] == funcStr.getPointer())
				{
					entry = e;
					++stackTrace;
					--stackSize;
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}

		entry->breakpointCounter += delta;
		return true;
	}

	uint64_t getTimeStamp() const
	{
		return timer.getMicroseconds();
	}

};

Profiler::Profiler()
:	 data(new Data())
{
}

Profiler::~Profiler()
{
	delete data;
}

void Profiler::setScheduler(task::Scheduler *scheduler)
{
	data->scheduler = scheduler;
}

void Profiler::reset()
{
	temporaryDisableFlag = true;

	ThreadStateCollection::StateArray stateList = threadStateCollection->lockStateList();
	for (ThreadStateCollection::StateArray::Iterator it = stateList.getBegin(); it != stateList.getEnd(); ++it)
	{
		(*it)->root.resetData();
	}

	threadStateCollection->unlockStateList();
	temporaryDisableFlag = false;
}

// TODO: this is not the most optimal solution...
HeapString Profiler::getDump(const DynamicString &stateName, bool curlyScopedFormat)
{
	/*
	State *dumpState = data->getWantedState(stateName);
	HeapString string;
	dumpState->root.outputData(string, 0, curlyScopedFormat);
	*/
	return data->outputData(stateName, curlyScopedFormat);
}

void Profiler::outputData(const DynamicString &stateName)
{
	#if FB_BUILD == FB_FINAL_RELEASE
		//return;
	#endif

	HeapString string = getDump(stateName, false);
	if (data->printDebugOutput)
	{
		FB_PRINTF(string.getPointer());
	}

	FILE *f = fopen("log/performance.log", "wb");
	if (f)
	{
		fprintf(f, "%s", string.getPointer());
		fclose(f);
	}
}

bool Profiler::addBreakPoint(const DynamicString *stackTrace, SizeType stackSize)
{
	return data->modifyBreakpoint(stackTrace, stackSize, 1);
}

bool Profiler::removeBreakPoint(const DynamicString *stackTrace, SizeType stackSize)
{
	return data->modifyBreakpoint(stackTrace, stackSize, -11);
}

void Profiler::setPrintDebugOutput(bool printDebugOutput)
{
	data->printDebugOutput = printDebugOutput;
}

bool Profiler::getPrintDebugOutput() const
{
	return data->printDebugOutput;
}

void Profiler::getMostRecentTimes(MostRecentTimeEntry *entries, SizeType numEntries)
{
	temporaryDisableFlag = true;

	PodVector<Entry::MostRecentTimeEntryInternal> internalEntries;
	internalEntries.resize(numEntries);

	for (SizeType i = 0; i < numEntries; i++)
	{
		fb_assert(entries[i].functionName);
		internalEntries[i].className = entries[i].className;
		internalEntries[i].functionName = entries[i].functionName;
		internalEntries[i].tempSum = 0;
		fb_assert(internalEntries[i].functionName);
	}

	ThreadStateCollection::StateArray stateList = threadStateCollection->lockStateList();
	for (ThreadStateCollection::StateArray::Iterator it = stateList.getBegin(); it != stateList.getEnd(); ++it)
	{
		(*it)->root.getMostRecentTimes(&internalEntries[0], numEntries);
	}

	for (SizeType i = 0; i < numEntries; i++)
	{
		if (getCurrentFrequency() >= 1000)
			entries[i].time = uint64_t(internalEntries[i].tempSum / (getCurrentFrequency() / 1000));
		else
			entries[i].time = uint64_t((internalEntries[i].tempSum * 1000) / getCurrentFrequency());
	}

	threadStateCollection->unlockStateList();
	temporaryDisableFlag = false;
}

#if FB_COMPILER == FB_MSC
#pragma warning( pop )
#endif

FB_END_PACKAGE1()

