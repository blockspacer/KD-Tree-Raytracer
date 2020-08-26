#ifndef FB_STREAM_TEXTOUTPUTSTREAM_H
#define FB_STREAM_TEXTOUTPUTSTREAM_H

#include "fb/stream/OutputStream.h"

FB_PACKAGE1(stream)

/**
 * Writes text to a memory stream
 */
class TextOutputStream : public OutputStream<lang::NativeByteOrder>
{
public:
	/**
	 * Writes string to stream
	 */
	template<class StringType> void write(const StringType &str);
	void write(const char *str);

	/**
	 * Writes string with line ending to stream
	 */
	template<class StringType> void writeLine(const StringType &str);
	void writeLine(const char *str);

	/**
	 * Writes line ending to stream
	 */
	void writeLineEnding();
};

FB_END_PACKAGE1()

#include "TextOutputStreamInline.h"

#endif
