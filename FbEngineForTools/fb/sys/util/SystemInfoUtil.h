#pragma once

FB_PACKAGE2(sys, util)

/**
 * Some random system info getter functions.
 */
class SystemInfoUtil
{
public:
	/**
	 * A hint for thread count to use when doing some generic multithreading.
	 * The returned value is always a positive value of at least 1.
	 *
	 * The value defined may be rather arbitary, but it is likely to be something like:
	 * number of processors * number of cores per processor * optimal threads per core 
	 *
	 * You should not assume that this is a exponent of 2! It may be something else.
	 */
	static SizeType getConcurrentThreadCountHint();
	/* Use these only if you know what you are doing. It may be less than clear which is good for you. */
	static SizeType getPhysicalThreadCount();
	static SizeType getLogicalThreadCount();

	/**
	 * This will return a string representing the TYPE of the platform. Expected to be all lower case.
	 * You could expect values such as: "pc" or "console"... or any other arbitary platform specific string ;)
	 * The returned value is a pointer to _const_ string, do NOT delete it.
	 */
	static const char *getPlatformTypeString();
	/**
	 * Same value as getPlatformTypeString, but returned in all upper case letters.
	 */
	static const char *getPlatformTypeStringUpperCase();

	/**
	 * This will return a string representing the platform. Expected to be all lower case.
	 * You could expect values such as: "windows", "linux" or "ps4", etc... or any other arbitary platform 
	 * specific string ;)
	 * The returned value is a pointer to _const_ string, do NOT delete it.
	 */
	static const char *getPlatformString();
	/**
	 * Same value as getPlatformString, but returned in all upper case letters.
	 */
	static const char *getPlatformStringUpperCase();

	/**
	 *	Returns the os version in major, minor format.
	 *	For example windows 8 (v6.2) is returned as 0x0602.
	 *	NOTE: Currently returns 0 for everything but Windows.
	 *	Windows version numbers: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724832(v=vs.85).aspx
	 */
	static uint32_t getOsVersion();

	// this kind of information could be used to map specific software threads to specific hardware cores...
	// but it might be more useful in the thread classes and such - as these require more than getters.
	//static int getNumberOfHardwareCores();
	//static Thread::HardwareCoreId getHardwareCoreId(int hardwareCoreNumber);
};

FB_END_PACKAGE2()
