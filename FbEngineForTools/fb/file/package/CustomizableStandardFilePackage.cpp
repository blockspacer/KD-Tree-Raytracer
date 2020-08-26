#include "Precompiled.h"
#include "CustomizableStandardFilePackage.h"

#include "fb/file/FileAllocator.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/FileList.h"
#include "fb/file/InputFile.h"
#include "fb/file/OutputFile.h"
#include "fb/file/RootPath.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/IncludeWindows.h"

FB_PACKAGE2(file, package)

FB_STACK_SET_CLASS(CustomizableStandardFilePackage);

CustomizableStandardFilePackage::FilePackageFilter::FilePackageFilter()
{
}


CustomizableStandardFilePackage::FilePackageFilter::~FilePackageFilter()
{
}


void CustomizableStandardFilePackage::FilePackageFilter::addBasePathToExcludeList(const DynamicString &basePath)
{
	excludeBasePathList.pushBack(basePath);
}


void CustomizableStandardFilePackage::FilePackageFilter::addSubPathToExcludeList(const DynamicString &path)
{
	excludeSubPathList.pushBack(path);
}


bool CustomizableStandardFilePackage::FilePackageFilter::isPathExcluded(const StringRef &path)
{
	for (const DynamicString &str : excludeBasePathList)
	{
		if (path.doesStartWith(str))
		{
			return true;
		}
	}
	for (const DynamicString &str : excludeSubPathList)
	{
		if (path.doesContain(str))
		{
			return true;
		}
	}
	return false;
}


class CustomizableStandardFilePackage::Impl
{
public:
	DynamicString basePath;
	CustomizableStandardFilePackage::FilePackageFilter filter;

	Impl(const DynamicString &basePath_, const FilePackageFilter &filter)
	{
		if (basePath_ != "./")
		{
			basePath = basePath_;
		}

		this->filter = filter;
		fb_assert(!isPathExcluded(basePath));
	}

	/**
	 * This will return true if the filepath should be excluded. The given path may specify a folder or a file name.
	 */
	bool isPathExcluded(const DynamicString &path)
	{
		return filter.isPathExcluded(path);
	}

	bool isPathExcluded(const StringRef &path)
	{
		return filter.isPathExcluded(path);
	}
};

CustomizableStandardFilePackage::CustomizableStandardFilePackage(const DynamicString &basePath, const FilePackageFilter &filter)
{
	FB_ZONE("CustomizableStandardFilePackage()");
	impl.reset(new Impl(basePath, filter));
}

CustomizableStandardFilePackage::~CustomizableStandardFilePackage()
{
	// nop
}

File CustomizableStandardFilePackage::openFile(const DynamicString &fileName, bool archiveOnly)
{
	FB_ZONE("CustomizableStandardFilePackage::openFile");
	FB_STACK_METHOD();

	if (archiveOnly)
		return File(getEmptyBuffer(), 0);

	//FB_PRINTF("Opening standard file package file: %s\n", fileName.getPointer());

	file::InputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	if (file.open(str))
	{
		BigSizeType bufferSize = file.getSize();
		if (bufferSize > 0)
		{
			file::FileBufferPointer buffer = getFileBuffer(bufferSize, fileName.getPointer());
			file.readData(buffer.getMutable(), bufferSize);

			return File(buffer, bufferSize);
		}
	}

	return File(getEmptyBuffer(), 0);
}

bool CustomizableStandardFilePackage::openFiles(const FileNameVector &names, FileVector &outFiles)
{
	bool success = true;
	for (const DynamicString &fileName : names)
	{
		outFiles.pushBack(openFile(fileName, false));
		success = success && outFiles.getBack();
	}

	return success;
}

bool CustomizableStandardFilePackage::doesFileExist(const DynamicString &fileName)
{
	FB_ZONE("CustomizableStandardFilePackage::doesFileExist");
	if (impl->isPathExcluded(fileName))
		return false;

	file::InputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	if (file.open(str))
		return true;

	return false;
}


file::TimeStamp64 CustomizableStandardFilePackage::getFileTimeStamp(const DynamicString &fileName)
{
	if (impl->isPathExcluded(fileName))
		return file::TimeStamp64(-1);

	FB_ZONE("CustomizableStandardFilePackage::getFileTimeStamp");
	return file::getFileTimestamp64(fileName);
}


BigSizeType CustomizableStandardFilePackage::getFileSize(const DynamicString &fileName)
{
	if (impl->isPathExcluded(fileName))
		return 0;

	FB_ZONE("CustomizableStandardFilePackage::getFileSize");
	file::InputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	if (file.open(str))
		return file.getSize();

	return 0;
}


BigSizeType CustomizableStandardFilePackage::getFileSizeOnDisk(const DynamicString &fileName)
{
	return getFileSize(fileName);
}


bool CustomizableStandardFilePackage::writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath)
{
	FB_ZONE("CustomizableStandardFilePackage::writeFile");
	FB_STACK_METHOD();

	if (impl->isPathExcluded(fileName))
	{
		fb_assert(0 && "Notice, writing a file that will be excluded by read operations (due to path exclusion filter).");
	}

	file::OutputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;
	if (createPath)
		file::createPathIfMissing(str);

	if (!file.open(str))
		return false;

	file.writeData(data, dataSize);
	return true;
}

bool CustomizableStandardFilePackage::deleteFile(const DynamicString &fileName, bool suppressErrorMessages)
{
	FB_ZONE("CustomizableStandardFilePackage::deleteFile");
	FB_STACK_METHOD();

	if (impl->isPathExcluded(fileName))
	{
		fb_assert(0 && "Notice, deleting a file that would have been excluded by read operations (due to path exclusion filter).");
	}

	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;
	return file::deleteFile(str, suppressErrorMessages);
}

void CustomizableStandardFilePackage::listFiles(FileList &results, const StringRef &dir, FileFlagMask flags)
{
	FB_ZONE("CustomizableStandardFilePackage::listFiles");
	FB_STACK_METHOD();

	bool includeFiles = !(flags & FileFlagExcludeFiles);
	bool includeDirs = !(flags & FileFlagExcludeDirs);
	bool recurse = !(flags & FileFlagNoRecurse);

	TempString modifiedDir = impl->basePath;
	if (modifiedDir.getLength() > 0 && (modifiedDir[modifiedDir.getLength() - 1] != '/' && modifiedDir[modifiedDir.getLength() - 1] != '\\') && !dir.isEmpty())
		modifiedDir += "/";

	modifiedDir += dir;
	
	if (impl->isPathExcluded(modifiedDir))
	{
		// this seems like an unintended scenario - someone filtering out an entire dir that someone is specifically
		// trying to list files in?
		fb_expensive_assert_once("The entire listFiles call will be ignored due to path exclusion filter.");
		return;
	}

	file::FileList list(modifiedDir);
	file::FileList::Entry *entry = nullptr;
	while ((entry = list.next()) != nullptr)
	{
		TempString fullName(dir);
		if (!fullName.isEmpty())
			fullName += "/";

		fullName += entry->name;

		if (impl->isPathExcluded(fullName))
		{
			continue;
		}

		if (entry->directory)
		{
			if (includeDirs)
				results.insertOrAssign(DynamicString(fullName), this);

			if (recurse)
				listFiles(results, fullName, flags);
		}
		else
		{
			if (includeFiles)
			{
				results.insertOrAssign(DynamicString(fullName), this);
			}
		}
	}
}

void CustomizableStandardFilePackage::sortFiles(FileSortInfo *pointer, SizeType files)
{
}

bool CustomizableStandardFilePackage::resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const
{
	file::InputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	if (file.open(str))
	{
		resolveDataOut.offset = 0;
		resolveDataOut.rawSize = file.getSize();
		resolveDataOut.size = resolveDataOut.rawSize;

		file.close();
		return true;
	}

	file.close();
	return false;
}

bool CustomizableStandardFilePackage::resolveFile(const DynamicString &fileName, DynamicString &resultFilename, uint32_t &offset, uint32_t *size)
{
	FB_ZONE("CustomizableStandardFilePackage::resolveFile");
	file::InputFile file;
	TempString str;
	str += file::RootPath::get();
	str += impl->basePath;
	str += fileName;

	if (file.open(str))
	{
		resultFilename = DynamicString(str);
		offset = 0;
		if (size)
			*size = (uint32_t) file.getSize();

		file.close();
		return true;
	}

	file.close();
	return false;
}

bool CustomizableStandardFilePackage::resolvePackage(const DynamicString &fileName, ResolveData &resolveData)
{
	// We don't care
	return false;
}

FB_END_PACKAGE2()
