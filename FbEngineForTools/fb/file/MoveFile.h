#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on windows.
// We want to make sure we don't compile code using these
// If newFile already exists, it is overwritten
bool moveFile(const StringRef &oldFile, const StringRef &newFile, bool logErrors = true);

FB_END_PACKAGE1()
