#include "Precompiled.h"
#include "Base64.h"

#include "fb/string/HeapString.h"

// Base64 implementation from https://github.com/B-Con/crypto-algorithms

#include "base64/base64.h"

FB_PACKAGE1(util)

uint32_t getBase64EncodedSizeInBytes(uint32_t sourceSizeInBytes)
{
	if (sourceSizeInBytes == 0)
		return 0;
	return (uint32_t) base64_encode(NULL, NULL, sourceSizeInBytes, 0);
}

uint32_t getBase64DecodedSizeInBytes(const void *sourceBuffer, uint32_t sourceSizeInBytes)
{
	if (sourceSizeInBytes == 0)
		return 0;
	return (uint32_t) base64_decode((const BYTE*) sourceBuffer, NULL, sourceSizeInBytes);
}

uint32_t base64Encode(const void *sourceBuffer, uint32_t sourceSizeInBytes, void *destinationBuffer, uint32_t destinationSizeInBytes)
{
	if (sourceSizeInBytes == 0)
		return 0;
	uint32_t resultInBytes = (uint32_t) base64_encode((const BYTE*) sourceBuffer, (BYTE*) destinationBuffer, sourceSizeInBytes, 0);
	fb_assert(resultInBytes <= destinationSizeInBytes);
	return resultInBytes;
}

uint32_t base64Decode(const void *sourceBuffer, uint32_t sourceSizeInBytes, void *destinationBuffer, uint32_t destinationSizeInBytes)
{
	if (sourceSizeInBytes == 0)
		return 0;
	uint32_t resultInBytes = (uint32_t) base64_decode((const BYTE*) sourceBuffer, (BYTE*) destinationBuffer, sourceSizeInBytes);
	fb_assert(resultInBytes <= destinationSizeInBytes);
	return resultInBytes;
}

void base64Encode(const void *sourceBuffer, uint32_t sourceSizeInBytes, HeapString &dstStr)
{
	dstStr.resizeAndFill(util::getBase64EncodedSizeInBytes(sourceSizeInBytes), ' ');
	FB_UNUSED_NAMED_VAR(uint32_t, resultSizeInBytes) = util::base64Encode(sourceBuffer, sourceSizeInBytes, &dstStr[0], dstStr.getLength());
	fb_assert(resultSizeInBytes <= dstStr.getLength());
}

FB_END_PACKAGE1()

#pragma warning(disable: 4365)
#include "base64/base64.c"
