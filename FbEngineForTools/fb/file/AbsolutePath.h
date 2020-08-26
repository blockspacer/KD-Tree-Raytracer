#pragma once

FB_PACKAGE1(file)

static bool isAbsolutePath(const StringRef &fileName)
{
	if (fileName.doesStartWith("/") || fileName.doesStartWith("\\"))
		return true;

	if (fileName.getLength() < 3)
		return false;

	int c = fileName[0];
	if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
		return false;

	if (fileName[1] != ':')
		return false;

	if (fileName[2] != '/' && fileName[2] != '\\')
		return false;

	return true;
}

FB_END_PACKAGE1()
