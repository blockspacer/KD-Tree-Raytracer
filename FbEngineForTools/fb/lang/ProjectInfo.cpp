#include "Precompiled.h"
#include "ProjectInfo.h"

#include "fb/lang/logger/GlobalLogger.h"
#include "fb/string/HeapString.h"

/* Don't assert for uninitialized data in tools project */
#if FB_ENGINE_FOR_TOOLS == FB_TRUE
	#define FB_PI_NOT_INIT_ASSERT(...)
#else
	#define FB_PI_NOT_INIT_ASSERT(...) fb_assert(__VA_ARGS__)
#endif

FB_PACKAGE1(lang)

const StaticString &ProjectInfo::getVersionNumberString()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.versionNumberString;
}


const StaticString &ProjectInfo::getVersionString()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.versionString;
}


uint32_t ProjectInfo::getVersionMajor()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.majorVersion;
}


uint32_t ProjectInfo::getVersionMinor()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.minorVersion;
}


uint32_t ProjectInfo::getVersionHotfix()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.hotfixVersion;
}


uint32_t ProjectInfo::getGameVersion()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.gameVersion;
}


uint32_t ProjectInfo::getBuildNumber()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.buildNumber;
}


uint32_t ProjectInfo::getSourceRevisionNUmber()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.sourceRevisionNumber;
}


bool ProjectInfo::isCompatibleWithVersion(const StringRef &versionString)
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	if (info.versionString == versionString)
	{
		// this is the current version
		return true;
	}
	else
	{
		/* We don't actually care whether compatibilityInfoList is initialized or not. Empty list is fine. We'll still 
		 * mark the info initialized at this point to avoid problems with someone using the info before initialization */
		info.compatibleVersionListInitialized = true;
		TempString versionStringWithLf(versionString);
		versionStringWithLf << "\r\n";
		if (info.compatibleVersionsString.doesContain(versionStringWithLf))
		{
			// found it in the compatible list.
			return true;
		}
		return false;
	}

}


uint32_t ProjectInfo::getBaseNetVersion()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.baseNetVersion;
}


uint32_t ProjectInfo::getNetVersion()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.netVersion;
}


uint32_t ProjectInfo::getNetGameID()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.netGameID;
}


const StaticString &ProjectInfo::getVersionStringForUsersLine1()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.versionStringForUsersLine1;
}


const StaticString &ProjectInfo::getVersionStringForUsersLine2()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info not initialized yet");
	return info.versionStringForUsersLine2;
}


const StaticString &ProjectInfo::getBuildString()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && info.nameInfoInitialized && "Version or name info not initialized yet");
	return info.buildString;
}


const StaticString &ProjectInfo::getFullBuildString()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && info.nameInfoInitialized && "Version or name info not initialized yet");
	return info.fullBuildString;
}


const StaticString &ProjectInfo::getApplicationName()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.applicationName;
}


const StaticString &ProjectInfo::getApplicationNameShort()
{
	/* Note: We don't actually have short application name, though to be more precise, our normal name is already 
	 * short, e.g. "Trine 4" instead of "Trine 4 - Nightmare Prince" */
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.applicationName;
}


const StaticString &ProjectInfo::getApplicationFilename()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.applicationFileName;
}


const StaticString &ProjectInfo::getNetGameName()
{
	/* Same as application file name */
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.applicationFileName;
}


const StaticString &ProjectInfo::getSaveAndConfigDirectoryName()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	if (info.saveAndConfigDirectoryInitialized)
		return info.saveAndConfigDirectoryName;

	info.saveAndConfigDirectoryName = info.defaultSaveAndConfigDirectoryName;
	return info.saveAndConfigDirectoryName;
}


const StaticString &ProjectInfo::getDefaultSaveAndConfigDirectoryName()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.defaultSaveAndConfigDirectoryName;
}


const StaticString &ProjectInfo::getBuildDateString()
{
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.nameInfoInitialized && "Name info not initialized yet");
	return info.gameSourceDateString;
}


void ProjectInfo::initializeVersionInfo(uint32_t majorVersion, uint32_t minorVersion, uint32_t hotfixVersion, uint32_t baseNetVersion, 
	uint32_t netGameID, uint32_t buildNumber, uint32_t sourceRevisionNumber, const StringRef &versionStringPostfix, 
	bool useLegacyNumericalversionGeneration)
{
	/* Can't call fb_main_thread_assert() during static initialization */
	//fb_main_thread_assert();
	ProjectInfo &info = getInstance();

	info.majorVersion = majorVersion;
	info.minorVersion = minorVersion;
	info.hotfixVersion = hotfixVersion;
	info.baseNetVersion = baseNetVersion;
	if (useLegacyNumericalversionGeneration)
	{
		/* Works for major, minor and hotfix version of 0-9 */
		info.gameVersion = majorVersion * 10 * 10 + minorVersion * 10 + hotfixVersion;
		info.netVersion = baseNetVersion * 10 * 10 * 10 + info.gameVersion;
	}
	else
	{
		/* Works for minor and hotfix version of 0-99 */
		info.gameVersion = majorVersion * 100 * 100 + minorVersion * 100 + hotfixVersion;
		info.netVersion = baseNetVersion * 100 * 100 * 100 + info.gameVersion;
	}
	info.netGameID = netGameID;
	info.buildNumber = buildNumber;
	info.sourceRevisionNumber = sourceRevisionNumber;
	TempString tmp;
	tmp << majorVersion << "." << minorVersion << "." << hotfixVersion;
	info.versionNumberString = StaticString(tmp);
	tmp << versionStringPostfix;
	info.versionString = StaticString(tmp);
	info.versionStringForUsersLine1 = info.versionString;

	tmp.clear();
	tmp << "(build " << buildNumber << ")";
	info.versionStringForUsersLine2 = StaticString(tmp);

	info.versionInfoInitialized = true;
}


void ProjectInfo::initializeNameInfo(const StringRef &applicationName, const StringRef &applicationFileName, const StringRef &applicationFileNameUppercase, const StringRef &gameSourceDateString)
{
	/* Can't call fb_main_thread_assert() during static initialization */
	//fb_main_thread_assert();
	ProjectInfo &info = getInstance();
	FB_PI_NOT_INIT_ASSERT(info.versionInfoInitialized && "Version info must be initialized first");

	#if (FB_BUILD == FB_DEBUG)
		const StringRef buildType("Debug");
	#elif (FB_BUILD == FB_RELEASE)
		const StringRef buildType("Release");
	#elif (FB_BUILD == FB_FINAL_RELEASE)
		const StringRef buildType("Final Release");
	#else
		#error "Unknown build."
	#endif

	info.applicationName = StaticString(applicationName);
	info.applicationFileName = StaticString(applicationFileName);
	info.gameSourceDateString = StaticString(gameSourceDateString);

	info.defaultSaveAndConfigDirectoryName = StaticString(applicationFileNameUppercase);

	TempString tmp;
	tmp << applicationName << " " << buildType << " " << info.versionString << " " << info.buildNumber;
	info.buildString = StaticString(tmp);

	tmp.clear();
	tmp << applicationName << " " << buildType << ", v" << info.versionString << ", Build date: " << gameSourceDateString <<
		", revision: " << info.sourceRevisionNumber << " (build " << info.buildNumber << ")";
	info.fullBuildString = StaticString(tmp);

	info.nameInfoInitialized = true;
}


void ProjectInfo::initializeCompatibleVersionsList(const StringRef &compatibleVersionsString)
{
	/* Can't call fb_main_thread_assert() during static initialization */
	//fb_main_thread_assert();
	ProjectInfo &info = getInstance();
	fb_assert(!info.compatibleVersionListInitialized && "Too late to call this, already initialized");
	info.compatibleVersionsString = StaticString(compatibleVersionsString);
	info.compatibleVersionListInitialized = true;
}


void ProjectInfo::initializeSaveAndConfigDirectory(const StringRef &directoryName)
{
	/* Can't call fb_main_thread_assert() during static initialization */
	//fb_main_thread_assert();
	ProjectInfo &info = getInstance();
	fb_assert(!info.saveAndConfigDirectoryInitialized && "Too late to call this, already initialized");
	info.saveAndConfigDirectoryName = StaticString(directoryName);
	info.saveAndConfigDirectoryInitialized = true;
}


void ProjectInfo::debugPrintAll()
{
	/* Please add any new stuff here */
	ProjectInfo &info = getInstance();
	fb_assert(info.nameInfoInitialized && "Name info not initialized yet");
	fb_assert(info.versionInfoInitialized && "Version info must be initialized first");
	TempString msg("ProjectInfo debug:\n");
	msg << "\tVersion number string: " << getVersionNumberString() << "\n";
	msg << "\tVersion string: " << getVersionString() << "\n";
	msg << "\tMajor version: " << getVersionMajor() << "\n";
	msg << "\tMinor version: " << getVersionMinor() << "\n";
	msg << "\tVersion hotfix: " << getVersionHotfix() << "\n";
	msg << "\tGame version: " << getGameVersion() << "\n";
	msg << "\tBuild number: " << getBuildNumber() << "\n";
	msg << "\tNet version: " << getNetVersion() << "\n";
	msg << "\tVersion string for users line 1: " << getVersionStringForUsersLine1() << "\n";
	msg << "\tVersion string for users line 2: " << getVersionStringForUsersLine2() << "\n";
	msg << "\tBuild string: " << getBuildString() << "\n";
	msg << "\tFull build string: " << getFullBuildString() << "\n";
	msg << "\tApplication name: " << getApplicationName() << "\n";
	msg << "\tApplication name short: " << getApplicationNameShort() << "\n";
	msg << "\tApplication file name: " << getApplicationFilename() << "\n";
	msg << "\tDefault save and config directory name: " << getDefaultSaveAndConfigDirectoryName() << "\n";
	msg << "\tSave and config directory name: ";
	if (info.saveAndConfigDirectoryInitialized)
		msg << getSaveAndConfigDirectoryName() << "\n";
	else
		msg << "[not initialized, using default]" << "\n";

	msg << "\tBuild date string: " << getBuildDateString() << "\n";
	msg << "\tVersion compatibility list: " << info.compatibleVersionsString;
	FB_LOG_INFO(msg);
}


ProjectInfo &ProjectInfo::getInstance()
{
	static ProjectInfo instance;
	return instance;
}


FB_END_PACKAGE1()