#pragma once

#include "IntTypes.h"

FB_PACKAGE0()

typedef uint32_t SizeType;
typedef int32_t SizeDiffType;
typedef uint64_t BigSizeType;
typedef int64_t BigSizeDiffType;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
typedef decltype(nullptr) nullptr_t;
typedef uint64_t size_t;

static_assert(sizeof(uintptr_t) == sizeof(void*), "32-bit builds should be deprecated right?");
static_assert(sizeof(intptr_t) == sizeof(void*), "32-bit builds should be deprecated right?");

FB_END_PACKAGE0()
