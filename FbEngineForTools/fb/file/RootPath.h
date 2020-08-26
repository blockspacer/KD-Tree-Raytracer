#pragma once

FB_PACKAGE1(file)

/**
 * Path prefix for file accesses (used (at least) on consoles)
 */
class RootPath
{
public:
	static const char *get();
	static void set(const char *str);
};

FB_END_PACKAGE1()
