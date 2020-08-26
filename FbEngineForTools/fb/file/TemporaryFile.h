#pragma once

FB_PACKAGE1(file)

// Low-level stuff, only defined on windows.
// We want to make sure we don't compile code using these
// If generation is succesful, the returned file MUST be deleted manually
bool getTemporaryFilename(HeapString &outFilename);

FB_END_PACKAGE1()
