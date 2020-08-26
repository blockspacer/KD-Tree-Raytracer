#pragma once

FB_PACKAGE1(util)

// High performance logging similar to FB_PRINTF, except logs directly file (log/%ProcessID%.log)

#if FB_BUILD != FB_FINAL_RELEASE
	#define FB_SPAMF(...) \
		fb::util::spamF(__VA_ARGS__)
#else
	#define FB_SPAMF(...)
#endif

void spamF(const char *fmt, ...);

FB_END_PACKAGE1()
