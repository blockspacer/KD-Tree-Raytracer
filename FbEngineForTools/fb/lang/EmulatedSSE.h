#ifndef FB_LANG_EMULATEDSSE_H
#define FB_LANG_EMULATEDSSE_H

struct __m128
{
	union
	{
		float f[4];
		int i[4];
	};
};

typedef __m128 __m128i;

static __m128 _mm_castsi128_ps(__m128i a)
{
	return a;
}

static __m128 _mm_set1_ps(float f)
{
	__m128i r;
	r.f[0] = f;
	r.f[1] = f;
	r.f[2] = f;
	r.f[3] = f;
	return r;
}

static __m128i _mm_set1_epi32(int i)
{
	__m128i r;
	r.i[0] = i;
	r.i[1] = i;
	r.i[2] = i;
	r.i[3] = i;
	return r;
}

#define _MM_SHUFFLE(z, y, x, w) ((z<<6) | (y<<4) | (x<<2) | w)
static __m128 _mm_shuffle_ps(__m128 a, __m128 b, int mask)
{
	int w = (mask>>0) & 3;
	int x = (mask>>2) & 3;
	int y = (mask>>4) & 3;
	int z = (mask>>6) & 3;
	__m128i r;
	r.f[0] = a.f[w];
	r.f[1] = a.f[x];
	r.f[2] = b.f[y];
	r.f[3] = b.f[z];
	return r;
}

static __m128 _mm_add_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.f[0] = a.f[0] + b.f[0];
	r.f[1] = a.f[1] + b.f[1];
	r.f[2] = a.f[2] + b.f[2];
	r.f[3] = a.f[3] + b.f[3];
	return r;
}

static __m128 _mm_add_ss(__m128 a, __m128 b)
{
	__m128i r;
	r.f[0] = a.f[0] + b.f[0];
	r.f[1] = r.f[0];
	r.f[2] = r.f[1];
	r.f[3] = r.f[2];
	return r;
}

static __m128 _mm_sub_ss(__m128 a, __m128 b)
{
	__m128i r;
	r.f[0] = a.f[0] - b.f[0];
	r.f[1] = r.f[0];
	r.f[2] = r.f[1];
	r.f[3] = r.f[2];
	return r;
}

static __m128 _mm_sub_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.f[0] = a.f[0] - b.f[0];
	r.f[1] = a.f[1] - b.f[1];
	r.f[2] = a.f[2] - b.f[2];
	r.f[3] = a.f[3] - b.f[3];
	return r;
}

static __m128 _mm_mul_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.f[0] = a.f[0] * b.f[0];
	r.f[1] = a.f[1] * b.f[1];
	r.f[2] = a.f[2] * b.f[2];
	r.f[3] = a.f[3] * b.f[3];
	return r;
}

static __m128 _mm_or_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] | b.i[0];
	r.i[1] = a.i[1] | b.i[1];
	r.i[2] = a.i[2] | b.i[2];
	r.i[3] = a.i[3] | b.i[3];
	return r;
}

static __m128 _mm_xor_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] ^ b.i[0];
	r.i[1] = a.i[1] ^ b.i[1];
	r.i[2] = a.i[2] ^ b.i[2];
	r.i[3] = a.i[3] ^ b.i[3];
	return r;
}

static __m128 _mm_and_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] & b.i[0];
	r.i[1] = a.i[1] & b.i[1];
	r.i[2] = a.i[2] & b.i[2];
	r.i[3] = a.i[3] & b.i[3];
	return r;
}

static __m128 _mm_and_si128(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] & b.i[0];
	r.i[1] = a.i[1] & b.i[1];
	r.i[2] = a.i[2] & b.i[2];
	r.i[3] = a.i[3] & b.i[3];
	return r;
}

static __m128 _mm_or_si128(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] | b.i[0];
	r.i[1] = a.i[1] | b.i[1];
	r.i[2] = a.i[2] | b.i[2];
	r.i[3] = a.i[3] | b.i[3];
	return r;
}

static __m128 _mm_cmpeq_epi32(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.i[0] == b.i[0] ? ~0 : 0;
	r.i[1] = a.i[1] == b.i[1] ? ~0 : 0;
	r.i[2] = a.i[2] == b.i[2] ? ~0 : 0;
	r.i[3] = a.i[3] == b.i[3] ? ~0 : 0;
	return r;
}

static __m128 _mm_andnot_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = ~a.i[0] & b.i[0];
	r.i[1] = ~a.i[1] & b.i[1];
	r.i[2] = ~a.i[2] & b.i[2];
	r.i[3] = ~a.i[3] & b.i[3];
	return r;
}

static __m128 _mm_setzero_ps()
{
	__m128i r;
	r.i[0] = 0;
	r.i[1] = 0;
	r.i[2] = 0;
	r.i[3] = 0;
	return r;
}

static __m128 _mm_cmpgt_ps(__m128 a, __m128 b)
{
	__m128i r;
	r.i[0] = a.f[0] > b.f[0] ? ~0 : 0;
	r.i[1] = a.f[1] > b.f[1] ? ~0 : 0;
	r.i[2] = a.f[2] > b.f[2] ? ~0 : 0;
	r.i[3] = a.f[3] > b.f[3] ? ~0 : 0;
	return r;
}

static __m128 _mm_setr_ps(float z, float y, float x, float w)
{
	__m128i r;
	r.f[0] = z;
	r.f[1] = y;
	r.f[2] = x;
	r.f[3] = w;
	return r;
}

static __m128 _mm_setr_epi32(int z, int y, int x, int w)
{
	__m128i r;
	r.i[0] = z;
	r.i[1] = y;
	r.i[2] = x;
	r.i[3] = w;
	return r;
}

static __m128i _mm_loadu_si128(const __m128i *src)
{
	return *src;
}

static __m128 _mm_loadu_ps(const float *src)
{
	__m128 r;
	memcpy(r.f, src, sizeof(r));
	return r;
}

static void _mm_storeu_ps(float *dest, __m128 a)
{
	memcpy(dest, a.f, sizeof(a));
}

static void _mm_storeu_si128(__m128i *dest, __m128 a)
{
	memcpy(dest, &a, sizeof(a));
}

static int _mm_movemask_ps(__m128 a)
{
	struct Impl
	{
		static int sign(int i)
		{
			return ((uint32_t)i >> 31) & 1;
		}
	};
	return (Impl::sign(a.i[3])<<3) | (Impl::sign(a.i[2])<<2) | (Impl::sign(a.i[1])<<1) | (Impl::sign(a.i[0]));
}

#endif
