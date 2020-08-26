#pragma once

// Includes
#include <limits>
#include <cfloat>
#include "IntTypes.h"

#pragma warning(push)
#pragma warning(disable : 4756)

FB_PACKAGE1(lang)

// Empty default case
template <typename T> class NumericLimits
{
public:
	static constexpr bool isSpecialized = false;

	// Left empty for the base class, values are defined only in specialized templates.
};


template <>
class NumericLimits<bool>
{
private:
	typedef bool T;

public:
	static const bool isSpecialized = true;

	static constexpr T getMin() { return false; }
	static constexpr T getMax() { return true; }
	static T getInfinity() { return T(); }
	static constexpr T getLowest() { return false; }
	static constexpr T getHighest() { return true; }

	static constexpr bool isSigned = false;
	static constexpr bool isInteger = true;
	static constexpr bool isExact = true;
	static constexpr bool hasInfinity = false;
	static constexpr int nonSignBits = 1;
};


template <>
class NumericLimits<float>
{
private:
	typedef float T;

public:
	static const bool isSpecialized = true;

	static constexpr T getMin() { return FLT_MIN; }
	static constexpr T getMax() { return FLT_MAX; }
	static constexpr T getInfinity() { return std::numeric_limits<T>::infinity(); }
	static constexpr T getLowest() { return -FLT_MAX; }
	static constexpr T getHighest() { return FLT_MAX; }

	static const bool isSigned = true;
	static const bool isInteger = false;
	static const bool isExact = false;
	static const bool hasInfinity = true;
	static const int nonSignBits = FLT_MANT_DIG;
};


template <>
class NumericLimits<double>
{
private:
	typedef double T;

public:
	static const bool isSpecialized = true;

	static constexpr T getMin() { return DBL_MIN; }
	static constexpr T getMax() { return DBL_MAX; }
	static constexpr T getInfinity() { return std::numeric_limits<T>::infinity(); }
	static constexpr T getLowest() { return -DBL_MAX; }
	static constexpr T getHighest() { return DBL_MAX; }

	static constexpr bool isSigned = true;
	static constexpr bool isInteger = false;
	static constexpr bool isExact = false;
	static constexpr bool hasInfinity = true;
	static constexpr int nonSignBits = DBL_MANT_DIG;
};

// Create exact type specialisations
#define FB_DECLARE_EXACT_TYPE(p_type) \
	template <> \
	class NumericLimits<p_type> \
	{ \
	private: \
		typedef p_type T; \
		\
	public: \
		static const bool isSpecialized = true; \
		\
		static constexpr T getMin() { return std::numeric_limits<T>::min(); } \
		static constexpr T getMax() { return std::numeric_limits<T>::max(); } \
		static T getInfinity() { return std::numeric_limits<T>::infinity(); } \
		static constexpr T getLowest() { return std::numeric_limits<T>::lowest(); } \
		static constexpr T getHighest() { return std::numeric_limits<T>::max(); } \
		\
		static constexpr bool isSigned = std::numeric_limits<T>::is_signed; \
		static constexpr bool isInteger = std::numeric_limits<T>::is_integer; \
		static constexpr bool isExact = std::numeric_limits<T>::is_exact; \
		static constexpr bool hasInfinity = std::numeric_limits<T>::has_infinity; \
		static constexpr int nonSignBits = std::numeric_limits<T>::digits; \
	};

FB_DECLARE_EXACT_TYPE(int8_t);
FB_DECLARE_EXACT_TYPE(int16_t);
FB_DECLARE_EXACT_TYPE(int32_t);
FB_DECLARE_EXACT_TYPE(int64_t);
FB_DECLARE_EXACT_TYPE(uint8_t);
FB_DECLARE_EXACT_TYPE(uint16_t);
FB_DECLARE_EXACT_TYPE(uint32_t);
FB_DECLARE_EXACT_TYPE(uint64_t);

FB_DECLARE_EXACT_TYPE(signed long);
FB_DECLARE_EXACT_TYPE(unsigned long);

#undef FB_DECLARE_EXACT_TYPE

FB_END_PACKAGE1()

#pragma warning(pop)
