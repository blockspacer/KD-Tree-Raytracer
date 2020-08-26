#include "Precompiled.h"
#include "fb/util/Compressor.h"
#include "fb/lang/CallStack.h"

// Only editor saves should use compressor, and thus zlib
// So, make sure it's not included

#if FB_UTIL_COMPRESSOR_ENABLED == FB_TRUE

#include <stdio.h>
#define ZLIB_WIN32_NODLL
#include "external/minizip/include/zlib.h"

FB_PACKAGE1(util)

FB_STACK_SET_CLASS(Compressor);

bool Compressor::compressStream(const void *inBuffer, SizeType inBufferSize, CompressionSetting setting)
{
	FB_STACK_METHOD();

	resultBuffer.clear();
	z_stream compressionStream;

	compressionStream.zalloc = Z_NULL;
	compressionStream.zfree  = Z_NULL;
	compressionStream.opaque = Z_NULL;

	int mode = Z_DEFAULT_COMPRESSION;
	if (setting == CompressorFast)
		mode = Z_BEST_SPEED;
	else if (setting == CompressorBest)
		mode = Z_BEST_COMPRESSION;

	int err = deflateInit(&compressionStream, mode);
	if (err != Z_OK)
	{
		return false;
	}

	if (inBufferSize == 0)
	{
		return false;
	}

	resultBuffer.resize(inBufferSize);
	compressionStream.next_in  = (Bytef *)inBuffer;
	compressionStream.avail_in = inBufferSize;
	compressionStream.next_out = (Bytef *)&resultBuffer[0];
	compressionStream.avail_out = inBufferSize;

	err = deflate(&compressionStream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&compressionStream);
		return false;
	}

	SizeType dataSize = inBufferSize - compressionStream.avail_out;
	resultBuffer.resize(dataSize);

	err = deflateEnd(&compressionStream);
	return err == Z_OK;
}

bool Compressor::decompressStream(const void *inBuffer, SizeType inBufferLength)
{
	FB_STACK_METHOD();

	resultBuffer.clear();
	if (inBufferLength == 0)
		return false;

	int err;
	z_stream decompressionStream;

	decompressionStream.zalloc = Z_NULL;
	decompressionStream.zfree  = Z_NULL;
	decompressionStream.opaque = Z_NULL;
	decompressionStream.total_out = 0;
	decompressionStream.next_in  = Z_NULL;
	decompressionStream.avail_in = 0;

	err = inflateInit(&decompressionStream);
	if (err != Z_OK)
		return false;

	const int tempBufferLength = 1000;
	char tempBuffer[1000];

	decompressionStream.next_in  = (Bytef *)inBuffer;
	decompressionStream.avail_in = inBufferLength;
	decompressionStream.next_out = (Bytef *)tempBuffer;

	//int insertCounter = 0;
	while (decompressionStream.total_in < (uLong)inBufferLength)
	{
		decompressionStream.avail_out = tempBufferLength;
		decompressionStream.next_out = (Bytef *)tempBuffer;
		err = inflate(&decompressionStream, Z_NO_FLUSH);

		SizeType max_i = tempBufferLength - decompressionStream.avail_out;
		resultBuffer.insert(resultBuffer.getEnd(), tempBuffer, tempBuffer + max_i);
		//++insertCounter;

		if (err == Z_STREAM_END)
			break;

		if (err != Z_OK)
		{
			inflateEnd(&decompressionStream);
			return false;
		}
	}

	//FB_PRINTF("Done decompressing %u bytes, after %d inserts.\n", resultBuffer.getSize(), insertCounter);

	err = inflateEnd(&decompressionStream);
	return (err == Z_OK);
}

/*
bool Compressor::decompressStream(const void *srcData, int srcDataSize, void *destData, int destDataSize)
{
	if (srcDataSize == 0)
		return false;

	int err;
	z_stream decompressionStream;

	decompressionStream.zalloc = Z_NULL;
	decompressionStream.zfree  = Z_NULL;
	decompressionStream.opaque = Z_NULL;
	decompressionStream.total_out = 0;
	decompressionStream.next_in  = Z_NULL;
	decompressionStream.avail_in = 0;

	err = inflateInit(&decompressionStream);
	if (err != Z_OK)
		return false;

	const int tempBufferLength = 1000;
	char tempBuffer[1000];

	decompressionStream.next_in  = (Bytef *)srcData;
	decompressionStream.avail_in = srcDataSize;
	decompressionStream.next_out = (Bytef *)tempBuffer;
	int offset = 0;

	while (decompressionStream.total_in < (uLong)srcDataSize)
	{
		decompressionStream.avail_out = tempBufferLength;
		decompressionStream.next_out = (Bytef *)tempBuffer;
		err = inflate(&decompressionStream, Z_NO_FLUSH);

		//int max_i = tempBufferLength - decompressionStream.avail_out;
		//resultBuffer.insert(resultBuffer.getEnd(), tempBuffer, tempBuffer + max_i);
		int bytes = tempBufferLength - decompressionStream.avail_out;
		char *destPointer = (char *) destData;
		memcpy(destPointer + offset, tempBuffer, bytes);
		offset += bytes;

		if (err == Z_STREAM_END)
			break;

		if (err != Z_OK)
		{
			inflateEnd(&decompressionStream);
			return false;
		}
	}

	err = inflateEnd(&decompressionStream);
	return (err == Z_OK);

}
*/

bool Compressor::compressBuffer(const void *srcData, SizeType srcDataSize, CompressionSetting setting)
{
	int mode = Z_DEFAULT_COMPRESSION;
	if (setting == CompressorFast)
		mode = Z_BEST_SPEED;
	else if (setting == CompressorBest)
		mode = Z_BEST_COMPRESSION;

	SizeType upperEstimate = compressBound(srcDataSize);
	resultBuffer.resize(upperEstimate);

	uLongf bytesWritten = upperEstimate;
	int ret = compress2((Bytef *) &resultBuffer[0], &bytesWritten, (const Bytef *) srcData, srcDataSize, mode);

	resultBuffer.resize(bytesWritten);
	fb_assert(ret == Z_OK);
	return ret == Z_OK;
}

bool Compressor::decompressBuffer(const void *srcData, SizeType srcDataSize, void *destData, SizeType destDataSize)
{
	uLongf bytesWritten = destDataSize;
	int ret = uncompress((Bytef *)destData, &bytesWritten, (const Bytef *) srcData, srcDataSize);
	fb_assert(ret == Z_OK);

	return bytesWritten == destDataSize;
}

bool Compressor::decompressRawStream(const void *compressedBuffer, SizeType compressedSize, void *uncompressedBuffer, SizeType uncompressedSize)
{
	uLongf bytesWritten = uncompressedSize;
	int ret = uncompress((Bytef *)uncompressedBuffer, &bytesWritten, (const Bytef *) compressedBuffer, compressedSize);
	fb_assert(ret == Z_OK);

	return bytesWritten == uncompressedSize;
}

FB_END_PACKAGE1()

#endif
