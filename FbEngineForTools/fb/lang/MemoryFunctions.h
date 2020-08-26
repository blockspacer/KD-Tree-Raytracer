#pragma once

#include "fb/lang/Package.h"
#include "fb/lang/Types.h"

FB_PACKAGE1(lang)

// Guaranteed to map to generic runtime heap. Avoid unless absolutely necessary.
void *osAllocate(size_t sizeInBytes);
void *osReallocate(void *buffer, size_t sizeInBytes);
void osFree(void *buffer);

// Preferred versions. These can use internal pooling without overhead.
void *allocateFixed(size_t sizeInBytes);
void *reallocateFixed(void *buffer, size_t oldSizeInBytes, size_t newSizeInBytes);
void freeFixed(void *buffer, size_t sizeInBytes);

// Preferred versions if having to specify alignment. These use internal pooling, with some overhead
void *allocateAligned(size_t sizeInBytes, uint32_t alignment);
void *reallocateAligned(void *buffer, size_t sizeInBytes, uint32_t alignment);
void freeAligned(void *buffer, uint32_t alignment);

// Preferred versions if buffer size is not available outside allocation. These use internal pooling, with some overhead
void *allocateMemory(size_t sizeInBytes);
void *reallocateMemory(void *buffer, size_t sizeInBytes);
void freeMemory(void *buffer);

FB_END_PACKAGE1()

FB_PACKAGE0()

// Don't use heap directly. Use stuff above instead.
/*
static inline void* malloc(size_t size) { fb_assert(0 && "Don't call me"); return lang::allocateMemory(size); }
static inline void* realloc(void* ptr, size_t size) { fb_assert(0 && "Don't call me"); return lang::reallocateMemory(ptr, size); }
static inline void free(void* ptr) { fb_assert(0 && "Don't call me"); lang::freeMemory(ptr); }
*/
FB_END_PACKAGE0()
