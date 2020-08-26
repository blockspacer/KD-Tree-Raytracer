#include "Precompiled.h"
#include "CallstackStorage.h"

#include "fb/container/LinearMap.h"
#include "fb/container/PodVector.h"
#include "fb/container/StringMapFind.h"
#include "fb/lang/logger/GlobalLogger.h"
#include "fb/lang/MemoryFunctions.h"
#include "fb/lang/ProgrammerAssertPrinting.h"
#include "fb/lang/stacktrace/StackTrace.h"
#include "fb/lang/thread/DataGuard.h"
#include "fb/profiling/ZoneProfilerCommon.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"

FB_PACKAGE0()

struct StoreCallstackManager
{
	enum
	{
		MaxCallstackDepth = DumpCallstackEvent::MaxCallstackDepth
	};

	struct SingleCallstack
	{
		lang::CallStackCapture<MaxCallstackDepth> stack;
		uint64_t cpuTimestamp;
		uint32_t threadId;
	};

	struct CallstackInfo
	{
		SizeType count = 0;
		SizeType total = 0;
		SizeType newestValueIndex = 0;
		SizeType offset = 2;
		PodVector<SingleCallstack> stacks;
		PodVector<const char *> messages;

		struct StringRingBuffer
		{
			char *buffer = NULL;
			SizeType head = 0;
			SizeType capacity = 0;
		};
		StringRingBuffer ringBuffer;
	};
	typedef LinearMap<StaticString, CallstackInfo> StackMap;
	StackMap stackMap;
	typedef StackMap::Iterator Iterator;

	CallstackInfo &getCallstackInfo(const StringRef &name)
	{
		Pair<Iterator, bool> pair = StringMapFind::findOrInsertIterator(stackMap, name);
		if (!pair.second)
		{
			return pair.first.getValue();
		}

		StoreCallstackManager::CallstackInfo &info = pair.first.getValue();
		info.offset = 2;
		info.stacks.resize(20);
		info.messages.resize(20, NULL);
		return pair.first.getValue();
	}
};

static ScopedRef<StoreCallstackManager> getStoreCallstackManager()
{
	static DataGuard<StoreCallstackManager> singleton("StoreCallstackManager DataGuard");
	return singleton;
}
typedef ScopedRef<StoreCallstackManager> StoreCallstackManagerRef;

void CallstackStorage::registerCallstackInfo(const StringRef &name, SizeType maxCount, SizeType offset)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	StoreCallstackManager::CallstackInfo &info = StringMapFind::findOrInsert(self->stackMap, name);
	info.offset = offset;
	info.stacks.resize(maxCount);
	info.messages.resize(maxCount, NULL);
	if (info.count > maxCount)
	{
		info.count = 0;
		info.newestValueIndex = 0;
	}
}

void CallstackStorage::storeCallstack(const StringRef &name, const StringRef &message)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	StoreCallstackManager::CallstackInfo &info = self->getCallstackInfo(name);
	SizeType index = info.newestValueIndex;
	info.newestValueIndex = (index + 1) % info.stacks.getSize();
	if (info.count < info.stacks.getSize())
		info.count += 1;
	info.total += 1;

	info.stacks[index].cpuTimestamp = profiling::ZoneTimeStamp::getCpuTimestamp();
	info.stacks[index].threadId = profiling::ZoneThreadId::getZoneThreadId();
	FB_CAPTURE_STACK_TRACE(info.stacks[index].stack);

	// Just storing the message from this point on
	if (message.getPointer() && message.getLength() > 0)
	{
		SizeType messageLength = message.getLength();

		if (info.ringBuffer.buffer)
		{ /* likely */
		}
		else
		{
			// Allocate string ring buffer if it hasn't yet been allocated
			SizeType toBeCapacity = info.stacks.getSize() * 128;
			info.ringBuffer.buffer = (char *)lang::allocateMemory(toBeCapacity);
			info.ringBuffer.capacity = toBeCapacity;
		}

		SizeType totalLength = messageLength + 1;

		if (messageLength > 0 && totalLength <= info.ringBuffer.capacity)
		{
			if (info.ringBuffer.head + totalLength >= info.ringBuffer.capacity)
			{
				info.ringBuffer.head = totalLength;
			}
			else
			{
				info.ringBuffer.head += totalLength;
			}

			if (info.ringBuffer.buffer)
			{
				char *dest = info.ringBuffer.buffer + info.ringBuffer.head - totalLength;
				lang::MemCopy::copy(dest, message.getPointer(), messageLength);
				dest[messageLength] = '\0';
				info.ringBuffer.buffer[info.ringBuffer.head] = 0;
				info.messages[index] = dest;
			}
		}
	}
}

bool CallstackStorage::resolveFrame(const StringRef &name, SizeType index, SizeType depth, lang::StackFrame &stackFrame, const char *&outMessage)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	StoreCallstackManager::Iterator it = StringMapFind::findIterator(self->stackMap, name);
	if (it == self->stackMap.getEnd())
		return false;
	StoreCallstackManager::CallstackInfo &info = it.getValue();
	SizeType offset = info.offset;
	if (offset + depth >= StoreCallstackManager::MaxCallstackDepth)
		return false;
	if (index >= info.count)
		return false;
	SizeType finalIndex = (info.newestValueIndex + info.count - index) % info.count;
	if (offset + depth >= info.stacks[finalIndex].stack.numCapturedFrames)
		return false;

	if (info.messages[finalIndex])
		outMessage = info.messages[finalIndex];
	else
		outMessage = "NO_MESSAGE";

	return lang::resolveCallStackFrame(stackFrame, info.stacks[finalIndex].stack.capturedFrames[offset + depth]);
}

bool CallstackStorage::resolveFrame(const StringRef &name, SizeType index, SizeType depth, HeapString &outFile, HeapString &outFunction, SizeType &outLine, const char *&outMessage)
{
	lang::StackFrame stackFrame;
	if (resolveFrame(name, index, depth, stackFrame, outMessage))
	{
		outFile = stackFrame.file;
		outFunction = stackFrame.function;
		outLine = stackFrame.line;
		return true;
	}
	return false;
}
SizeType CallstackStorage::getFrameCount(const StringRef &name)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	StoreCallstackManager::Iterator it = StringMapFind::findIterator(self->stackMap, name);
	if (it == self->stackMap.getEnd())
		return 0U;
	StoreCallstackManager::CallstackInfo &info = it.getValue();
	return info.count;
}
SizeType CallstackStorage::getFrameDepth(const StringRef &name, SizeType index)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	StoreCallstackManager::Iterator it = StringMapFind::findIterator(self->stackMap, name);
	if (it == self->stackMap.getEnd())
		return 0U;
	StoreCallstackManager::CallstackInfo &info = it.getValue();
	if (index >= info.count)
		return 0U;

	SizeType finalIndex = (info.newestValueIndex + info.count - index) % info.count;
	SizeType offset = info.offset;
	if (offset < info.stacks[finalIndex].stack.numCapturedFrames)
		return info.stacks[finalIndex].stack.numCapturedFrames - offset;
	return 0U;
}

static SizeType lastIndexOf(const char *str, char character)
{
	SizeType hit = 0;
	for (SizeType i = 0; i < 1024; ++i)
	{
		if (str[i] == character)
			hit = i + 1; // Skip the character
		if (str[i] == '\0')
			break;
	}
	return hit;
};

void CallstackStorage::printCallstacks()
{
	StoreCallstackManagerRef self = getStoreCallstackManager();

	for (const StoreCallstackManager::Iterator &pair : self->stackMap)
	{
		const StaticString &name = pair.getKey();
		const StoreCallstackManager::CallstackInfo &info = pair.getValue();

		for (SizeType index = 0; index < info.count; ++index)
		{
			SizeType totalIndex = index + info.total - info.count;
			SizeType finalIndex = (info.newestValueIndex + info.count - index) % info.count;

			{
				TempString tempStr;
				tempStr.doSprintf("%s [%d <= %d <= %d] '%s'", name.getPointer(), info.total - info.count, totalIndex, info.total, info.messages[finalIndex]);
				FB_LOG_INFO_FUNC("", tempStr);
			}

			for (SizeType depth = info.offset; depth < StoreCallstackManager::MaxCallstackDepth; ++depth)
			{
				lang::StackFrame stackFrame;
				if (!lang::resolveCallStackFrame(stackFrame, info.stacks[finalIndex].stack.capturedFrames[depth]))
					break;
				if (stackFrame.line == 0 && !stackFrame.hasFile() && !stackFrame.hasFunction())
					continue;

				TempString str;
				str.doSprintf("%d> %s:%d - %s", depth, stackFrame.file + lastIndexOf(stackFrame.file, '\\'), stackFrame.line, stackFrame.function);
				FB_LOG_INFO_FUNC("", str);
			}
		}
	}
}

void CallstackStorage::getCallStackNames(Vector<StaticString> &outResults)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	for (const StoreCallstackManager::Iterator &pair : self->stackMap)
	{
		if (findIfContains(outResults, pair.getKey()))
			continue;

		outResults.pushBack(pair.getKey());
	}
}

void CallstackStorage::dumpCallstacks(uint64_t nowTimestamp, Vector<DumpCallstackEvent> &dumpResult)
{
	StoreCallstackManagerRef state = getStoreCallstackManager();
	double conversionMult = profiling::ZoneTimeStamp::convertCpuTimestampToSeconds(1);
	double nowTime = nowTimestamp * conversionMult;
	uint64_t start = nowTimestamp - profiling::ZoneTimeStamp::convertSecondsToCpuTimestamp(60);
	for (StoreCallstackManager::StackMap::ConstIterator callstackInfo : state->stackMap)
	{
		StaticString name = callstackInfo.getKey();

		for (SizeType i = 0; i < callstackInfo.getValue().stacks.getSize(); ++i)
		{
			const StoreCallstackManager::SingleCallstack &e = callstackInfo.getValue().stacks[i];
			if (e.cpuTimestamp > nowTimestamp || e.cpuTimestamp < start)
				continue;

			const char *messageStr = "";
			if (i < callstackInfo.getValue().messages.getSize())
				messageStr = callstackInfo.getValue().messages[i];

			DumpCallstackEvent &t = dumpResult.pushBack();
			t.name = name;
			t.message = messageStr;
			t.time = (e.cpuTimestamp) * conversionMult - nowTime;
			t.thread = e.threadId;
			t.stack = e.stack;
		}
	}
}

void CallstackStorage::appendSingleRowCallstack(HeapString &outputStr, const StringRef &name, SizeType index)
{
	StoreCallstackManagerRef self = getStoreCallstackManager();
	const StoreCallstackManager::CallstackInfo &info = self->getCallstackInfo(name);

	if (info.count == 0)
		return;

	SizeType finalIndex = (info.newestValueIndex + info.count - index) % info.count;
	if (finalIndex >= info.stacks.getSize() || info.stacks[finalIndex].stack.numCapturedFrames <= 0)
		return;

	if (finalIndex < info.messages.getSize() && info.messages[finalIndex])
		outputStr << info.messages[finalIndex];

	ProgrammerAssertPrinting::appendStackTraceSingleRow(outputStr, info.stacks[finalIndex].stack);
}

FB_END_PACKAGE0()
