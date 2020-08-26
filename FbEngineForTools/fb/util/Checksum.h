#pragma once

#include "fb/lang/IntTypes.h"

FB_PACKAGE1(util)

// Endian-independent variant of Fletcher-32
uint32_t calculateChecksumFast(const void *buffer, uint64_t length);

FB_END_PACKAGE1()
