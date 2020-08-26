#pragma once

#include "CustomizableStandardFilePackage.h"

FB_DECLARE(task, Scheduler);

FB_PACKAGE2(file, package)

/**
 * This is like the CustomizableStandardFilePackage, but adds some caching for file info and data.
 */
class CachingCustomizableStandardFilePackage : public IFilePackage
{
public:
	CachingCustomizableStandardFilePackage(const DynamicString &basePath, const CustomizableStandardFilePackage::FilePackageFilter &filter, task::Scheduler* scheduler);
	~CachingCustomizableStandardFilePackage();

	File openFile(const DynamicString &fileName, bool archiveOnly) override;
	bool openFiles(const FileNameVector &names, FileVector &outFiles) override;

	bool doesFileExist(const DynamicString &fileName) override;
	bool doesFileExistInArchive(const DynamicString &fileName)  override { return false; }
	file::TimeStamp64 getFileTimeStamp(const DynamicString &fileName) override;

	BigSizeType getFileSize(const DynamicString &fileName) override;
	BigSizeType getFileSizeOnDisk(const DynamicString &fileName) override;
	bool writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath) override;
	bool deleteFile(const DynamicString &fileName, bool suppressErrorMessages) override;
	void listFiles(FileList &results, const StringRef &dir, FileFlagMask flags) override;
	void sortFiles(FileSortInfo *pointer, SizeType files) override;

	bool resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const override;
	bool resolveFile(const DynamicString &fileName, DynamicString &resultfileName, uint32_t &offset, uint32_t *size = 0) override;
	bool resolvePackage(const DynamicString &fileName, ResolveData &resolveData) override;

	bool isMemoryResident() const override { return false; }
	void waitForFileSystem() override;

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
