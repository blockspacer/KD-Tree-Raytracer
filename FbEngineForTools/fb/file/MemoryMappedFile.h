#pragma once

FB_PACKAGE1(file)

/**
 * Memory mapped file is a read-only file that uses OS's file mapping facilities to map contents of a file directly to 
 * memory. On a PC, where disk cache is a thing, this may considerably reduce need for data copying and explicit 
 * memory allocation.
 */
class MemoryMappedFile
{
public:
	MemoryMappedFile();
	MemoryMappedFile(MemoryMappedFile &&other);
	MemoryMappedFile(const StringRef &fileName, bool suppressErrorMessages = false);
	~MemoryMappedFile();

	bool open(const StringRef &fileName, bool suppressErrorMessages = false);
	operator bool() const;
	void reset();
	BigSizeType getSize() const;
	const char *getData() const;

private:
	FB_PIMPL
};

FB_END_PACKAGE1()

#define FB_MEMORY_MAPPED_FILE_SUPPORT FB_TRUE

