#include "Precompiled.h"
#include "fb/file/SpecialDirectory.h"

#include "fb/lang/DebugHelp.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"

#include "fb/file/CreateDirectory.h"
#include "fb/file/DoesFileExist.h"
#include "fb/string/util/UnicodeConverter.h"

#include <stdio.h>
#include "fb/lang/IncludeWindows.h"
#pragma warning(push)
#if FB_VS2017_IN_USE == FB_TRUE
	/* __declspec attributes before linkage specification are ignored */
	#pragma warning(disable: 4768)
#endif
#pragma warning(disable: 4917)
#include <shlobj.h>
#pragma warning(pop)

FB_PACKAGE1(file)

bool SpecialDirectory::useLocalApplicationDataDirectory = false;

bool SpecialDirectory::getUseLocalApplicationDataDirectory()
{
	return useLocalApplicationDataDirectory;
}

void SpecialDirectory::setUseLocalApplicationDataDirectory(bool value)
{
	useLocalApplicationDataDirectory = value;
}

static StaticString getApplicationDataDirectoryImpl()
{
	if (SpecialDirectory::getUseLocalApplicationDataDirectory())
	{
		wchar_t *path = new wchar_t[MAX_PATH + 1 + 1];
		DWORD result = GetCurrentDirectoryW(MAX_PATH + 1 + 1, path);
		if (result == 0)
		{
			TempString msg("GetCurrentDirectory failed. ");
			DebugHelp::addGetLastErrorCode(msg);
			FB_FINAL_LOG_ERROR(msg);
			return StaticString::empty;
		}
		TempString pathStr;
		string::UnicodeConverter::addUTF16StrToUTF8String(path, pathStr);
		pathStr.replace("\\", "/");
		if (!pathStr.doesEndWith("/"))
			pathStr << "/";

		pathStr << "localAppData/";
		if (!file::doesFileExist(pathStr))
		{
			file::createPath(pathStr);
		}
		else if (!file::isFileFolder(pathStr))
		{
			FB_FINAL_LOG_ERROR(FB_MSG("Could not create application data directory, as previously created file (",
				pathStr, ") is blocking the creation"));
			return StaticString::empty;
		}
		return StaticString(pathStr);
	}
	else
	{
		PWSTR path = nullptr;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &path);
		if (hr != S_OK)
		{
			FB_FINAL_LOG_ERROR(FB_FMT("Failed to get application data directory. SHGetKnownFolderPath() failed: 0x%x", hr));
			return StaticString::empty;
		}

		TempString pathStr;
		string::UnicodeConverter::addUTF16StrToUTF8String(path, pathStr);
		CoTaskMemFree(path);
		pathStr.replace("\\", "/");
		if (!pathStr.doesEndWith("/"))
			pathStr << "/";

		return StaticString(pathStr);
	}
}

const DynamicString &SpecialDirectory::getApplicationDataDirectory()
{
	FB_UNUSED_NAMED_VAR(static const bool, useLocalApplicationDataDirectoryOnFirstCall) = useLocalApplicationDataDirectory;
	static const StaticString dir(getApplicationDataDirectoryImpl());
	fb_assert(useLocalApplicationDataDirectoryOnFirstCall == useLocalApplicationDataDirectory && "UseLocalApplicationDataDirectory was changed too late");
	return dir;
}


FB_END_PACKAGE1()
