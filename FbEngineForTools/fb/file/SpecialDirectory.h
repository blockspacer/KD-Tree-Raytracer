#pragma once

FB_PACKAGE1(file)

/**
 * A class from which you can query OS specific special folders (user home directory, temporary directories, and that kind of stuff)
 *
 * TODO: Add temporary directories and that kind of stuff
 */
class SpecialDirectory
{
public:

	/* For now, this is only available for PC platforms. Consoles are assumed to have some hard coded paths instead */

	// Returns the user specific application data directory.
	// Probably in some AppData folder in Windows for final release, but might be under binary folder in release/debug.
	// Probably somewhere under user home directory in Mac / Linux.
	// Notice: This is not the game specific directory, you need to append some game specific directory name to this!
	// (Don't write files directly to this directory)
	// Notice: This function only returns absolute paths on windows (empty on fail) as some APIs require them.
	// Notice, this folder name is always fully empty or followed by a slash (for example: "./", etc.)
	static const DynamicString &getApplicationDataDirectory();

	/* In some cases, like when running multiple editors or versions of the game at the same time, using preset 
	 * process numbering (like Builder does), it is a good idea to use local directory instead of shared AppData. 
	 * Note: This must be called early in initialization, as most things using getApplicationDataDirectory() will 
	 * cache the value */
	static bool getUseLocalApplicationDataDirectory();
	static void setUseLocalApplicationDataDirectory(bool value);

private:
	static bool useLocalApplicationDataDirectory;
};

FB_END_PACKAGE1()
