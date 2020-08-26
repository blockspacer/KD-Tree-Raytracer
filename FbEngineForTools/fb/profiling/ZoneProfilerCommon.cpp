#include "Precompiled.h"
#include "ZoneProfilerCommon.h"

#include "fb/profiling/ZoneRing.h"

FB_PACKAGE1(profiling)

SizeType ZoneRingBound::lower(const ZoneRing &ring, uint64_t t)
{
	ZoneRing::Iter begin = 0;
	ZoneRing::Iter end = 0;
	if (ring.hasTwoHalves())
	{
		ZoneRing::Iter secondHalfBeging = ring.getBeginSecondHalf();
		if (ring[secondHalfBeging].timeEnd < t)
		{
			begin = ring.getBeginSecondHalf();
			end = ring.getEndSecondHalf();
		}
		else
		{
			begin = ring.getBeginFirstHalf();
			end = ring.getEndFirstHalf();
		}
	}
	else
	{
		begin = ring.getBeginOnlyHalf();
		end = ring.getEndOnlyHalf();
	}

	fb_assert(begin.index <= end.index);

	SizeType count = SizeType(end.index - begin.index);
	while (count > 0)
	{
		ZoneRing::Iter it = begin;
		SizeType step = count / 2;
		it.index += step;
		if (ring.vec[it.index].timeEnd < t)
		{
			begin = ++it.index;
			count -= step + 1;
		}
		else
			count = step;
	}
	fb_assertf(begin.index <= ring.vec.getSize(), "%d <= %d", begin.index, ring.vec.getSize());
	return ring.toIndex(begin.index);
}
SizeType ZoneRingBound::upper(const ZoneRing &ring, SizeType startIndex, uint64_t nowTime)
{
	SizeType endIndex = ring.write;
	while (endIndex != startIndex)
	{
		SizeType prevIndex = ring.toIndexHandleNegative((int)endIndex - 1);
		if (ring.vec[prevIndex].timeStart > nowTime)
		{
			endIndex = prevIndex;
			continue;
		}
		break;
	}
	fb_assert(endIndex == startIndex || ring.vec[ring.toIndexHandleNegative((int)endIndex - 1)].id);
	return ring.toIndex(endIndex);
}

thread_local uint32_t ZoneThreadId::t_zoneListIndex = 0;

FB_END_PACKAGE1()
