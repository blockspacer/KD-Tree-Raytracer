#pragma once

#include "fb/lang/FBPrintf.h"
#include "fb/lang/MinMax.h"
#include "fb/lang/Types.h"

FB_PACKAGE1(lang)

struct StackFrame
{
	StackFrame()
		: line(0)
		, displacement(0)
	{
		file[0] = '\0';
		function[0] = '\0';
	}

	bool hasFile() const
	{
		return file[0] != '\0';
	}

	bool hasFunction() const
	{
		return function[0] != '\0';
	}

	const static SizeType MaxFilenameLength = 256;
	const static SizeType MaxFunctionNameLength = 512;

	char file[MaxFilenameLength];
	char function[MaxFunctionNameLength];
	uint32_t line;
	uint64_t displacement;
};

typedef void* CallStackFrameCapture;

struct CallStackCaptureBase
{
	SizeType getMaxDepth() const
	{
		return bufferCapacity;
	}
	bool operator<(const CallStackCaptureBase &other) const
	{
		if (this->numCapturedFrames != other.numCapturedFrames)
			return this->numCapturedFrames < other.numCapturedFrames;

		for (uint32_t i = 0, num = lang::min(bufferCapacity, numCapturedFrames); i < num; ++i)
		{
			if (this->getCapturedFrames()[i] != other.getCapturedFrames()[i])
				return this->getCapturedFrames()[i] < other.getCapturedFrames()[i];
		}
		/* They are equal */
		return false;
	}

	bool operator==(const CallStackCaptureBase &other) const
	{
		if (this->numCapturedFrames != other.numCapturedFrames)
			return false;

		for (uint32_t i = 0, num = lang::min(bufferCapacity, numCapturedFrames); i < num; ++i)
		{
			if (this->getCapturedFrames()[i] != other.getCapturedFrames()[i])
				return false;
		}
		return true;
	}

	inline CallStackFrameCapture *getCapturedFrames();
	inline const CallStackFrameCapture *getCapturedFrames() const;

	uint32_t numCapturedFrames = 0;

protected:

	// Use CallStackCapture
	CallStackCaptureBase(SizeType bufferCapacity)
		: bufferCapacity(bufferCapacity)
	{
	}

private:
	uint32_t bufferCapacity = 0;
};

template<uint32_t BufferCapacity>
struct CallStackCapture : public CallStackCaptureBase
{
	CallStackCapture()
		: CallStackCaptureBase(BufferCapacity)
	{
	}

	static const uint32_t maxDepth = BufferCapacity;
	CallStackFrameCapture capturedFrames[BufferCapacity];
};

inline CallStackFrameCapture *CallStackCaptureBase::getCapturedFrames()
{
	return static_cast<CallStackCapture<1>*>(this)->capturedFrames;
}
inline const CallStackFrameCapture *CallStackCaptureBase::getCapturedFrames() const
{
	return static_cast<const CallStackCapture<1>*>(this)->capturedFrames;
}


bool resolveCallStackFrame(StackFrame& outFrame, const CallStackFrameCapture &capturedFrame);

void debugDumpCallStack(const CallStackCaptureBase &capture, const char *prefix="");


uint32_t captureCallStackImpl(CallStackFrameCapture *frameStore, uint32_t maxDepth, uint32_t skipCount);

#define FB_CAPTURE_STACK_TRACE(callStackCapture) \
	callStackCapture.numCapturedFrames = lang::captureCallStackImpl(callStackCapture.getCapturedFrames(), callStackCapture.getMaxDepth(), 1);

// Use skipping version when possible to avoid wasting callstack space in the result for stuff that is just going to be thrown away
#define FB_CAPTURE_STACK_TRACE_WITH_SKIPPING(callStackCapture, skipCount) \
	callStackCapture.numCapturedFrames = lang::captureCallStackImpl(callStackCapture.getCapturedFrames(), callStackCapture.getMaxDepth(), skipCount + 1);

FB_END_PACKAGE1()
