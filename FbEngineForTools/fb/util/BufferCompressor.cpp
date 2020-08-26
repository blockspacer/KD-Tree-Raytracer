#include "Precompiled.h"
#include "fb/util/BufferCompressor.h"

#include "fb/lang/ByteOrderSwap.h"
#include "fb/profiling/ZoneProfiler.h"

FB_PACKAGE2(util, lz4)
#include "lz4/lz4.h"
FB_END_PACKAGE2()

FB_PACKAGE2(util, lz4hc)
#include "lz4/lz4hc.h"
FB_END_PACKAGE2()

FB_PACKAGE2(util, zstd)
#include "zstd2/zstd.h"
#include "zstd2/zstd_v04.h"

bool isLegacy(const void *sourceData, uint32_t sizeInBytes)
{
	// Legacy support should be removed once done integrating
	if(sourceData && (sizeInBytes > sizeof(uint32_t)))
	{
		uint32_t magicValue = 0;
		lang::MemCopy::copy(&magicValue, sourceData, sizeof(uint32_t));
		if(magicValue == ZSTDv04_magicNumber)
			return true;
	}

	return false;
}

FB_END_PACKAGE2()


FB_PACKAGE1(util)

uint32_t getCompressionBound(CompressionType type, uint32_t uncompressedSize)
{
	switch(type)
	{
		case CompressionTypeLZ4:
		case CompressionTypeLZ4HC:
		case CompressionTypeLZ4HC2:
			return LZ4_COMPRESSBOUND(uncompressedSize);

		case CompressionTypeZSTD:
		case CompressionTypeZSTD2:
			return (uint32_t) zstd::ZSTD_compressBound(uncompressedSize);

		default:
			break;
	}

	fb_assert(0 && !"Missing compressionType handler.");
	return 0;
}

uint32_t compressBuffer(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize)
{
	if (type == CompressionTypeLZ4)
	{
		FB_ZONE("compressBuffer lz4");
		int destActualSize = lz4::LZ4_compress_limitedOutput((const char*) srcData, (char*) destData, (int) srcDataSize, (int) destDataSize);
		if (destActualSize >= 0)
			return (uint32_t) destActualSize;
	}
	else if (type == CompressionTypeLZ4HC || type == CompressionTypeLZ4HC2)
	{
		FB_ZONE("compressBuffer lz4hc");
		int compressionLevel = 0;
		if (type == CompressionTypeLZ4HC2)
			compressionLevel = 16;

		int destActualSize = lz4hc::LZ4_compressHC2_limitedOutput((const char*) srcData, (char*) destData, (int) srcDataSize, (int) destDataSize, compressionLevel);
		if (destActualSize >= 0)
			return (uint32_t) destActualSize;
	}
	else if (type == CompressionTypeZSTD || type == CompressionTypeZSTD2)
	{
		FB_ZONE("compressBuffer zstd");
		int compressionLevel = 3;
		if (type == CompressionTypeZSTD2)
			compressionLevel = 22;

		size_t compressedSize = zstd::ZSTD_compress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize, compressionLevel);
		return (uint32_t) compressedSize;
	}

	fb_assert(0 && !"Failed to compress, or CompressionType is not handled properly.");
	return 0;
}

uint32_t compressBufferUnsafe(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize)
{
	fb_assert(srcDataSize);

	// Undefined results otherwise
	fb_assert(destDataSize >= getCompressionBound(type, srcDataSize));

	if (type == CompressionTypeLZ4)
	{
		FB_ZONE("compressBufferUnsafe lz4");
		int destActualSize = lz4::LZ4_compress((const char*) srcData, (char*) destData, (int) srcDataSize);
		fb_assert(destActualSize <= (int)destDataSize);
		if (destActualSize >= 0)
			return (uint32_t) destActualSize;
	}
	else if (type == CompressionTypeLZ4HC || type == CompressionTypeLZ4HC2)
	{
		FB_ZONE("compressBufferUnsafe lz4hc");
		int compressionLevel = 0;
		if (type == CompressionTypeLZ4HC2)
			compressionLevel = 16;

		int destActualSize = lz4hc::LZ4_compressHC2((const char*) srcData, (char*) destData, (int) srcDataSize, compressionLevel);
		fb_assert(destActualSize <= (int)destDataSize);
		if (destActualSize >= 0)
			return (uint32_t) destActualSize;
	}
	else if (type == CompressionTypeZSTD || type == CompressionTypeZSTD2)
	{
		FB_ZONE("compressBufferUnsafe zstd");
		int compressionLevel = 3;
		if (type == CompressionTypeZSTD2)
			compressionLevel = 20;

		size_t compressedSize = zstd::ZSTD_compress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize, compressionLevel);
		return (uint32_t) compressedSize;
	}

	fb_assert(0 && !"Failed to compress, or CompressionType is not handled properly.");
	return 0;
}

uint32_t decompressBuffer(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize)
{
	if (type == CompressionTypeLZ4 || type == CompressionTypeLZ4HC || type == CompressionTypeLZ4HC2)
	{
		FB_ZONE("decompressBuffer lz4");
		int destActualSize = lz4::LZ4_decompress_safe((const char*) srcData, (char*) destData, (int) srcDataSize, (int) destDataSize);
		fb_assert(destActualSize <= (int)destDataSize);
		if (destActualSize >= 0)
			return (uint32_t) destActualSize;
	}
	else if (type == CompressionTypeZSTD || type == CompressionTypeZSTD2)
	{
		if(zstd::isLegacy(srcData, srcDataSize))
		{
			FB_ZONE("decompressBuffer legacy zstd");
			size_t uncompressedSize = zstd::ZSTDv04_decompress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize);
			return (uint32_t) uncompressedSize;
		}
		else
		{
			FB_ZONE("decompressBuffer zstd");
			size_t uncompressedSize = zstd::ZSTD_decompress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize);
			return (uint32_t) uncompressedSize;
		}
	}

	fb_assert(0 && !"Failed to uncompress, or CompressionType is not handled properly.");
	return 0;
}

uint32_t decompressBufferUnsafe(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize)
{
	if (type == CompressionTypeLZ4 || type == CompressionTypeLZ4HC || type == CompressionTypeLZ4HC2)
	{
		FB_ZONE("decompressBufferUnsafe lz4");
		int sourceActualSize = lz4::LZ4_decompress_fast((const char*) srcData, (char*) destData, (int) destDataSize);
		fb_assert(uint32_t(sourceActualSize) == srcDataSize);
		if (sourceActualSize >= 0)
			return destDataSize;
	}
	else if (type == CompressionTypeZSTD || type == CompressionTypeZSTD2)
	{
		if(zstd::isLegacy(srcData, srcDataSize))
		{
			FB_ZONE("decompressBufferUnsafe legacy zstd");
			size_t uncompressedSize = zstd::ZSTDv04_decompress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize);
			return (uint32_t) uncompressedSize;
		}
		else
		{
			FB_ZONE("decompressBufferUnsafe zstd");
			size_t uncompressedSize = zstd::ZSTD_decompress(destData, (size_t) destDataSize, srcData, (size_t) srcDataSize);
			return (uint32_t) uncompressedSize;
		}
	}

	fb_assert(0 && !"Failed to uncompress, or CompressionType is not handled properly.");
	return 0;
}

FB_END_PACKAGE1();