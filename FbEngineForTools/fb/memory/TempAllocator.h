#pragma once

FB_PACKAGE1(memory)

	/**
	 * This allocator is intentionally extremely limited. It's by no means meant to be a general one.
	 * Main use case is to allocate a big buffer, and use this to dynamically get a few big chunks out of it.
	 * Any error cases will lead to assertion failures, and no solid fallback behaviors are supported.
	 * This ment to be a lowlevel solution, so you've been warned.
	 *
	 * For example, filesystem should have big buffer which is divided for opened files, to avoid big runtime allocations.
	 */
	class TempAllocator
	{
		// Not defined
		TempAllocator(const TempAllocator &);
		void operator= (const TempAllocator &);

		void defrag();

	public:
		TempAllocator(char *buffer, SizeType bufferSize);
		TempAllocator(SizeType bufferSize);
		~TempAllocator();

		char *allocate(SizeType size);
		void deallocate(char *ptr);

		// Maximum amount of allocations we support in single TempAllocator
		static const SizeType MaxAllocations = 12;

	private:
		char *memoryBuffer;
		SizeType memoryBufferSize;
		SizeType allocationOffset[MaxAllocations];
		SizeType allocationSize[MaxAllocations];
		bool freeMemoryBuffer;
	};

FB_END_PACKAGE1()
