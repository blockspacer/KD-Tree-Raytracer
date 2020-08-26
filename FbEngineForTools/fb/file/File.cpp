#include "Precompiled.h"
#include "File.h"

FB_PACKAGE1(file)

FileBuffer::FileBuffer(char *data)
	: data(data)
{
}


#if FB_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE

	FileBuffer::FileBuffer(MemoryMappedFile &&mFile)
		: mFile(lang::move(mFile))
		, data(nullptr)
	{
	}

#endif


FileBuffer::~FileBuffer()
{
	fb_assert(lang::atomicLoadRelaxed(refCount) == 0);
	delete[] data;
}


void FileBuffer::increaseRefCount()
{
	lang::atomicIncRelaxed(refCount);
}


void FileBuffer::decreaseRefCount()
{
	// release/acquire semantics based on boost
	int oldValue = lang::atomicDecRelease(refCount);
	fb_assert(oldValue >= 1);
	if (oldValue == 1)
	{
		lang::atomicThreadFenceAcquire();
		delete this;
	}
}


const char *FileBuffer::getData() const
{
	#if FB_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		fb_assert(!data || !mFile && "Cannot have both valid data pointer and memory mapped file");
		return mFile ? mFile.getData() : data;
	#else
		return data;
	#endif
}


char *FileBuffer::getMutableData() const
{
	#if FB_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		fb_assert(!mFile && "Cannot get mutable data pointer from memory mapped file");
	#endif
	return data;
}


FB_END_PACKAGE1()
