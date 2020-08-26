#include "Precompiled.h"
#include "LinearHashBase.h"

#include "fb/lang/FBPrintf.h"
#include "fb/lang/FBStaticAssert.h"
#include "fb/lang/RoundToPow.h"

#include <cstring>

FB_PACKAGE1(container)

uint32_t LinearHashBase::getCapacityFromSize(uint32_t wantedSize) const
{
	uint32_t requiredCapacity = 2 * wantedSize;
	requiredCapacity =  roundToPow2(requiredCapacity);
	return requiredCapacity;
}

float LinearHashBase::getAverageProbeCount() const
{
	if (getSize() == 0)
		return 0.f;

	uint64_t counter = 0;
	uint32_t maxDistance = 0;

	const uint32_t *FB_RESTRICT hashArray = getHashArray();
	uint32_t locaCapacity = getCapacity();
	for (uint32_t i = 0; i < locaCapacity; ++i)
	{
		if (isValidHash(hashArray[i]))
		{
			uint32_t currentDistance = hashimp::getProbeDistance(hashArray[i], i, locaCapacity);
			maxDistance = currentDistance > maxDistance ? currentDistance : maxDistance;
			counter += (uint64_t) currentDistance;
		}
	}

	counter *= (uint64_t) 1000;
	counter /= (uint64_t) getSize();
	float result = float(counter) / 1000.f;

	FB_PRINTF("Average probe count %f, max distance %u\n", result, maxDistance);
	return result;
}

uint32_t LinearHashBase::getFirstIndex() const
{
	if(size)
	{
		const uint32_t *FB_RESTRICT hashArray = getHashArray();
		uint32_t locaCapacity = getCapacity();
		for (uint32_t i = 0; i < locaCapacity; ++i)
		{
			if (isValidHash(hashArray[i]))
				return i;
		}
	}

	return getCapacity();
}

uint32_t LinearHashBase::getNextIndex(uint32_t index) const
{
	const uint32_t *FB_RESTRICT hashArray = getHashArray();
	uint32_t locaCapacity = getCapacity();
	for (uint32_t i = index + 1; i < locaCapacity; ++i)
	{
		if (isValidHash(hashArray[i]))
			return i;
	}

	return getCapacity();
}

uint32_t global_default_hash_function(float number)
{
	fb_static_assert(sizeof(float) == 4 && "Suddenly float is not 4 bytes long. Fix global_default_hash_function.");
	uint32_t v;
	memcpy(&v, &number, sizeof(float));
	return getNumberHashValue(v);
}

FB_END_PACKAGE1()
