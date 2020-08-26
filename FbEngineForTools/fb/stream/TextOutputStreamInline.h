#ifndef FB_STREAM_TEXTOUTPUTSTREAMINLINE_H
#define FB_STREAM_TEXTOUTPUTSTREAMINLINE_H

FB_PACKAGE1(stream)

template<class StringType>
void TextOutputStream::write(const StringType &str)
{
	OutputStream<lang::NativeByteOrder>::write(str.getPointer(), str.getLength());
}

template<class StringType>
void TextOutputStream::writeLine(const StringType &str)
{
	OutputStream<lang::NativeByteOrder>::write(str.getPointer(), str.getLength());
	writeLineEnding();
}

FB_END_PACKAGE1()

#endif
