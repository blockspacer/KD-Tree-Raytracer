#include "Precompiled.h"

#include "fb/file/FileList.h"
#include "fb/file/TimeStamp.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/util/UnicodeConverter.h"

FB_PACKAGE1(file)

class FileList::Impl
{
public:
	int flags;
	HANDLE handle;
	int result;
	int fileIndex;
	Entry entry;
	WIN32_FIND_DATAW data;

	Impl(const StringRef &dir, int flags)
		: flags(flags)
		, handle(INVALID_HANDLE_VALUE)
		, result(1)
		, fileIndex(0)
	{
		memset(&entry, 0, sizeof(Entry));
		memset(&data, 0, sizeof(WIN32_FIND_DATAW));
		TempString searchString(dir);
		if (!dir.isEmpty())
			searchString << "/";

		searchString << "*.*";

		string::SimpleUTF16String searchStringWide;
		string::UnicodeConverter::addUTF8StrToUTF16String(searchString, searchStringWide);

		handle = FindFirstFileExW(searchStringWide.getPointer(), FindExInfoBasic, &data, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
	}

	~Impl()
	{
		if (handle != INVALID_HANDLE_VALUE)
			FindClose(handle);
	}

	bool isValid() const
	{
		if (end())
			return false;

		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (flags & FlagExcludeDirs)
				return false;

			const wchar_t *dir = data.cFileName;
			if (wcscmp(dir, L".") == 0)
				return false;
			if (wcscmp(dir, L"..") == 0)
				return false;

			if (!(flags & FlagIncludeSpecialDirs))
			{
				if (wcscmp(dir, L"CVS") == 0)
					return false;
				if (wcscmp(dir, L".svn") == 0)
					return false;
				if (wcscmp(dir, L".git") == 0)
					return false;
				if (wcscmp(dir, L"dev") == 0)
					return false;
			}
		}
		else
		{
			if (flags & FlagExcludeFiles)
				return false;
		}

		return true;
	}

	Entry *next()
	{
		if (fileIndex++ == 0)
		{
			if (isValid())
			{
				entry.name.clear();
				string::UnicodeConverter::addUTF16StrToUTF8String(data.cFileName, entry.name);
				entry.directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
				entry.size = (BigSizeType(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
				entry.lastModified = (TimeStamp64(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime;
				return &entry;
			}
		}

		while (!end())
		{
			result = FindNextFileW(handle, &data);
			if (isValid())
			{
				entry.name.clear();
				string::UnicodeConverter::addUTF16StrToUTF8String(data.cFileName, entry.name);
				entry.directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
				entry.size = (BigSizeType(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
				entry.lastModified = (TimeStamp64(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime;
				return &entry;
			}
		}
		return nullptr;
	}

	bool end() const
	{
		return handle == INVALID_HANDLE_VALUE || result == 0;
	}
};

FileList::FileList(const StringRef &dir, int32_t flags)
	: impl(new Impl(dir, flags))
{
}

FileList::~FileList()
{
}

FileList::Entry *FileList::next()
{
	return impl->next();
}

FB_END_PACKAGE1()
