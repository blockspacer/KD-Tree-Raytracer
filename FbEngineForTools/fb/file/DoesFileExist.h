#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on windows.
// We want to make sure we don't compile code using these
bool doesFileExist(const StringRef &file, bool allowDirs = true, bool caseSensitive = false);
bool isFileFolder(const StringRef &file);

FB_END_PACKAGE1()
