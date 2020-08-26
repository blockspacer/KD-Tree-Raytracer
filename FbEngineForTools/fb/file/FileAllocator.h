#ifndef FB_FILE_FILEALLOCATOR_H
#define FB_FILE_FILEALLOCATOR_H

#include "File.h"

FB_PACKAGE1(file)

FileBufferPointer getFileBuffer(BigSizeType size, const char *filename = "");
FileBufferPointer getEmptyBuffer();

FB_END_PACKAGE1()

#endif
