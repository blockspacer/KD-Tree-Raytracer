#include "Precompiled.h"

#include "MemoryMappedFile.h"

#include "fb/lang/DebugHelp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

class MemoryMappedFile::Impl
{
public:
	
	HANDLE fileHandle = INVALID_HANDLE_VALUE;
	HANDLE fileMappingHandle = INVALID_HANDLE_VALUE;
	BigSizeType size = 0;
	char *dataPtr = nullptr;
};


MemoryMappedFile::MemoryMappedFile()
	: impl(new Impl())
{
}


MemoryMappedFile::MemoryMappedFile(MemoryMappedFile &&other)
	: impl(new Impl())
{
	impl.swap(other.impl);
}


MemoryMappedFile::MemoryMappedFile(const StringRef &fileName, bool suppressErrorMessages)
	: impl(new Impl())
{
	open(fileName, suppressErrorMessages);
}


MemoryMappedFile::~MemoryMappedFile()
{
	reset();
}


bool MemoryMappedFile::open(const StringRef &fileName, bool suppressErrorMessages)
{
	reset();
	string::SimpleUTF16String fileNameWide;
	fileNameWide.appendUTF8(fileName);
	impl->fileHandle = CreateFileW(fileNameWide.getPointer(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (impl->fileHandle == INVALID_HANDLE_VALUE)
	{
		if (!suppressErrorMessages)
		{
			TempString msg("Failed to open file ");
			msg << fileName << " for memory mapped file access. ";
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
		return false;
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(impl->fileHandle, &fileSize))
	{
		if (!suppressErrorMessages)
		{
			TempString msg("Failed to get file ");
			msg << fileName << " size for memory mapped file access. ";
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
		reset();
		return false;
	}
	impl->size = BigSizeType(fileSize.QuadPart);
	
	impl->fileMappingHandle = CreateFileMappingW(impl->fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	if (impl->fileMappingHandle == INVALID_HANDLE_VALUE)
	{
		if (!suppressErrorMessages)
		{
			TempString msg("Failed to create file ");
			msg << fileName << " mapping for memory mapped file access. ";
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
		reset();
		return false;
	}

	impl->dataPtr = reinterpret_cast<char*>(MapViewOfFile(impl->fileMappingHandle, FILE_MAP_READ, 0, 0, impl->size));
	if (!impl->dataPtr)
	{
		if (!suppressErrorMessages)
		{
			TempString msg("Failed to map view of file ");
			msg << fileName << " for memory mapped file access. ";
			DebugHelp::addGetLastErrorCode(msg);
			FB_LOG_ERROR(msg);
		}
		reset();
		return false;
	}
	return true;
}


MemoryMappedFile::operator bool() const
{
	return getData() != nullptr;
}


void MemoryMappedFile::reset()
{
	if (impl->dataPtr)
		UnmapViewOfFile(impl->dataPtr);

	if (impl->fileMappingHandle != INVALID_HANDLE_VALUE)
		CloseHandle(impl->fileMappingHandle);
	
	if (impl->fileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(impl->fileHandle);
	
	impl->dataPtr = nullptr;
	impl->fileMappingHandle = INVALID_HANDLE_VALUE;
	impl->fileHandle = INVALID_HANDLE_VALUE;
	impl->size = 0;
}


BigSizeType MemoryMappedFile::getSize() const
{
	return impl->size;
}


const char *MemoryMappedFile::getData() const
{
	return impl->dataPtr;
}

FB_END_PACKAGE1()
