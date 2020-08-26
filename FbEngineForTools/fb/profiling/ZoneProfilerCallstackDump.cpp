#include "Precompiled.h"
#include "ZoneProfilerCallstackDump.h"

#include "fb/lang/Atomics.h"
#include "fb/profiling/ZoneList.h"
#include "fb/profiling/ZoneCursor.h"
#include "fb/profiling/ZoneProfilerCommon.h"
#include "fb/profiling/ZoneDefine.h"

FB_PACKAGE1(profiling)

void ZoneProfilerCallstackDump::dumpProfileCallstackZoneList(const Args &args, const ZoneList &zl, PodVector<ZoneCallstackFrame>& outZones)
{
#if FB_USE_ZONE_PROFILER == FB_TRUE

	fb_assert(atomicLoadRelaxed(zl.listFreezeFlag));

	if (args.optionalTimeStamp == 0 && args.startTime == 0)
	{
		// Only care about currently entered zones

		for (const ZoneEnter &e : zl.enterEvents)
		{
			ZoneCallstackFrame &f = outZones.pushBack();
			f.name = e.id;
			f.startTime = ZoneTimeStamp::convertCpuTimestampToSeconds(e.time);
			f.zoneType = e.zoneType == profiling::ZoneWork ? DumpZoneTypeWork : DumpZoneTypeBlock;
		}
		return;
	}

	const uint64_t nowTime = args.optionalTimeStamp == 0 ? ZoneTimeStamp::getCpuTimestamp() : args.optionalTimeStamp;
	uint64_t startTimeLength = ZoneTimeStamp::convertSecondsToCpuTimestamp(args.startTime);
	uint64_t startTime = nowTime >= startTimeLength ? nowTime - startTimeLength : 0;

	const SizeType startIndex = ZoneRingBound::lower(zl.finishedEvents, startTime);
	const SizeType endIndex = zl.finishedEvents.write;

	// NOTE: The timestamps here have been converted from absolute seconds in double to
	// uint64_t ticks blindly, everything is very likely to be broken. -Samuli_R 27.5.2019

	SizeType previousLevel = ~0U;
	for (SizeType index = startIndex; index != endIndex; index = zl.finishedEvents.toIndex(index + 1))
	{
		if (zl.finishedEvents.vec[index].level >= previousLevel)
			continue;

		const ZoneEvent e = zl.finishedEvents.vec[index];
		previousLevel = e.level;

		ZoneCallstackFrame &frame =  outZones.pushBack();
		frame.name = e.id;
		frame.startTime = -ZoneTimeStamp::convertCpuTimestampToSeconds(e.timeStart >= startTime ? e.timeStart - startTime : 0);
		frame.zoneType = e.zoneType == ZoneWork ? DumpZoneTypeWork : DumpZoneTypeBlock;
	
		if (outZones.getSize() >= args.maxDepth)
			return;
		else if (frame.startTime >= args.maxDuration)
			return;
	}

	if (previousLevel > zl.enterEvents.getSize())
		previousLevel = zl.enterEvents.getSize();

	for ( ; previousLevel-- > 0; )
	{
		const ZoneEnter e = zl.enterEvents[previousLevel];
		ZoneCallstackFrame &frame = outZones.pushBack();
		frame.name = e.id;
		frame.startTime = -ZoneTimeStamp::convertCpuTimestampToSeconds(e.time >= startTime ? e.time - startTime : 0);
		frame.zoneType = e.zoneType == ZoneWork ? DumpZoneTypeWork : DumpZoneTypeBlock;

		if (outZones.getSize() >= args.maxDepth)
			return;
		else if (frame.startTime >= args.maxDuration)
			return;
	}

#endif
}

// From ZoneProfiler.cpp
extern void freezeZoneList(ZoneList *zl);
extern void unfreezeZoneList(ZoneList *zl);
extern ZoneList *getZoneListExternal(SizeType zoneListIndex);
extern ZoneList *getZoneList();

void ZoneProfilerCallstackDump::dumpProfileCallstackOnThread(const Args &args, SizeType threadIndex, ZoneProfilerCallstackDump::ResultVector& outZones)
{
#if FB_USE_ZONE_PROFILER == FB_TRUE

	ZoneList *zl = getZoneListExternal(threadIndex);
	if (!zl)
		return;

	freezeZoneList(zl);

	dumpProfileCallstackZoneList(args, *zl, outZones);

	unfreezeZoneList(zl);

#endif
}
void ZoneProfilerCallstackDump::dumpProfileCallstack(const Args &args, ZoneProfilerCallstackDump::ResultVector& outZones)
{
#if FB_USE_ZONE_PROFILER == FB_TRUE

	ZoneList *zl = getZoneList();
	if (!zl)
		return;

	freezeZoneList(zl);

	dumpProfileCallstackZoneList(args, *zl, outZones);

	unfreezeZoneList(zl);

#endif
}

FB_END_PACKAGE1()
