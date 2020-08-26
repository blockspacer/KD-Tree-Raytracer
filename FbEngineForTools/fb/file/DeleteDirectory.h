#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on windows.
// We want to make sure we don't compile code using these
bool deleteEmptyDirectory(const StringRef &dirname);
bool deleteDirectoryAndFiles(const StringRef &dirname, HeapString &errorStr);

FB_END_PACKAGE1()
