#include "Precompiled.h"
#include "Div128.h"

#include "fb/lang/Types.h"

FB_PACKAGE0()

#if FB_COMPILER == FB_MSC && FB_PLATFORM_BITS == 64

// this is ridiculous, MSVC really has no intrinsic for x64 DIV ????????

#pragma section(".div", read, execute)  

__declspec(allocate(".div")) const unsigned char udiv128Data[] =
{
	0x48, 0x89, 0xD0, // mov rax,rdx
	0x48, 0x89, 0xCA, // mov rdx,rcx
	0x49, 0xF7, 0xF0, // div r8
	0x49, 0x89, 0x11, // mov [r9],rdx
	0xC3              // ret
};

__declspec(allocate(".div")) static const unsigned char sdiv128Data[] =
{
	0x48, 0x89, 0xD0, // mov rax,rdx
	0x48, 0x89, 0xCA, // mov rdx,rcx
	0x49, 0xF7, 0xF8, // idiv r8
	0x49, 0x89, 0x11, // mov [r9],rdx
	0xC3              // ret
};

unsigned __int64(__fastcall *udiv128)(unsigned __int64 numhi, unsigned __int64 numlo, unsigned __int64 den, unsigned __int64* rem)
	= (unsigned __int64(__fastcall *)(unsigned __int64, unsigned __int64, unsigned __int64, unsigned __int64*))(const  void*)udiv128Data;
__int64(__fastcall *sdiv128)(__int64 numhi,	__int64 numlo,	__int64 den, __int64* rem)
	= (__int64(__fastcall *)(__int64, __int64, __int64, __int64*))(const  void*)sdiv128Data;

#endif

void udiv128By64(uint64_t a_hi, uint64_t a_lo, uint64_t b, uint64_t &quotient, uint64_t &remainder)
{
#if FB_COMPILER == FB_MSC && FB_PLATFORM_BITS == 64
	quotient = udiv128(a_hi, a_lo, b, &remainder);
#else
	// quotient
	uint64_t q = a_lo << 1;

	// remainder
	uint64_t rem = a_hi;

	uint64_t carry = a_lo >> 63;
	uint64_t temp_carry = 0;

	for (SizeType i = 0; i < 64; i++)
	{
		temp_carry = rem >> 63;
		rem <<= 1;
		rem |= carry;
		carry = temp_carry;

		if (carry == 0)
		{
			if (rem >= b)
			{
				carry = 1;
			}
			else
			{
				temp_carry = q >> 63;
				q <<= 1;
				q |= carry;
				carry = temp_carry;
				continue;
			}
		}

		rem -= b;
		rem -= (1 - carry);
		carry = 1;
		temp_carry = q >> 63;
		q <<= 1;
		q |= carry;
		carry = temp_carry;
	}

	quotient = q;
	remainder = rem;
#endif
}

FB_END_PACKAGE0()