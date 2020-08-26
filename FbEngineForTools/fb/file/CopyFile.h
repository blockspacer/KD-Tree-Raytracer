#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on windows.
// We want to make sure we don't compile code using these
// If newFile already exists, it is overwritten
bool copyFile(const StringRef &oldFile, const StringRef &newFile);
bool hardLinkFile(const StringRef &sourceFile, const StringRef &targetFile);

FB_END_PACKAGE1()
