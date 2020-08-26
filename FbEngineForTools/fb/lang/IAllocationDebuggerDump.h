#ifndef FB_LANG_LOWLEVELMEMORY_DEBUGGER_H
#define FB_LANG_LOWLEVELMEMORY_DEBUGGER_H

#include "fb/lang/IntTypes.h"

FB_DECLARE_STRUCT(lang, StackFrame)

FB_PACKAGE1(lang)

class IAllocationDebuggerDump
{
public:
	virtual ~IAllocationDebuggerDump() {}

	virtual uint32_t getOverheadPerAllocation() const = 0;

	/* Allocation tracker info */
	virtual SizeType getNumAllocationPathTrackers() const = 0;
	virtual SizeType getAPTNumStackFrames(SizeType index) const = 0;
	virtual SizeType getAPTRecommendedStackFramesToIgnore() const = 0;
	virtual const StackFrame &getAPTStackFrame(SizeType index, SizeType stackFrameIndex) const = 0;
	virtual uint64_t getAPTNumAllocations(SizeType index) const = 0;
	virtual uint64_t getAPTNumDeallocations(SizeType index) const = 0;
	virtual uint64_t getAPTMaxNumLiveAllocations(SizeType index) const = 0;
	virtual uint64_t getAPTAllocatedBytes(SizeType index) const = 0;
	virtual uint64_t getAPTDeallocatedBytes(SizeType index) const = 0;
	virtual uint64_t getAPTMaxLiveAllocatedBytes(SizeType index) const = 0;
};


FB_END_PACKAGE1()

#endif
