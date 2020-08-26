#pragma once

#include "fb/lang/platform/FBMath.h"
#include "fb/math/Quaternion.h"
#include "fb/math/Bounds.h"

FB_PACKAGE1(math)
	
// Rand2RersRs by Mark A. Overton
class Random64
{
public:
	Random64()
	{
		seed(0);
	}

	Random64(uint32_t seedValue)
	{
		seed(seedValue);
	}

	#define rotl(r,n) (((r)<<(n)) | ((r)>>((8*sizeof(r))-(n))))

	void seed(uint32_t seed)
	{
		unsigned n;
		xx = 2257535ULL;
		yy = 821507ULL;
		zz = 819103680ULL;
		for (n=((seed>>22)&0x3ff)+20; n>0; n--) xx = rotl(xx,52) - rotl(xx, 9);
		for (n=((seed>>11)&0x7ff)+20; n>0; n--) yy = rotl(yy,24) - rotl(yy,45);
		for (n=((seed    )&0x7ff)+20; n>0; n--) zz -= rotl(zz,38);
	}

	uint64_t get()
	{
		xx = rotl(xx,52) - rotl(xx, 9); // RERS, period = 1157113674487 = 71*10067*1618891
		yy = rotl(yy,24) - rotl(yy,45); // RERS, period = 1405504503483 = 3*17*27558911833
		zz -= rotl(zz,38);              // RS,   period = 10483687178 = 2*23*47*251*19319
		return xx ^ yy ^ zz;
	}

	#undef rotl

private:
	uint64_t xx, yy, zz;
};

// Returns float in range [0..1]
static float getRandomFloat(math::Random64 &rng)
{
	const uint32_t maxRandValue = 0xFFFFF;
	const uint32_t randValue = ((uint32_t)rng.get()) & maxRandValue;
	return randValue / (float)maxRandValue;
}

static SizeType getRandomSizeType(math::Random64 &rng, SizeType min, SizeType max)
{
	return SizeType((uint64_t)min + rng.get() % ((uint64_t)max - (uint64_t)min));
}

// Returns float in range [min..max]
static float getRandomFloat(math::Random64 &rng, float min, float max)
{
	return min + getRandomFloat(rng) * (max - min);
}

// Returns double in range [0..1]
static double getRandomDouble(math::Random64 &rng)
{
	const uint64_t maxRandValue = 0xFFFFFFFF;
	const uint64_t randValue = rng.get() & maxRandValue;
	return randValue / (double)maxRandValue;
}

// Returns random vector in range [0..1]
static math::VC3 getRandomVC3(math::Random64 &rng)
{
	const uint64_t randValue = rng.get();
	const uint32_t maxRandValue = 0xFFFFF;
	const uint32_t rx = (uint32_t)((randValue>>0) & maxRandValue);
	const uint32_t ry = (uint32_t)((randValue>>20) & maxRandValue);
	const uint32_t rz = (uint32_t)((randValue>>40) & maxRandValue);
	return math::VC3(rx / (float)maxRandValue, ry / (float)maxRandValue, rz / (float)maxRandValue);
}

static math::VC3 getRandomCube(math::Random64 &rng)
{
	return math::VC3(1.f, 1.f, 1.f) - getRandomVC3(rng) * 2;
}

static math::VC3 getRandomInBounds(math::Random64 &rng, const math::Bounds &bounds)
{
	return math::VC3
	(
		getRandomFloat(rng, bounds.boundsMin.x, bounds.boundsMax.x),
		getRandomFloat(rng, bounds.boundsMin.y, bounds.boundsMax.y),
		getRandomFloat(rng, bounds.boundsMin.z, bounds.boundsMax.z)
	);
}

// Returns random quatenion
static math::QUAT getRandomQUAT(math::Random64 &rng)
{
	math::VC3 u = getRandomVC3(rng);
	float scale1 = FB_FSQRT(1.0f - u.x);
	float scale2 = FB_FSQRT(u.x);
	float x = scale1 * sinf(2.0f * math::Pi * u.y);
	float y = scale1 * cosf(2.0f * math::Pi * u.y);
	float z = scale2 * sinf(2.0f * math::Pi * u.z);
	float w = scale2 * cosf(2.0f * math::Pi * u.z);
	return math::QUAT(x, y, z, w);
}

FB_END_PACKAGE1()
