#include "Precompiled.h"
#include "TextOutputStream.h"

#include "fb/lang/platform/LineFeed.h"
#include <cstring>

FB_PACKAGE1(stream)

void TextOutputStream::write(const char *str)
{
	OutputStream<lang::NativeByteOrder>::write(str, SizeType(strlen(str)));
}

void TextOutputStream::writeLine(const char *str)
{
	OutputStream<lang::NativeByteOrder>::write(str, SizeType(strlen(str)));
	writeLineEnding();
}

void TextOutputStream::writeLineEnding()
{
	OutputStream<lang::NativeByteOrder>::write(FB_PLATFORM_LF, FB_PLATFORM_LF_LEN);
}

FB_END_PACKAGE1()