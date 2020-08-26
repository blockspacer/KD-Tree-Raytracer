#pragma once

#include "fb/lang/Types.h"
#include "fb/profiling/ZoneType.h"
#include "fb/profiling/ZoneDefine.h"
#include "fb/string/StringLiteral.h"

FB_DECLARE0(DynamicString)
FB_DECLARE0(StaticString)
FB_DECLARE0(StringRef)
FB_DECLARE(string, StringLiteral)

FB_PACKAGE1(profiling)


void pushZone(const string::StringLiteral &name, ZoneType type = ZoneWork);
void pushZone(const StaticString &name, ZoneType type = ZoneWork);
void pushZone(const DynamicString &name, ZoneType type = ZoneWork);
void pushZone(const char *name, ZoneType type = ZoneWork);

void pushZoneTrustMe(const char *name, ZoneType type);
class TrustMe {};
static inline void pushZone(TrustMe, const char *name, ZoneType type = ZoneWork) { pushZoneTrustMe(name, type); }

void popZone();
void pushZoneStart(); // Start timing when this special zone is pushed

// TODO: Move beginThread and endThread to a common profiling utility, all profilers depend on them
// NOTE: ZoneProfiler needs to do some initialization when beginThread is called
void beginThread(const char *name);
void endThread();

// TODO: Move heart beat to it's own profiler utility
// NOTE: ZoneProfiler settings are currently applied when heartBeat is called
void heartBeat();
float getLastHeartBeatInterval();

void applySettings();

typedef uint32_t CoThread;

CoThread createCoThread(const char *name);
void destroyCoThread(CoThread thread);
void syncCoThread(CoThread thread, uint64_t coTime, double coTimeToSeconds);
uint64_t convertCoTime(CoThread thread, uint64_t coTime);

void pushCoZone(CoThread thread, uint64_t coTime, const char *name, ZoneType type = ZoneWork);
void popCoZone(CoThread thread, uint64_t coTime);

void dumpProfileData(double maxTime, const StringRef &filepath);

struct ScopedZone
{
	ScopedZone(const DynamicString &name, ZoneType type = profiling::ZoneWork) { pushZone(name, type); }
	ScopedZone(const StaticString &name, ZoneType type = profiling::ZoneWork) { pushZone(name, type); }
	ScopedZone(string::StringLiteral name, ZoneType type = profiling::ZoneWork) { pushZone(name, type); }
	ScopedZone(const char *name, ZoneType type = profiling::ZoneWork) { pushZone(name, type); }
	ScopedZone(TrustMe, const char *name, ZoneType type = profiling::ZoneWork) { pushZoneTrustMe(name, type); }
	~ScopedZone() { popZone(); }
};

void disableThreadLeakAssert();

const char* pushTempZoneString(const StringRef &str);
const char* pushTempZoneString(const char* str, const SizeType strlen);

FB_END_PACKAGE1()

#if FB_USE_ZONE_PROFILER == FB_TRUE
#define FB_ZONE(...) ::fb::profiling::ScopedZone FB_PP_CONCAT(fb_impl_zone_guard, __LINE__)(__VA_ARGS__)
#define FB_ZONE_CONST_CHAR(...) ::fb::profiling::ScopedZone FB_PP_CONCAT(fb_impl_zone_guard, __LINE__)(::fb::profiling::TrustMe{}, __VA_ARGS__)
#define FB_ZONE_FUNC() ::fb::profiling::ScopedZone FB_PP_CONCAT(fb_impl_zone_guard, __LINE__)(::fb::profiling::TrustMe{}, __FUNCTION__)
#define FB_ZONE_FMT(...) FB_ZONE(::fb::profiling::TrustMe{}, ::fb::profiling::pushTempZoneString(::fb::TempString().doSprintf(__VA_ARGS__)))
#define FB_ZONE_ENTER(...) do { ::fb::profiling::pushZone(__VA_ARGS__); } while (false)
#define FB_ZONE_ENTER_CONST_CHAR(...) do { ::fb::profiling::pushZone(::fb::profiling::TrustMe{}, __VA_ARGS__); } while (false)
#define FB_ZONE_ENTER_FMT(...) do { ::fb::profiling::pushZone(::fb::profiling::TrustMe{}, ::fb::profiling::pushTempZoneString(::fb::TempString().doSprintf(__VA_ARGS__))); } while (false)
#define FB_ZONE_EXIT() do { ::fb::profiling::popZone(); } while (false)

#define FB_ZONE_START() do { ::fb::profiling::pushZoneStart(); } while(false)
#else
#define FB_ZONE(...) do { } while (false)
#define FB_ZONE_CONST_CHAR(...) do { } while (false)
#define FB_ZONE_FUNC() do { } while (false)
#define FB_ZONE_FMT(...) do { } while (false)
#define FB_ZONE_ENTER(...) do { } while (false)
#define FB_ZONE_ENTER_CONST_CHAR(...) do { } while (false)
#define FB_ZONE_ENTER_FMT(...) do { } while (false)
#define FB_ZONE_EXIT() do { } while (false)
#define FB_ZONE_START() do { } while(false)
#endif
