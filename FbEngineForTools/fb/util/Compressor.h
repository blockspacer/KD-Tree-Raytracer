#ifndef FB_UTIL_COMPRESSOR_H
#define FB_UTIL_COMPRESSOR_H

#include "fb/container/PodVector.h"

#define FB_UTIL_COMPRESSOR_ENABLED FB_TRUE

FB_PACKAGE1(util)

class Compressor
{
public:
	enum CompressionSetting
	{
		CompressorFast,
		CompressorBest,
		CompressorDefault,
	};

	bool compressStream(const void *srcData, SizeType srcDataSize, CompressionSetting setting = CompressorFast);
	bool decompressStream(const void *srcData, SizeType srcDataSize);

	bool compressBuffer(const void *srcData, SizeType srcDataSize, CompressionSetting setting = CompressorFast);
	bool decompressBuffer(const void *srcData, SizeType srcDataSize, void *destData, SizeType destDataSize);

	static bool decompressRawStream(const void *compressedBuffer, SizeType compressedSize, void *uncompressedBuffer, SizeType uncompressedSize);

	PodVector<char> &getResultBuffer() { return resultBuffer; }

private:
	PodVector<char> resultBuffer;
};

FB_END_PACKAGE1()

#endif
