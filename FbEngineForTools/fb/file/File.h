#pragma once

#include "MemoryMappedFile.h"

#include "fb/lang/Types.h"
#include "fb/lang/Atomics.h"

FB_PACKAGE1(file)

class FileBuffer
{
	// Not implemented
	FileBuffer(const FileBuffer &);
	void operator = (const FileBuffer &);

public:
	FileBuffer(char *data);
	#if FB_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		FileBuffer(MemoryMappedFile &&mFile);
	#endif
	virtual ~FileBuffer();

	void increaseRefCount();
	void decreaseRefCount();

	const char *getData() const;
	char *getMutableData() const;

protected:
	lang::AtomicInt32 refCount;
	#if FB_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		MemoryMappedFile mFile;
	#endif
	char *data = nullptr;
};

class FileBufferPointer
{
public:
	FileBufferPointer()
		: fileBuffer(nullptr)
	{
	}

	FileBufferPointer(FileBuffer *fileBuffer)
		: fileBuffer(fileBuffer)
	{
		fileBuffer->increaseRefCount();
	}

	FileBufferPointer(const FileBufferPointer &other)
		: fileBuffer(other.fileBuffer)
	{
		if (fileBuffer)
			fileBuffer->increaseRefCount();
	}

	~FileBufferPointer()
	{
		if (fileBuffer)
			fileBuffer->decreaseRefCount();
	}

	FileBufferPointer &operator=(const FileBufferPointer &other)
	{
		// Aargh!1! This had the usual refcount error.
		// Decreasing ref count BEFORE incrementing the new one.
		// With equal values -> kaboom
		if (fileBuffer == other.fileBuffer)
			return *this;

		if (fileBuffer)
			fileBuffer->decreaseRefCount();

		fileBuffer = other.fileBuffer;
		if (fileBuffer)
			fileBuffer->increaseRefCount();

		return *this;
	}

	void reset(FileBuffer *ptr = nullptr)
	{
		// Aargh!1! This had the usual refcount error.
		// Decreasing ref count BEFORE incrementing the new one.
		// With equal values -> kaboom
		if (fileBuffer == ptr)
			return;

		if (fileBuffer)
			fileBuffer->decreaseRefCount();

		fileBuffer = ptr;
		if (fileBuffer)
			fileBuffer->increaseRefCount();
	}
	
	const char *get() const { return fileBuffer ? fileBuffer->getData() : nullptr; }
	char *getMutable() const { return fileBuffer ? fileBuffer->getMutableData() : nullptr; }
	
	// implicit conversion to bool
	typedef FileBuffer * FileBufferPointer::*OperatorBoolHack;
	operator OperatorBoolHack() const
	{
		return fileBuffer ? &FileBufferPointer::fileBuffer : nullptr;
	}

private:
	FileBuffer *fileBuffer;
};

/**
 * Holds file data
 */
class File
{
public:
	File()
		: dataSize(0)
	{
	}

	File(const FileBufferPointer &data, BigSizeType dataSize)
		: data(data)
		, dataSize(dataSize)
	{
	}

	/**
	 * Returns pointer to file data
	 */
	const char *getData() const
	{
		return data.get();
	}

	/**
	 * Returns number of bytes in file
	 */
	BigSizeType getSize() const
	{
		return dataSize;
	}

	/**
	 * Release the file
	 */
	void release()
	{
		data.reset();
		dataSize = 0;
	}

	/**
	 * Returns true if file is valid
	 */
	operator bool() const
	{
		return data.get() != 0;
	}

private:
	FileBufferPointer data;
	BigSizeType dataSize;
};

FB_END_PACKAGE1()
