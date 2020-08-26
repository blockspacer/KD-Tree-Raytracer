#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on PC.
// We want to make sure we don't compile code using these
bool deleteFile(const StringRef &filename, bool suppressErrorMessages = false);
bool deleteFileIfExists(const StringRef &filename, bool suppressErrorMessages = false);

FB_END_PACKAGE1()
