#pragma once

FB_DECLARE0(HeapString)

FB_PACKAGE0()

//
// Example usage:
// 1) FB_LOG_ERROR(FB_MSG("Bad thing happened at ", AppendSingleRowCallstack()));
//
// 2) FB_LOG_ERROR("Bad thing happened. See callstack in next message.");
//    FB_LOG_DEBUG(FB_MSG(AppendCallstack()));
//
// 3) FB_LOG_INFO(FB_MSG("I want to specifically use Zone callstack here ", AppendSingleRowZoneCallstack()));
//    Can be used when the normal callstack does "work" but produces unhelpful results like: "(unknown), (unknown), (unknown), (unknown)".
//

struct AppendCallstack { };
struct AppendSingleRowCallstack { };
struct AppendSingleRowZoneCallstack { };

HeapString &debugAppendToString(HeapString &result, const AppendCallstack &);
HeapString &debugAppendToString(HeapString &result, const AppendSingleRowCallstack &);
HeapString &debugAppendToString(HeapString &result, const AppendSingleRowZoneCallstack &);

FB_END_PACKAGE0()
