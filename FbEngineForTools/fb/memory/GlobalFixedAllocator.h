#pragma once

FB_DECLARE(memory, IBlockAllocator);

FB_PACKAGE0()
memory::IBlockAllocator *getSystemHeapAllocator();
FB_END_PACKAGE0()

FB_PACKAGE2(memory, fixedallocator)

	// Use this if static init is actually running on different thread.
	void setCurrentThreadAsMain();
	void destruct();
	uint64_t getCurrentAllocationBytes();

	class GlobalFixedAllocatorScope
	{
	public:
		GlobalFixedAllocatorScope() { }
		~GlobalFixedAllocatorScope() { memory::fixedallocator::destruct(); }
	};

FB_END_PACKAGE2()
