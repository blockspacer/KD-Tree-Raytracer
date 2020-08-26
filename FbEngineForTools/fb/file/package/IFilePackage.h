#pragma once

#include "fb/container/LinearHashMap.h"
#include "fb/container/Vector.h"
#include "fb/file/File.h"
#include "fb/file/FileFlag.h"
#include "fb/file/TimeStamp.h"
#include "fb/lang/IntTypes.h"
#include "fb/string/DynamicString.h"
#include "fb/string/StringHash.h"

//#define FB_TRACK_UNUSED_FILES FB_TRUE
#define FB_TRACK_UNUSED_FILES FB_FALSE

FB_DECLARE(file, DebugBlockWriter);
FB_DECLARE_STRUCT(file, FileSortInfo)

FB_PACKAGE2(file, package)

struct FileResolveData
{
	/* A byte offset to file data in some internal data structure (like backing archive file on disk). May be zero
	 * if not applicable (or uninteresting). May be same for multiple files in some cases */
	uint64_t offset = 0;
	/* Size of file in internal data structure. If not applicable, should be the same as size */
	uint64_t rawSize = 0;
	/* Size of file if read (after decompression, may be same as rawSize) */
	uint64_t size = 0;
	/* Whether or not the file resides in memory */
	bool memoryResident = false;

	/* Note: Will return huge hole value if order of files is wrong */
	static BigSizeType calculateHoleSize(const FileResolveData &a, const FileResolveData &b)
	{
		return b.offset - (a.offset + a.rawSize);
	}

	BigSizeType calculateHoleSize(const FileResolveData &other) const
	{
		return calculateHoleSize(*this, other);
	}

	static bool areConsecutive(const FileResolveData &a, const FileResolveData &b, BigSizeType allowedHole = 0)
	{
		/* This math should work nicely for unsigned values */
		return calculateHoleSize(b, a) <= allowedHole;
	}

	bool isConsecutiveToThis(const FileResolveData &other, BigSizeType allowedHole = 0) const
	{
		return areConsecutive(*this, other);
	}
};


class IFilePackage
{
public:
	virtual ~IFilePackage() {}

	virtual File openFile(const DynamicString &fileName, bool archiveOnly = false) = 0;
	/* For opening several files with one call. This is meant to speed up sequential file access for 
	 * MysteryFilePackages and similar. Returns true, if opening all the files succeeded, false otherwise. Will always 
	 * populate outFiles with as many items (in the same order) as names has */
	typedef Vector<DynamicString> FileNameVector;
	typedef Vector<File> FileVector;
	virtual bool openFiles(const FileNameVector &names, FileVector &outFiles) = 0;

	virtual bool doesFileExist(const DynamicString &fileName) = 0;
	virtual bool doesFileExistInArchive(const DynamicString &fileName) = 0;
	virtual file::TimeStamp64 getFileTimeStamp(const DynamicString &fileName) = 0;

	virtual BigSizeType getFileSize(const DynamicString &fileName) = 0;
	virtual BigSizeType getFileSizeOnDisk(const DynamicString &fileName) = 0;
	virtual bool writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath) = 0;
	/* Deletes file. Returns true on success, false if file is not found or some other error occurs. Parameter 
	 * suppressErrorMessages can be used to prevent errors being written to log. It does not affect return value */
	virtual bool deleteFile(const DynamicString &fileName, bool suppressErrorMessages = false) = 0;
	virtual void sortFiles(FileSortInfo *pointer, SizeType files) = 0;

	typedef LinearHashMap<DynamicString, IFilePackage *> FileList;
	virtual void listFiles(FileList &results, const StringRef &dir, FileFlagMask flags) = 0;

	virtual void listFiles(Vector<DynamicString> &files) = 0;

	/* Resolves file. See FileResolveData for more details. If file is not found, returns false */
	virtual bool resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const = 0;

	/* Resolve given file name to matching raw file name/offset pair (if it is located uncompressed inside the archive) */
	virtual bool resolveFile(const DynamicString &fileName, DynamicString &resultFilename, uint32_t &offset, uint32_t *size = 0) = 0;

	struct ResolveData
	{
		IFilePackage *handler = nullptr;
		uint32_t internalData1 = 0;
		uint32_t internalData2 = 0;
	};
	virtual bool resolvePackage(const DynamicString &fileName, ResolveData &resolveData) = 0;

	virtual bool isMemoryResident() const = 0;

	virtual void waitForFileSystem() = 0;

#if FB_TRACK_UNUSED_FILES == FB_TRUE
	virtual void dumpUnusedFiles(file::DebugBlockWriter &writer, file::DebugBlockWriter &writer2) = 0;
#endif
};

FB_END_PACKAGE2()
