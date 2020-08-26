#pragma once

#include "fb/lang/IntTypes.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/platform/ByteOrder.h"

FB_PACKAGE1(lang)


inline void swapByteOrder2(void *p)
{
	uint8_t *ptr = reinterpret_cast<uint8_t*>(p);
	uint8_t copy[2];
	copy[0] = ptr[0];
	copy[1] = ptr[1];

	ptr[0] = copy[1];
	ptr[1] = copy[0];
}

inline void swapByteOrder4(void *p)
{
	uint8_t *ptr = reinterpret_cast<uint8_t*>(p);
	uint8_t copy[4];
	copy[0] = ptr[0];
	copy[1] = ptr[1];
	copy[2] = ptr[2];
	copy[3] = ptr[3];

	ptr[0] = copy[3];
	ptr[1] = copy[2];
	ptr[2] = copy[1];
	ptr[3] = copy[0];
}

inline void swapByteOrder8(void *p)
{
	uint8_t *ptr = reinterpret_cast<uint8_t*>(p);
	uint8_t copy[8];
	copy[0] = ptr[0];
	copy[1] = ptr[1];
	copy[2] = ptr[2];
	copy[3] = ptr[3];
	copy[4] = ptr[4];
	copy[5] = ptr[5];
	copy[6] = ptr[6];
	copy[7] = ptr[7];

	ptr[0] = copy[7];
	ptr[1] = copy[6];
	ptr[2] = copy[5];
	ptr[3] = copy[4];
	ptr[4] = copy[3];
	ptr[5] = copy[2];
	ptr[6] = copy[1];
	ptr[7] = copy[0];
}

inline void swapByteOrder(float &f)
{
	union FloatInt { float f; int i; };
	FloatInt fi;
	fi.f = f;
	swapByteOrder4(&fi.i);
	f = fi.f;
}

inline void swapByteOrder(double &f)
{
	union DoubleInt { double f; int64_t i; };
	DoubleInt fi;
	fi.f = f;
	swapByteOrder8(&fi.i);
	f = fi.f;
}

inline void swapByteOrder(uint8_t &i) { }
inline void swapByteOrder(uint16_t &i) { swapByteOrder2(&i); }
inline void swapByteOrder(uint32_t &i) { swapByteOrder4(&i); }
inline void swapByteOrder(uint64_t &i) { swapByteOrder8(&i); }
inline void swapByteOrder(char &i) { }
inline void swapByteOrder(int8_t &i) { }
inline void swapByteOrder(int16_t &i) { swapByteOrder2(&i); }
inline void swapByteOrder(int32_t &i) { swapByteOrder4(&i); }
inline void swapByteOrder(int64_t &i) { swapByteOrder8(&i); }


#if (FB_BYTE_ORDER == FB_LITTLE_ENDIAN)

	class BigEndian
	{
	public:
		template<class T>
		static void convertToNative(T &t) { swapByteOrder(t); }
		template<class T>
		static void convertFromNative(T &t) { swapByteOrder(t); }

		static bool compareToNative(const void *p1, const void *p2, size_t size)
		{
			// Inefficient, assuming it won't be called often
			const char *c1 = (const char *) p1;
			const char *c2 = (const char *) p2;
			c2 += size - 1;
			for (size_t i = 0; i < size; ++i)
			{
				if (*c1 != *c2)
					return false;

				++c1;
				--c2;
			}

			return true;
		}
	};

	class LittleEndian
	{
	public:
		template<class T>
		static void convertToNative(T &t) { }
		template<class T>
		static void convertFromNative(T &t) { }

		static bool compareToNative(const void *p1, const void *p2, size_t size)
		{
			return MemCompare::equals(p1, p2, size);
		}
	};

	typedef LittleEndian NativeByteOrder;
	typedef BigEndian NonNativeByteOrder;

#elif (FB_BYTE_ORDER == FB_BIG_ENDIAN)

	class BigEndian
	{
	public:
		template<class T>
		static void convertToNative(T &t) { }
		template<class T>
		static void convertFromNative(T &t) { }

		static bool compareToNative(const void *p1, const void *p2, size_t size)
		{
			return MemCompare::equals(p1, p2, size);
		}
	};

	class LittleEndian
	{
	public:
		template<class T>
		static void convertToNative(T &t) { swapByteOrder(t); }
		template<class T>
		static void convertFromNative(T &t) { swapByteOrder(t); }

		static bool compareToNative(const void *p1, const void *p2, size_t size)
		{
			// Inefficient, assuming it won't be called often
			const char *c1 = (const char *) p1;
			const char *c2 = (const char *) p2;
			c2 += size - 1;
			for (size_t i = 0; i < size; ++i)
			{
				if (*c1 != *c2)
					return false;

				++c1;
				--c2;
			}

			return true;
		}
	};

	typedef BigEndian NativeByteOrder;
	typedef LittleEndian NonNativeByteOrder;

#else
	#error "Unknown byte order (FB_BYTE_ORDER not defined?)"
#endif

FB_END_PACKAGE1()
