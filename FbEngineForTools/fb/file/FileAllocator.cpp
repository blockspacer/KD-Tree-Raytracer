#include "Precompiled.h"
#include "FileAllocator.h"

#include "fb/lang/CallStack.h"
#include "fb/lang/thread/Mutex.h"
#include "fb/memory/GlobalFixedAllocator.h"
#include "fb/memory/IBlockAllocator.h"
#include "fb/memory/TempAllocator.h"

// These memory limits are here for a reason.
// Especially for consoles, if you hit problems don't just double the buffer ...
/* Oh, yeah, totally for 'I just doubled these numbers because why not' reasons based on the commits
 * If some one actually has any idea about these, please fix these comments to actually explain this stuff */

// It would be nice, but we can't really make any assumptions about memory usage in development builds
#define USE_FILE_ALLOCATOR
static const int FileAllocatorMemory = 64 * 1024 * 1024;  // Should be enough for EveryOne(tm)

FB_PACKAGE1(file)

namespace {
	
	#ifdef USE_FILE_ALLOCATOR
		memory::TempAllocator *tempAllocator = 0;

		static Mutex mutex;
		struct MutexGuard
		{
			MutexGuard()
			{
				mutex.enter();
			}

			~MutexGuard()
			{
				mutex.leave();
			}
		};

	#endif
}

class TempAllocatorFileBuffer : public FileBuffer
{
	// Not implemented
	TempAllocatorFileBuffer(const TempAllocatorFileBuffer &);
	void operator = (const TempAllocatorFileBuffer &);

public:
	TempAllocatorFileBuffer(char *data)
	:	FileBuffer(data)
	{
	}
	~TempAllocatorFileBuffer()
	{
#ifdef USE_FILE_ALLOCATOR
		MutexGuard guard;
		tempAllocator->deallocate(data);
#endif
		data = 0;
	}
};

FileBufferPointer getFileBuffer(BigSizeType size, const char *filename)
{
	FB_STACK_FUNC();

	#ifdef USE_FILE_ALLOCATOR
	{
		MutexGuard guard;

		if (!tempAllocator)
		{
			FB_STACK_CUSTOM("FileAllocator init");

			// Manual buffer
			memory::IBlockAllocator *systemAllocator = getSystemHeapAllocator();
			tempAllocator = new memory::TempAllocator((char*) systemAllocator->allocate(FileAllocatorMemory), FileAllocatorMemory);
		}

		char *pointer = tempAllocator->allocate(SizeType(size));
		if (pointer)
		{
			FileBufferPointer result(new TempAllocatorFileBuffer(pointer));
			return result;
		}
	}
	#endif

	// Use standard memory management - also if the allocator above fails (it will assert, but at least it won't crash in final release)
	fb_assert(size < fb::BigSizeType(1) << 31);
	char *pointer = new char[SizeType(size)];
	if (pointer)
	{
		FileBufferPointer result(new FileBuffer(pointer));
		return result;
	}

	fb_assert(false && "Completely out of memory in FileAllocator?");
	return FileBufferPointer();
}

FileBufferPointer getEmptyBuffer()
{
	return FileBufferPointer();
}

FB_END_PACKAGE1()
