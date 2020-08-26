#include "Precompiled.h"
#include "fb/file/CreateDirectory.h"

FB_PACKAGE1(file)

bool createDirectory(const StringRef &dir)
{
	return createDirectory(dir, true);
}

bool createDirectoryIfMissing(const StringRef &dir)
{
	return createDirectory(dir, false);
}

bool createPath(const StringRef &path, bool errorIfAlreadyExists)
{
	bool noErrors = true;
	TempString subpath;
	// Loop through all intermediate results
	for (SizeType i = 0, len = path.getLength(); i < len; ++i)
	{
		char c = path[i];
		/* Avoid creating dir that ends to : (e.g. C:/ on Windows or [id]:/ on NX */
		if (i > 1 && c == '/' && path[i - 1] != ':')
		{
			/* Only error on last dir, if it exists (or not at all) */
			bool lastDir = (subpath.getLength() - path.getLength()) == 1;
			if (!createDirectory(subpath, errorIfAlreadyExists && lastDir))
				noErrors = false;
		}
		subpath.append(&c, 1);
	}

	return noErrors;
}

bool createPath(const StringRef &path)
{
	return createPath(path, true);
}

bool createPathIfMissing(const StringRef &path)
{
	return createPath(path, false);
}

FB_END_PACKAGE1()
