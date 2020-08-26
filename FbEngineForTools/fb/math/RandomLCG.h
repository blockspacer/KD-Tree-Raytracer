#pragma once

FB_PACKAGE1(math)

// Linear congruential generator
class RandomLCG
{
public:
	RandomLCG()
		: seed(0)
	{
	}

	RandomLCG(uint64_t seed)
		: seed(seed)
	{
	}

	uint32_t get()
	{
		// Java's java.util.Random, POSIX [ln]rand48, glibc [ln]rand48[_r]
		seed = (25214903917ULL * seed + 11ULL) & ((1ULL << 48) - 1);
		return (uint32_t)(seed >> 16);
	}

private:
	uint64_t seed;
};

FB_END_PACKAGE1()
