#include "Precompiled.h"
#include "CompareString.h"

#include "fb/string/detail/CaseConversion.h"

#include <cstring>

FB_PACKAGE2(string, detail)

bool compareStringCaseInsensitive(const StringRef &string1, const StringRef &string2)
{
	SizeType slen1 = string1.getLength();
	SizeType slen2 = string2.getLength();
	if (slen1 != slen2)
		return false;

	const char *ptr1 = string1.getPointer();
	const char *ptr2 = string2.getPointer();
	if (ptr1 == ptr2)
		return true;

	#if FB_COMPILER == FB_MSC
		return _strnicmp(ptr1, ptr2, slen1) == 0;
	#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
		return strncasecmp(ptr1, ptr2, slen1) == 0;
	#else
		/* Manual compare. Not optimal */
		#error "Unknown compiler"
		for (SizeType i = 0; i < slen1; i++)
		{
			if (string::toLower(ptr1[i]) == string::toLower(ptr2[i]))
				continue;
			return false;
		}
		return true;
	#endif
}


CompareResult compareStringWithSorting(const StringRef &string1, const StringRef &string2)
{
	int ret = strcmp(string1.getPointer(), string2.getPointer());
	if (ret < 0)
		return CompareResultFirstLessThanSecond;
	else if (ret > 0)
		return CompareResultFirstGreaterThanSecond;
	else // ret == 0
		return CompareResultEqual;
}


CompareResult compareStringWithSortingCaseInsensitive(const StringRef &string1, const StringRef &string2)
{
	#if FB_COMPILER == FB_MSC
		int ret = _stricmp(string1.getPointer(), string2.getPointer());
	#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
		int ret = strcasecmp(string1.getPointer(), string2.getPointer());
	#else
		#error "Unknown compiler"
	#endif

	if (ret < 0)
		return CompareResultFirstLessThanSecond;
	else if (ret > 0)
		return CompareResultFirstGreaterThanSecond;
	else // ret == 0
		return CompareResultEqual;
}


bool compareSubstring(const StringRef &string, SizeType index, const StringRef &substring)
{
	SizeType slen = string.getLength();
	SizeType sublen = substring.getLength();

	if (index + sublen > slen)
		return false;

	if (strncmp(&(string.getPointer()[index]), substring.getPointer(), substring.getLength()) == 0)
		return true;
	else
		return false;
}


bool compareSubstringCaseInsensitive(const StringRef &string, SizeType index, const StringRef &substring)
{
	SizeType slen = string.getLength();
	SizeType sublen = substring.getLength();

	if (index + sublen > slen)
		return false;

	#if FB_COMPILER == FB_MSC
		return _strnicmp(&(string.getPointer()[index]), substring.getPointer(), substring.getLength()) == 0;
	#elif FB_COMPILER == FB_GNUC || FB_COMPILER == FB_CLANG
		return strncasecmp(&(string.getPointer()[index]), substring.getPointer(), substring.getLength()) == 0;
	#else
		/* Manual compare. Not optimal */
		#error "Unknown compiler"
		for (SizeType i = 0; i < sublen; i++)
		{
			if (string::toLower(string[index + i]) != string::toLower(substring[i]))
				return false;
		}

		return true;
	#endif
}


FB_END_PACKAGE2()
