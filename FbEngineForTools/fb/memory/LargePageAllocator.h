#pragma once

#include "fb/lang/IntTypes.h"

FB_PACKAGE1(memory)

/**
 * LargePageAllocator tries to allocate memory using large pages, thus reducing TLB pressure. Should generally only be 
 * used for permanent (and large) allocations, preferably at the start of app. Minimum supported allocation size can 
 * be checked with getLargePageMinimumSize(), but if you have to ask, you probably don't want to use this.
 *
 * Returned memory is always non-pageable, so allocating huge amounts is a bad idea in general. Maximum that LPA will 
 * allocate depends on amount of physical memory in computer. It can also be set lower on runtime, if there's reason 
 * to suspect that e.g. several instances of the app will be running concurrently.
 *
 * As the memory is non-pageable, there will be physical memory fragmentation, if lot of allocations, reallocations 
 * and deallocations are done. This is especially true if several instances of the app are running (or if there's some 
 * other up doing similar things).
 *
 * Considering all that, it should not come as a surprise that LPA may simply fail without any particular reason. It 
 * may also fail depending on operating system's settings. Always be prepared to ask another allocator if LPA fails.
 *
 * tl;dr: Only use if you know what you are doing.
 */

class LargePageAllocator
{
	/* Private constructor. Use getInstance() instead */
	LargePageAllocator();
	~LargePageAllocator();

public:
	bool isAvailable() const;
	void setEnabled(bool enabled);
	/* Minimum amount of memory that can be requested. Smaller numbers than this will automatically fail. All requests 
	 * will be increased to multiple of this (so requesting minimum size + 1 will actually return minimum size * 2). 
	 * If LPA is disabled or not working for some other reason (like OS settings), this will return maximum uint64_t */
	uint64_t getLargePageMinimumSize() const;
	/* Sets maximum total amount of memory LPA will allocate. Further allocations after the limit is reached will 
	 * return nullptr */
	void setMaxAmountOfAllocatedBytes(uint64_t newAmount);

	/* Allocation and deallocation functions */
	void *allocate(uint64_t sizeInBytes);
	void deallocate(void *buffer);

	uint64_t getAmountOfAllocatedBytes() const;

	static LargePageAllocator &getInstance();

private:
	/* Hidden implementation */
	class Impl;
	Impl &impl;

	/* Non-copyable */
	LargePageAllocator &operator=(const LargePageAllocator&) = delete;
	LargePageAllocator(const LargePageAllocator&) = delete;
};

FB_END_PACKAGE1()
