#include "Precompiled.h"
#include "Filename.h"

#include "fb/string/HeapString.h"

#include <cstring>

FB_PACKAGE2(string, detail)

void appendPath(HeapString& result, const StringRef &str)
{
	const char *lastSlash = strrchr(str.getPointer(), '/');
	if (lastSlash)
	{
		// check if already a path
		if (lastSlash[1] == '\0')
			result += str;
		else
			result.append(str.getPointer(), SizeType(1 + lastSlash - str.getPointer()));
	}
}


StringRef getExtension(const StringRef &str)
{
	const char *dot = strrchr(str.getPointer(), '.');
	if (!dot)
		return StringRef::empty;

	const char *slash = strchr(dot, '/');
	if (slash)
		return StringRef::empty;

	return StringRef(dot + 1);
}


void appendWithoutExtension(HeapString& result, const StringRef &str)
{
	const char *lastDot = strrchr(str.getPointer(), '.');
	if (lastDot)
	{
		if (strchr(lastDot, '/') != NULL)
			result += str;
		else
			result.append(str.getPointer(), SizeType(lastDot - str.getPointer()));
	}
	else
	{
		result += str;
	}
}


StringRef getFile(const StringRef &str)
{
	const char *lastSlash = strrchr(str.getPointer(), '/');
	if (lastSlash)
	{
		// check if already a path
		if (lastSlash[1] == '\0')
			return "";

		return StringRef(lastSlash + 1);
	}
	// file is at root
	return str;
}


void appendFileWithoutExtension(HeapString& result, const StringRef &str)
{
	appendWithoutExtension(result, getFile(str));
}


void solvePath(HeapString &t)
{
	// detect ./
	if (t.getLength() >= 2)
	{
		if (t[0] == '.' && t[1] == '/')
		{
			// remove it
			t.erase(0, 2);
		}
	}

	SizeType i = 0;
	while (i < t.getLength())
	{
		// detect /../
		if (i + 3 < t.getLength())
		{
			if (t[i+0] == '/' && t[i+1] == '.' && t[i+2] == '.' && t[i+3] == '/')
			{
				// remove it
				t.erase(i, 4);
				// remove previous folder too
				if (i > 0)
				{
					SizeType numCharsToErase = 0;
					while (i-1 < t.getLength() && t[i-1] != '/')
					{
						numCharsToErase++;
						i--;
					}
					t.erase(i, numCharsToErase);
				}
				continue;
			}
		}
		i++;
	}
}

FB_END_PACKAGE2()
