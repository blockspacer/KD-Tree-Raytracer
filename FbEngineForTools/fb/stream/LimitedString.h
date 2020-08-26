#pragma once

FB_PACKAGE0()

bool streamReadFailed(const char *file, int line);

// Streamable string with a length limit
template<class StringClass, int MaxCharacters>
class LimitedString : public StringClass
{
public:
	LimitedString()
	{
	}

	LimitedString(const StringClass &other)
	{
		*this = other;
	}

	void operator=(const StringClass &other)
	{
		StringClass &baseClass = *this;
		baseClass = other;
		if (StringClass::getLength() > MaxCharacters)
			StringClass::truncateToSize(MaxCharacters);
	}

	template<class T>
	bool stream(T &strm)
	{
		return streamImpl(strm, typename T::StreamType());
	}

	static const uint32_t getMaxLength()
	{
		return MaxCharacters;
	}

	static ClassId getStaticClassId() { return FB_FOURCC('L', 'i', 'm', 'S'); }

	typedef void ImplementsStringRef;

private:
	template<class T>
	bool streamImpl(T &strm, OutputStreamType)
	{
		SizeType length = lang::min(StringClass::getLength(), (SizeType)MaxCharacters);
		fb_stream_write(strm, StringClass::getPointer(), length);
		uint8_t zero = 0;
		fb_stream_write(strm, zero);
		return true;
	}

	template<class T>
	bool streamImpl(T &strm, InputStreamType)
	{
		uint8_t data[MaxCharacters + 1];
		for (SizeType i = 0; i <= MaxCharacters; i++)
		{
			fb_stream_read(strm, data[i]);
			if (data[i] == '\0')
			{
				StringClass &baseClass = *this;
				baseClass = StringClass((const char *)data);
				return true;
			}
		}
		return streamReadFailed(FB_ASSERT_FILENAME, FB_ASSERT_LINENUMBER);
	}
};

FB_END_PACKAGE0()
