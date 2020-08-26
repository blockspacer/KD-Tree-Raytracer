#pragma once

#include "fb/container/Vector.h"

FB_PACKAGE1(util)

// Use explicit type for all permanently stored data.
// Otherwise things can break when adding/removing implementations
enum CompressionType
{
	// Basic LZ4
	CompressionTypeLZ4 = 0,
	// LZ4 HC with default compression level
	CompressionTypeLZ4HC = 1,
	// LZ4 HC with max compression level
	CompressionTypeLZ4HC2 = 2,

	// LZHAM with default compression level
	CompressionTypeLZHAM = 3,
	// LZHAM with max compression level
	CompressionTypeLZHAM2 = 4,

	// ZSTD with default compression level
	CompressionTypeZSTD = 5,
	// ZSTD with max compression level
	CompressionTypeZSTD2 = 6,

	// As light cpu overhead as possible
	CompressionTypeFast = CompressionTypeLZ4,
	// Decent compromise between cpu usage/compression level
	CompressionTypeDefault = CompressionTypeLZ4,
	// Compression time doesn't matter as it's not done runtime
	CompressionTypeOffline = CompressionTypeLZ4HC,
};

// Maximum amount of memory compressing a buffer of given with CompressionType can take
// This can be more than the original size (incompressible data + overhead)
// Returns the maximum size in bytes that the compressed data can be.
uint32_t getCompressionBound(CompressionType type, uint32_t uncompressedSize);

// Compress srcData to destData. destDataSize should to be >= getCompressionBound() for guaranteed success.
// Returns actual size of compressed data written to destData, or 0 if there wasn't enough space
uint32_t compressBuffer(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize);
// Compress srcData to destData. destDataSize _HAS_TO_BE_ >= getCompressionBound() or memory overwrites will occur.
// Returns actual size of compressed data written to destData
uint32_t compressBufferUnsafe(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize);

// Decompress srcData to destData. It's users responsibility to track needed space for successful decompression.
// Returns actual size of uncompressed data written to destData, or 0 if there wasn't enough space.
uint32_t decompressBuffer(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize);
// Decompress srcData to destData. 
// User _HAS_TO_ provide big enough buffer for decompression and otherwise validate (checksum/hash) given data or memory overwrites will occur.
// Returns actual size of uncompressed data written to destData
uint32_t decompressBufferUnsafe(CompressionType type, const void *srcData, uint32_t srcDataSize, void *destData, uint32_t destDataSize);


FB_END_PACKAGE1()
