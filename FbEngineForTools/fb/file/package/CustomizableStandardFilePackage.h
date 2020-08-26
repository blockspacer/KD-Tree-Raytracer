#pragma once

#include "IFilePackage.h"
#include "fb/lang/Pimpl.h"
#include "fb/string/HeapString.h"

FB_PACKAGE2(file, package)


/**
 * This is like the (deprecated) StandardFilePackage, but adds some filtering support and such.
 *
 * For performance considerations, this has been implemented as a separate complex file package, rather than 
 * by using the tidier separate filtering layer solution on top of the StandardFilePackage.
 */
class CustomizableStandardFilePackage : public IFilePackage
{
public:
#pragma region FilePackageFilter
	class FilePackageFilter
	{
	public:
		FilePackageFilter();
		~FilePackageFilter();

		void addBasePathToExcludeList(const DynamicString &basePath);

		/**
		 * Any directory (and dirs/files under it) that has this name will be ignored.
		 */
		void addSubPathToExcludeList(const DynamicString &path);

		bool isPathExcluded(const StringRef &path);

	protected:
		Vector<DynamicString> excludeBasePathList;
		Vector<DynamicString> excludeSubPathList;
	};
#pragma endregion

	CustomizableStandardFilePackage(const DynamicString &basePath, const FilePackageFilter &filter);
	~CustomizableStandardFilePackage();

	File openFile(const DynamicString &fileName, bool archiveOnly) override;
	bool openFiles(const FileNameVector &names, FileVector &outFiles) override;

	bool doesFileExist(const DynamicString &fileName) override;
	bool doesFileExistInArchive(const DynamicString &fileName) override { return false; }
	file::TimeStamp64 getFileTimeStamp(const DynamicString &fileName) override;

	BigSizeType getFileSize(const DynamicString &fileName) override;
	BigSizeType getFileSizeOnDisk(const DynamicString &fileName) override;
	bool writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath) override;
	bool deleteFile(const DynamicString &fileName, bool suppressErrorMessages) override;
	void listFiles(FileList &results, const StringRef &dir, FileFlagMask flags) override;
	void sortFiles(FileSortInfo *pointer, SizeType files) override;

	bool resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const override;
	bool resolveFile(const DynamicString &fileName, DynamicString &resultFilename, uint32_t &offset, uint32_t *size = 0) override;
	bool resolvePackage(const DynamicString &fileName, ResolveData &resolveData) override;

	bool isMemoryResident() const override { return false; }
	/* Not caching changes */
	void waitForFileSystem() override { }

	void listFiles(Vector<DynamicString> &files) override
	{
		fb_assert(0 && "Not implemented!");
	}

#if FB_TRACK_UNUSED_FILES == FB_TRUE
	virtual void dumpUnusedFiles(file::DebugBlockWriter &writer, file::DebugBlockWriter &writer2) override { }
#endif

	FB_PIMPL;
};

FB_END_PACKAGE2()
