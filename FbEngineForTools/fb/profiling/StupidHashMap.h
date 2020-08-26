#pragma once

#include "fb/lang/MemTools.h"
#include "fb/lang/platform/Likely.h"

FB_PACKAGE1(profiling)

// An allocation free, readily inlining, hashmap-like container
// Hash collisions are ignored

// The size template argument (PowerOfTwoOfSize) is expressed in log2 of the desired size,
// Meaning: if a capacity of 1024 elements is wanted the PowerOfTwoOfSize template-argument
// should be 10 (as 2 ** 10 == 1024). The reason for using a log2 for this is that we want
// to use bit-shift relative to the size of the buffer and this is the easiest way to define
// the shift amount at compile time

template <typename T, SizeType PowerOfTwoOfSize>
struct StupidHashMap
{
	typedef uint16_t MiniHashType;
	typedef uint64_t HashType;
	enum : SizeType
	{
		BufferSizeBits = PowerOfTwoOfSize,
		BufferSize = 1 << BufferSizeBits,
		HashBufferSize = BufferSize,
		MiniHashBits = sizeof(MiniHashType) * 8,
	};

	StupidHashMap(T (&b)[BufferSize], MiniHashType(&o)[HashBufferSize])
	{
		buffer = b;
		miniHashes = o;
		lang::MemSet::set(miniHashes, 0, HashBufferSize * sizeof(miniHashes[0]));
	}

	void clear()
	{
		lang::MemSet::set(miniHashes, 0, HashBufferSize * sizeof(miniHashes[0]));
	}

	FB_FORCEINLINE bool canSet(HashType hash) const
	{
		uint64_t index = getIndex(hash);
		MiniHashType miniHash = getMiniHash(hash);
		if (FB_LIKELY(miniHashes[index] == miniHash))
			return false;

		return canSetSlowPath(index, miniHash, hash);
	}

	FB_NOINLINE T &set(HashType hash)
	{
		uint64_t index = getIndex(hash);
		MiniHashType miniHash = getMiniHash(hash);
		for (uint64_t i = 0; i < 8; ++i)
		{
			uint64_t j = (index + i) % HashBufferSize;
			if (miniHashes[j] == 0)
			{
				miniHashes[j] = miniHash;
				return buffer[j];
			}
		}
		return buffer[index]; // Don't care; this can't happen if canSet was called properly before this
	}

	bool canGet(HashType hash) const
	{
		uint64_t index = getIndex(hash);
		MiniHashType miniHash = getMiniHash(hash);
		for (SizeType i = 0; i < 8; ++i)
		{
			SizeType j = (index + i) % HashBufferSize;
			MiniHashType existingHash = miniHashes[j];
			if (existingHash == miniHash)
				return true;

			if (existingHash == 0)
				return false;
		}
		return false;
	}

	const T &get(HashType hash) const
	{
		uint64_t index = getIndex(hash);
		MiniHashType miniHash = getMiniHash(hash);
		if (miniHashes[index] == miniHash)
			return buffer[index];

		for (uint64_t i = 1; i < 8; ++i)
		{
			uint64_t j = (index + i) % HashBufferSize;
			if (miniHashes[j] == miniHash)
				return buffer[j];
		}
		return buffer[index]; // Don't care; this can't happen if canGet was called properly before this
	}

private:
	FB_NOINLINE bool canSetSlowPath(uint64_t index, MiniHashType miniHash, HashType hash) const
	{
		for (SizeType i = 1; i < 8; ++i)
		{
			SizeType j = (index + i) % HashBufferSize;
			MiniHashType existingHash = miniHashes[j];
			if (existingHash == miniHash)
				return false;

			if (existingHash == 0)
				return true;
		}
		return false;
	}

	FB_FORCEINLINE static uint64_t getIndex(HashType hash)
	{
		// Fibonacci "hashing". Google it.
		const uint64_t goldenRatio = 11400714819323198485LLU;
		return (hash * goldenRatio) >> (64 - BufferSizeBits); // Using bit shift instead of modulo as the upper bits have more entropy
	}

	FB_FORCEINLINE static MiniHashType getMiniHash(HashType hash)
	{
		if (MiniHashBits < 64)
			return MiniHashType((hash >> 6) & ((1LLU << MiniHashBits) - 1)); // Using bits from the middle of the hash to avoid using potentially low-entropy low and high bits in pointers.
		else
			return MiniHashType(hash);
	}

	T *buffer = nullptr;
	MiniHashType *miniHashes = nullptr;
	typedef char PowerOfTwoOfSize_cant_be_large_than_63[BufferSizeBits <= 63 ? 1 : -1];
};

FB_END_PACKAGE1()
