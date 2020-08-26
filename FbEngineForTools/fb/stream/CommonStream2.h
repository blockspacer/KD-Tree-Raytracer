#pragma once

FB_PACKAGE0()

struct InputStreamType {};
struct OutputStreamType {};
struct StreamPersistentTrue { static const int value = 1; };
struct StreamPersistentFalse {static const int value = 0; };
struct StreamClassByte {};

FB_END_PACKAGE0()
