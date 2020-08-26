#include "Precompiled.h"
#include "fb/util/Checksum.h"

#include "fb/lang/platform/Prefetch.h"

FB_PACKAGE1(util)

namespace {

	uint16_t swapShort(uint16_t i)
	{
		return uint16_t(((i & 0xff00) >> 8) | ((i & 0xff) << 8));
	}

	// Modified version of wikipedia Fletcher-32 article
	uint32_t fletcher32(const void *dataRaw, uint64_t length)
	{
		int32_t len = (int32_t) length;
		uint32_t sum1 = 0xffff;
		uint32_t sum2 = 0xffff;

		// Do we have an odd byte?
		uint8_t oddByte = 0;
		if (len % 2 == 1)
		{
			const uint8_t *FB_RESTRICT data8 = (uint8_t *) dataRaw;
			oddByte = data8[len - 1];
		}

		// Go to 16bit mode
		len /= 2;
		const uint16_t *FB_RESTRICT data = (uint16_t *) dataRaw;

		while (len) 
		{
			int tlen = len > 360 ? 360 : len;
			len -= tlen;
			do 
			{
					uint16_t value = *data++;
					FB_PREFETCH(data + 32);

					#if (FB_BYTE_ORDER == FB_LITTLE_ENDIAN)
						value = swapShort(value);
					#endif

					sum1 += value;
					sum2 += sum1;
			} while (--tlen);

			sum1 = (sum1 & 0xffff) + (sum1 >> 16);
			sum2 = (sum2 & 0xffff) + (sum2 >> 16);
		}

		if (oddByte)
		{
			uint16_t value = oddByte;

			sum1 += value;
			sum2 += sum1;
			sum1 = (sum1 & 0xffff) + (sum1 >> 16);
			sum2 = (sum2 & 0xffff) + (sum2 >> 16);
		}

		// Second reduction step to reduce sums to 16 bits
		sum1 = (sum1 & 0xffff) + (sum1 >> 16);
		sum2 = (sum2 & 0xffff) + (sum2 >> 16);

		uint32_t result = sum2 << 16 | sum1;
		return result;
	}
}

uint32_t calculateChecksumFast(const void *buffer, uint64_t length)
{
	if (!length)
		return 0;

	return fletcher32(buffer, length);
}

FB_END_PACKAGE1()
