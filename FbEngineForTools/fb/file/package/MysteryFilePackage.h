#pragma once

#include "IFilePackage.h"
#include "fb/lang/Signal.h"
#include "fb/lang/Pimpl.h"

FB_DECLARE(file, package, IFilePackageReadCallback);
FB_DECLARE0(Semaphore);

FB_PACKAGE2(file, package)

class MysteryFilePackage : public IFilePackage
{
public:
	/**
	 * Function called if archive is corrupt
	 */
	typedef void (*ErrorFunc)(const char*, const char*);

	// Helpers for "crypt" stuff
	static const int maxFileLength = 1024;
	static unsigned char *getXorTable(int &length);
	static bool shouldCrypt(const DynamicString &fileName, unsigned char &keyFudge);

	MysteryFilePackage(const StringRef &archive, bool optional, ErrorFunc errorFunc, bool useCryptStuff, bool memoryPackage);
	~MysteryFilePackage();

	bool hasFiles() const;

	File openFile(const DynamicString &fileName, bool archiveOnly = false) override;
	bool openFiles(const FileNameVector &names, FileVector &outFiles) override;

	bool doesFileExist(const DynamicString &fileName) override;
	bool doesFileExistInArchive(const DynamicString &fileName) override;
	file::TimeStamp64 getFileTimeStamp(const DynamicString &fileName)  override { return file::TimeStamp64(-1); }

	BigSizeType getFileSize(const DynamicString &fileName) override;
	BigSizeType getFileSizeOnDisk(const DynamicString &fileName) override;
	bool writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath) override;
	bool deleteFile(const DynamicString &fileName, bool suppressErrorMessages) override;
	void listFiles(FileList &results, const StringRef &dir, FileFlagMask flags) override;
	void sortFiles(FileSortInfo *pointer, SizeType files) override;
	void listFiles(Vector<DynamicString> &files) override;

	bool resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const override;
	bool resolveFile(const DynamicString &fileName, DynamicString &resultfileName, uint32_t &offset, uint32_t *size = 0) override;
	bool resolvePackage(const DynamicString &fileName, ResolveData &resolveData) override;

	bool isMemoryResident() const override;

	/* Read only, not changed on the fly */
	void waitForFileSystem() override { }

	uint32_t getPackageChecksum() const;

#if FB_TRACK_UNUSED_FILES == FB_TRUE
	virtual void dumpUnusedFiles(file::DebugBlockWriter &writer, file::DebugBlockWriter &writer2) override;
#endif

private:
	FB_PIMPL;

	static Semaphore &getFileSemaphore();
};

FB_END_PACKAGE2()
