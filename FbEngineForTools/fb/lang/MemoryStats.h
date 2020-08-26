#pragma once

FB_PACKAGE1(lang)

enum class MemoryPool: uint32_t
{
	RuntimeHeap = 0,
	FixedPools,
	Amount
};

struct MemoryStats
{
	uint64_t totalAllocationAmount = 0;
	uint64_t totalAllocationAmountInBytes = 0;
	uint64_t currentAllocationAmount = 0;
	uint64_t currentAllocationAmountInBytes = 0;
	uint64_t peakAllocationAmountInBytes = 0;
};

MemoryStats getMemoryStats(MemoryPool pool);

FB_END_PACKAGE1()
