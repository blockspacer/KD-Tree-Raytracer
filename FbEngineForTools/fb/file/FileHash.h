#pragma once

FB_PACKAGE1(file)

bool getHash64(const StringRef &filepath, uint64_t &hashOut);
bool getHash(const StringRef &filepath, uint32_t &hashOut);

FB_END_PACKAGE1()
