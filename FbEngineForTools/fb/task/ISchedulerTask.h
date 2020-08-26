#pragma once

#include "fb/lang/IntTypes.h"
#include "fb/lang/MemTools.h"

FB_PACKAGE1(task)

class Scheduler;
class SchedulerTaskData
{
	const char *optionalData;
	uint32_t optionalDataSize; 
	const void *optionalType;

	bool validateAll() const
	{
		fb_assert(optionalDataSize <= MaxDataBytes);
		return optionalData && optionalDataSize && optionalType;
	}

	bool validateNone() const
	{
		return !optionalData && !optionalDataSize && !optionalType;
	}

	bool validate() const
	{
		return (validateAll() || validateNone());
	}

public:

	static uint32_t getNextSchedulerTypeThingy();

	// Maximum amount of task parameter data in bytes.
#if FB_COMPILER == FB_MSC
	static const int MaxDataBytes = 56;
#elif FB_COMPILER == FB_CLANG
	/* Delegate is larger for clang, so allocate extra space */
	static const int MaxDataBytes = 64;
#elif FB_COMPILER == FB_GNUC
	/* No idea how GCC compares to MSC and Clang. If you run into this, update size */
	static const int MaxDataBytes = 56;
#else
#error Unsupported compiler
#endif

	// Default version
	SchedulerTaskData()
	:	optionalData(0)
	,	optionalDataSize(0)
	,	optionalType()
	{
		fb_assert(validateNone());
	}

	// Raw data version
	SchedulerTaskData(const char *data, uint32_t dataSize, const void *type)
	{
		// Note: Does not make a copy
		optionalData = data;
		optionalDataSize = dataSize;
		optionalType = type;

		fb_assert(validate());
	}

	// POD instance version
	template<typename T>
	SchedulerTaskData(const T &instance)
	{
		// Note: Does not make a copy
		optionalData = (const char*) &instance;
		optionalDataSize = sizeof(T);
		optionalType = instance.getType();

		fb_assert(validateAll());
	}

	// Getters

	bool hasData() const
	{
		return validateAll();
	}

	template<typename T>
	void getParam(T &instance) const
	{
		fb_assert(hasData());
		fb_assert(sizeof(T) == optionalDataSize);
		fb_assert(T::getType() == optionalType);
		lang::MemCopy::copy(&instance, optionalData, optionalDataSize);
	}

	// Note: Does not make a copy
	void getParam(const char *&data, uint32_t &dataSize, const void *&type) const
	{
		fb_assert(hasData());

		data = optionalData;
		dataSize = optionalDataSize;
		type = optionalType;
	}
};

// Use this in your ISchedulerTask parameter structs
#define FB_SCHEDULER_TASK_PARAM_TYPE() \
	static const void *getType() \
	{ \
		static uint32_t mySchedulerTypeThingy = task::SchedulerTaskData::getNextSchedulerTypeThingy(); \
		return (const void *)(uintptr_t)mySchedulerTypeThingy; \
	} \

class ISchedulerTask
{
public:
	virtual ~ISchedulerTask() {}

	// Used for profiling etc, so please make it descriptive. Also, avoid using Class::Task format as that doesn't play nice with current profiler implementation
	virtual const char *getStaticTaskNameString() const = 0;

	// data/dataSize contain user data given to scheduler along with the task pointer. This allows running the same task instance with multiple data sets.
	// Scheduler which activated the task is given as parameter, in case more tasks should be spawned. 
	// It is also allowed to add yourself back to queue, with the same or different data.
	// Note: It is guaranteed to be safe for ISchedulerTask to delete itself before returning to scheduler from run().
	virtual void run(const task::SchedulerTaskData &data, task::Scheduler &scheduler) = 0;
};

FB_END_PACKAGE1()
