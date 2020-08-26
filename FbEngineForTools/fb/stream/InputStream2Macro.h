#pragma once

FB_PACKAGE0()
bool streamReadFailed(const char *file, int line);
FB_END_PACKAGE0();

#define fb_stream_read(strm, ...) do { if (!(strm).readImpl(__VA_ARGS__)) return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER); } while (false)
#define fb_stream_setPosition(strm, value) do { if (!(strm).setPositionImpl(value)) return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER); } while (false)
#define fb_stream_skipData(strm, value) do { if (!(strm).skipDataImpl(value)) return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER); } while (false)
