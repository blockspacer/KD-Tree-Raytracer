#include "Precompiled.h"
#include "FixedSizeAllocatorBase.h"

#include "fb/memory/stats/DebugStats.h"
#include "fb/lang/AlignmentFunctions.h"

FB_PACKAGE1(memory)

	FB_STACK_SET_CLASS(FixedSizeAllocatorBase);

	FixedSizeAllocatorBase::FixedSizeAllocatorBase(SizeType allocationAmount, SizeType allocationSize_, SizeType alignment_, SizeType extraBufferSize_)
		: memoryBuffer(0)
		, memoryBufferSize(0)
		, allocationSize(0)
		, extraBufferSize(0)
		, freeBuffer(1)
	{
		init(allocationAmount, allocationSize_, alignment_, extraBufferSize_);
	}

	FixedSizeAllocatorBase::FixedSizeAllocatorBase(char *memoryBuffer_, SizeType memoryBufferSize_, SizeType allocationSize_, SizeType alignment_)
		: memoryBuffer(memoryBuffer_)
		, memoryBufferSize(memoryBufferSize_)
		, allocationSize(allocationSize_)
		, freeBuffer(0)
	{
		allocationSize = SizeType(lang::alignValue(allocationSize_, alignment_));
	}

	FixedSizeAllocatorBase::FixedSizeAllocatorBase()
		: memoryBuffer(0)
		, memoryBufferSize(0)
		, allocationSize(0)
		, extraBufferSize(0)
		, freeBuffer(1)
	{
	}

	FixedSizeAllocatorBase::~FixedSizeAllocatorBase()
	{
		if (freeBuffer)
			delete[] memoryBuffer;
	}

	static int totalbytes = 0;

	void FixedSizeAllocatorBase::init(SizeType allocationAmount, SizeType allocationSize_, SizeType alignment, SizeType extraBufferSize_)
	{
		allocationSize = (SizeType) lang::alignValue(allocationSize_, alignment);
		extraBufferSize = extraBufferSize_;

		memoryBufferSize = allocationAmount * allocationSize;
		memoryBufferSize += extraBufferSize_;
		totalbytes += memoryBufferSize;
		memoryBuffer = new char[memoryBufferSize];
	}

	SizeType FixedSizeAllocatorBase::getMaxAllocationAmount() const
	{
		if (!memoryBufferSize)
			return 0;

		fb_expensive_assert(memoryBufferSize && allocationSize);
		return (memoryBufferSize - extraBufferSize) / allocationSize;
	}

	char *FixedSizeAllocatorBase::getAllocation(SizeType index) const
	{
		fb_expensive_assert(index < getMaxAllocationAmount());
		return memoryBuffer + (allocationSize * index);
	}

	char *FixedSizeAllocatorBase::getExtraBuffer() const
	{
		fb_assert(extraBufferSize);
		return memoryBuffer + (memoryBufferSize - extraBufferSize);
	}

	SizeType FixedSizeAllocatorBase::getIndex(char *ptr) const
	{
		SizeType offset = (SizeType) (ptr - memoryBuffer);
		fb_expensive_assert(offset % allocationSize == 0);
		fb_expensive_assert(offset < memoryBufferSize - extraBufferSize);

		return offset / allocationSize;
	}

	bool FixedSizeAllocatorBase::doesOwnPointer(char *ptr) const
	{
		/* Freeing NULL is OK and this is called on free, so can't assert (it doesn't matter 
		 * anyway, as far as I can see). */
		//fb_expensive_assert(ptr != NULL);

		if (ptr < memoryBuffer)
			return false;
		if (ptr >= &memoryBuffer[memoryBufferSize - extraBufferSize])
			return false;
		return true;
		//int offset = (int) (ptr - memoryBuffer);
		//if (offset >= 0 && offset < memoryBufferSize - extraBufferSize)
		//	return true;
		//return false;
	}


FB_END_PACKAGE1()
