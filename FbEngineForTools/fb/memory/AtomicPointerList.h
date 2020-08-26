#pragma once

#include "fb/lang/Atomics.h"

FB_PACKAGE1(memory)

// Pushing/popping pointers to a list. 
// Pointers themselves are aliased as nodes, so must be able to hold RequiredPointerSizeInBytes of data.
struct AtomicPointerList
{
	// Pushing pointers smaller than RequiredPointerSizeInBytes will corrupt memory
	static const uint32_t RequiredPointerSizeInBytes = sizeof(lang::AtomicPointer);
	lang::AtomicAbaNode headNode;
	lang::AtomicPointer headPointer;
	bool useAtomics = true;
	/* Pad to cache line size */
	char padding[24];

	// Safe interface for multiple threads
	void pushAtomic(void *ptr);
	void pushAtomic(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements);
	void *popAtomic();

	// Relaxed variants which make list somewhat cheaper. NOT multithreading safe.
	void pushRelaxed(void *ptr);
	void pushRelaxed(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements);
	void *popRelaxed();

	// These choose between atomic an drelaxed versions based on boolean toggle
	void push(void *ptr);
	void push(void *ptr, uint32_t sizeInBytes, uint32_t amountInElements);
	void *pop();
};

FB_END_PACKAGE1()
