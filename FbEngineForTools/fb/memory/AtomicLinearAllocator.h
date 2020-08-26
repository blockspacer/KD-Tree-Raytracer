#pragma once

FB_DECLARE(memory, IBlockAllocator);

#include "IAllocator.h"
#include "fb/lang/Atomics.h"
#include "fb/lang/thread/Mutex.h"

FB_PACKAGE1(memory)

	// Wrap block/heap allocator to simple atomic linear allocator.
	// Atomicy only applies within individual memory buffers, swapping buffers triggers a mutex.
	// Chains allocated blocks to linked list to support arbitrary amount of them.
	// Passing NULL blockAllocator uses generic heap instead.
	//   reset() starts to overwrite previous allocations so user beware.
	//   deallocate() is NOP.
	class AtomicLinearAllocator: public IAllocator
	{
		AtomicLinearAllocator(const AtomicLinearAllocator &);
		void operator = (const AtomicLinearAllocator &);

	public:
		AtomicLinearAllocator(IBlockAllocator *allocator, SizeType blockSizeInBytes, uint32_t initialBlockAmount, bool expandDynamically);
		~AtomicLinearAllocator();

		// Start reusing previously allocated buffers. 
		// User responsibility to make sure previous allocations are no longer used.
		void reset();

		virtual void *allocate(uint64_t size) final;
		virtual void deallocate(void *pointer, uint64_t size = 0) final;
		void *allocateAligned(uint64_t size, SizeType alignment);

	private:
		void getNextBlockImp();
		void allocateNewBlockImp();

		IBlockAllocator *blockAllocator = NULL;
		struct Buffer
		{
			lang::AtomicPointer nextBuffer;
			lang::AtomicUInt32 positionInBytes;
			char *pointer;
		};
		Buffer *firstBuffer = NULL;
		lang::AtomicPointer currentBuffer;
		Mutex mutex;
		SizeType totalMemoryAllocatedInBytes = 0;
		uint32_t blockSizeInBytes = 0;
		bool expandDynamically = false;
		static const uint32_t OverheadInBytes = sizeof(Buffer);
	};

FB_END_PACKAGE1()
