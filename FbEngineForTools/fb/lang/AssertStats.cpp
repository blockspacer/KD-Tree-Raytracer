#include "Precompiled.h"
#include "fb/lang/FBAssert.h" // NOTE: Must be included before AssertStats.h to have FB_ASSERT_COUNTER_ENABLED be defined
#include "AssertStats.h"

#include "fb/container/PodVector.h"
#include "fb/lang/FastCopy.h"
#include "fb/lang/thread/MutexGuard.h"

#include <stdlib.h> 

FB_PACKAGE2(lang, asserts)

class AssertStats::Impl
{
public:
	Impl()
		: reenterCount(0)
	{
		counters.reserve(1024);
	}

	~Impl()
	{
		AssertStats::getSingleton().prepareForDoom();
	}

	uint32_t reenterCount;
	Mutex mutex;
	PodVector<AssertCounterRegisterable> counters;

	enum Constants
	{
		StaticCounterCapacity = 32
	};
	static AssertCounterRegisterable staticCounters[StaticCounterCapacity];
	static uint32_t numStaticCounters;
};

AssertStats::AssertCounterRegisterable AssertStats::Impl::staticCounters[StaticCounterCapacity];
uint32_t AssertStats::Impl::numStaticCounters = 0;

bool AssertStats::isBeingInitialized = false;
bool AssertStats::hasBeenDoomed = false;


AssertStats::AssertCounterRegisterable::AssertCounterRegisterable(const char * predicate, const char * functionName, const char * fileName, uint32_t lineNumber, FB_ASSERT_COUNTER_COUNTER_TYPE *callCount)
	: predicate(predicate)
	, functionName(functionName)
	, fileName(fileName)
	, lineNumber(lineNumber)
	, callCount(callCount)
{
}


AssertStats::AssertCounter::AssertCounter(AssertCounterRegisterable & registerableCounter)
	: predicate(registerableCounter.getPredicate())
	, functionName(registerableCounter.getFunctionName())
	, fileName(registerableCounter.getFileName())
	, lineNumber(registerableCounter.getLineNumber())
	, callCount(registerableCounter.getCallCount())
{
}


AssertStats::AssertStats()
	: impl(NULL)
{

}


AssertStats::~AssertStats()
{
	AssertStats::hasBeenDoomed = true;
	impl = NULL;
}


bool AssertStats::registerAssertCounter(const AssertCounterRegisterable &assertCounter)
{
	if (impl == NULL)
	{
		/* NULL impl should only happen during initial initialization or just before death. */
		if (Impl::numStaticCounters >= Impl::StaticCounterCapacity)
			return false;

		Impl::staticCounters[Impl::numStaticCounters] = assertCounter;
		++Impl::numStaticCounters;
		return true;
	}

	MutexGuard guard(impl->mutex);

	++impl->reenterCount;
	if (impl->reenterCount == 1)
	{
		impl->counters.pushBack(assertCounter);
	}
	else
	{
		if (impl->numStaticCounters < Impl::StaticCounterCapacity)
		{
			Impl::staticCounters[Impl::numStaticCounters] = assertCounter;
			++Impl::numStaticCounters;
		}
		else
		{
			/* Whoops, we are reentering more than expected. Probably a bug of some sort, unless this happening at exit */
			--impl->reenterCount;
			return false;
		}
	}
	--impl->reenterCount;
	if (impl->reenterCount == 0)
	{
		while (Impl::numStaticCounters != 0)
		{
			AssertCounterRegisterable staticCountersCopy[Impl::StaticCounterCapacity];
			lang::fastSmallMemoryCopy(&staticCountersCopy[0], &Impl::staticCounters[0], sizeof(Impl::staticCounters));
			uint32_t numStaticCountersCopy = Impl::numStaticCounters;
			Impl::numStaticCounters = 0;
			for (uint32_t i = 0; i < numStaticCountersCopy; ++i)
			{
				registerAssertCounter(staticCountersCopy[i]);
			}
		}
	}
	return true;
}


uint32_t AssertStats::getNumCounters() const
{
	if (impl == NULL)
		return 0;

	return impl->counters.getSize();
}


void AssertStats::getCounters(PodVector<AssertCounter>& outCounters) const
{
	if (impl == NULL)
		return;

	MutexGuard guard(impl->mutex);
	for (SizeType i = 0, num = impl->counters.getSize(); i < num; ++i)
		outCounters.pushBack(AssertCounter(impl->counters[i]));
}


void AssertStats::clearAssertCounts()
{
	if (impl == NULL)
		return;

	MutexGuard guard(impl->mutex);

	for (SizeType i = 0, num = impl->counters.getSize(); i < num; ++i)
		*impl->counters[i].callCount = 0;
}


void AssertStats::prepareForDoom()
{
	if (impl != NULL)
	{
		hasBeenDoomed = true;
		Impl* implCopy = impl;
		impl = NULL;
		implCopy->counters.clear();
		implCopy->counters.trimMemory();
	}
}


AssertStats &AssertStats::getSingleton()
{
	/* Done like this to avoid memory allocation on first call, which would cause asserts to be registered */
	static AssertStats assertStats;
	static uint32_t numCalls = 0;
	++numCalls;
	/* Delay actual initialization so we can get over static initialization of  */
	if (assertStats.impl != NULL || isBeingInitialized || hasBeenDoomed || numCalls < Impl::StaticCounterCapacity / 2)
		return assertStats;

	isBeingInitialized = true;
	static Impl impl;
	assertStats.impl = &impl;
	isBeingInitialized = false;
	return assertStats;
}

FB_END_PACKAGE2()
