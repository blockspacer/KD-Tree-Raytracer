#pragma once

#include "fb/algorithm/BinarySearch.h"
#include "fb/profiling/ZoneList.h"
#include "fb/profiling/ZoneProfilerCommon.h"

FB_PACKAGE1(profiling)

struct ZoneCursor
{
	const ZoneList &zl;

	uint32_t syncIndex = ~0U;
	uint32_t firstEvent = 0;
	uint32_t lastEvent = 0;

	uint64_t syncCoBegin = 0;
	double syncCoLength = 1.0;
	double syncSecBegin = 0.0;
	double syncSecLength = 0.0;

	bool syncGood = false;

	ZoneCursor(const ZoneList &zl)
		: zl(zl)
	{
	}

	bool updateSync()
	{
		if (zl.syncs.getSize() < 2 || syncIndex > zl.syncs.getSize() - 2)
			return false;

		const ZoneSync &pre = zl.syncs[syncIndex];
		const ZoneSync &post = zl.syncs[syncIndex + 1];

		if (post.numEvents < pre.numEvents)
			return false;

		firstEvent = pre.numEvents;
		lastEvent = post.numEvents;

		syncCoBegin = pre.coTime;
		syncCoLength = (double)(post.coTime - pre.coTime);
		syncSecBegin = ZoneTimeStamp::convertCpuTimestampToSeconds(pre.cpuTime);
		syncSecLength = ZoneTimeStamp::convertCpuTimestampToSeconds(post.cpuTime) - syncSecBegin;
		return true;
	}

	double getZoneTimeSec(uint64_t timeStamp)
	{
		// 100% predicted branch
		if (zl.isCoZone)
		{
			// TEMPORARILY DEPRECATED!
			// CoZones are not in use on platforms besides NX
			// Blindly adding support for new zones is tough
			fb_assert(!"TEMPORARILY DEPRECATED!");
			// TEMPORARILY DEPRECATED!
			return 0;

			//if (eventIndex >= firstEvent && eventIndex < lastEvent)
			//{
			//	// Happy case: Between same syncs, do nothing
			//}
			//else if (syncIndex != ~0U && syncIndex + 1 < zl.syncs.getSize() && eventIndex >= lastEvent && eventIndex < zl.syncs[syncIndex + 1].numEvents)
			//{
			//	// Happy case pt. 2: Between next syncs, advance sync
			//	syncIndex++;
			//	syncGood = updateSync();
			//}
			//else
			//{
			//	// Sad case: Search for good slot
			//	auto cmp = [](uint32_t eventIndex, const ZoneSync& sync) { return eventIndex < sync.numEvents; };
			//	auto it = algorithm::upperBound(zl.syncs.getBegin(), zl.syncs.getEnd(), eventIndex, cmp);
			//	syncIndex = (uint32_t)(it - zl.syncs.getBegin() - 1);
			//	syncGood = updateSync();
			//}
			//
			//fb_assert(!syncGood || (eventIndex >= firstEvent && eventIndex < lastEvent));
			//double relT = (double)(zl.finishedEvents.vec[eventIndex].timeStart - syncCoBegin) / syncCoLength;
			//double t = syncSecBegin + relT * syncSecLength;
			//return t;
		}
		else
		{
			return ZoneTimeStamp::convertCpuTimestampToSeconds(timeStamp);
		}
	}

};

FB_END_PACKAGE1();
