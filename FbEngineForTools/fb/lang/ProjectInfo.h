#pragma once

#include "fb/string/StaticString.h"

FB_DECLARE0(StringRef)

FB_PACKAGE1(lang)

class ProjectInfo
{
public:
	/* Returns version number in format "x.y.z", where x is major version and y is minor version and z hotfix version
	 * (the exact formatting is likely to be like: "x.y.z" - in example, "1.0.1", but you should not rely on that) */
	static const StaticString &getVersionNumberString();

	/* Returns a string like getVersionNumberString, but may have a postfix added to it (such as "beta", "dev" or something similar) */
	static const StaticString &getVersionString();

	/* Getters for major, minor and hotfix version */
	static uint32_t getVersionMajor();
	static uint32_t getVersionMinor();
	static uint32_t getVersionHotfix();
	/* Single value combining major, minor and hotfix version numbers */
	static uint32_t getGameVersion();

	/* Returns the number of the build, which is automatically kept track by Builder */
	static uint32_t getBuildNumber();

	static uint32_t getSourceRevisionNUmber();
	/**
	 * This call should return true if the _application_ recognized the given version string and is compatible
	 * with it (any data for that version).
	 *
	 * You should also notice that any data may define it's own compatibilities with specific application
	 * versions, in which case, the application (this call) does not have to recognize the data.
	 *
	 * In other words, the compatibility may go both ways:
	 * new data -> (defines compatibility with) -> old application
	 * new application -> (defines compatibility with) -> old data
	 */
	static bool isCompatibleWithVersion(const StringRef &versionString);

	/* Separate version for netgame compatibility. We may, for example, have compatibility with save files while net
	 * game is incompatible */
	static uint32_t getBaseNetVersion();

	/* Similar to base net version, but also depends on major, minor and hotfix version of the game */
	static uint32_t getNetVersion();

	/* Project specific FOURCC ID for net game (e.g. TRN4 or something like that) */
	static uint32_t getNetGameID();

	/* Returns the user version info string (combine this with getVersionStringForUsersLine2()). This is usually shown 
	 * in game's mainmenu or credits. Users and our internal QA team will use this info for reporting bugs */
	static const StaticString &getVersionStringForUsersLine1();

	/* Returns the user version info string (combine this with getVersionStringForUsersLine1()). This is usually shown 
	 * in game's mainmenu or credits. Users and our internal QA team will use this info for reporting bugs */
	static const StaticString &getVersionStringForUsersLine2();

	/* Returns the build string. This string is likely to contain the version string as well as additional
	 * information about the build */
	static const StaticString &getBuildString();

	/* Like getBuildString, but returns even more stuff */
	static const StaticString &getFullBuildString();

	/* Will return the application name in poorly defined (human readable) syntax. :) */
	static const StaticString &getApplicationName();

	/* Will return the application short name in poorly defined (human readable) syntax. :) */
	static const StaticString &getApplicationNameShort();

	/* Will return a valid alphanumberic identifier, suitable for filenames, for the application */
	static const StaticString &getApplicationFilename();

	/* Will return a valid alphanumberic identifier */
	static const StaticString &getNetGameName();

	/* Will return a valid alphanumberic identifier, suitable for filenames, for save and config directory name */
	static const StaticString &getSaveAndConfigDirectoryName();
	/* Same as above, but always returns the default name (not affected by set) */
	static const StaticString &getDefaultSaveAndConfigDirectoryName();

	/* Human readable (but not localized) string of build date and (approximate) time */
	static const StaticString &getBuildDateString();

	/* First two initialization functions must be called in order. All initialization functions should be called 
	 * before multi-threaded execution starts, as they are not thread-safe */

	/* VersionStringPostfix is something like "alpha" or "beta" */
	static void initializeVersionInfo(uint32_t majorVersion, uint32_t minorVersion, uint32_t hotfixVersion, uint32_t baseNetVersion, 
		uint32_t netGameID, uint32_t buildNumber, uint32_t sourceRevisionNumber, const StringRef &versionStringPostfix, 
		bool useLegacyNumericalversionGeneration);
	/* ApplicationFileNames must be something that can be appended to or be part of file path. Should be shortish, must not contain spaces */
	static void initializeNameInfo(const StringRef &applicationName, const StringRef &applicationFileName, const StringRef &applicationFileNameUppercase, const StringRef &gameSourceDateString);
	/* It is not necessary to call this. In that case only current version is considered compatible */
	static void initializeCompatibleVersionsList(const StringRef &compatibleVersionsString);
	/* It is not necessary to call this, as default directory will be based on applicationFileName. This is for 
	 * overwriting the default, and must be called before first call to getApplicationSaveAndConfigDirectoryName() is 
	 * made */
	static void initializeSaveAndConfigDirectory(const StringRef &directoryName);
	/* Use this to check all project info if in doubt */
	static void debugPrintAll();

private:
	static ProjectInfo &getInstance();

	static StaticString getUninitializedString();
	/* Version info */
	uint32_t majorVersion = 0;
	uint32_t minorVersion = 0;
	uint32_t hotfixVersion = 0;
	uint32_t baseNetVersion = 0;
	uint32_t netVersion = 0;
	uint32_t netGameID = 0;
	uint32_t gameVersion = 0;
	uint32_t buildNumber = 0;
	uint32_t sourceRevisionNumber = 0;
	StaticString versionNumberString;
	StaticString versionString;
	StaticString versionStringForUsersLine1;
	StaticString versionStringForUsersLine2;

	/* Name info */
	StaticString applicationName;
	StaticString applicationFileName;
	StaticString gameSourceDateString;
	StaticString buildString;
	StaticString fullBuildString;

	/* Compatibility info */
	StaticString compatibleVersionsString;

	/* Save and config dir info */
	StaticString saveAndConfigDirectoryName;
	StaticString defaultSaveAndConfigDirectoryName;

	bool nameInfoInitialized = false;
	bool versionInfoInitialized = false;
	bool compatibleVersionListInitialized = false;
	bool saveAndConfigDirectoryInitialized = false;
};

FB_END_PACKAGE1()