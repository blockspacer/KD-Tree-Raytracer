#include "Precompiled.h"
#include "StringToNumberConversion.h"

#include "fb/lang/BooleanTemplateType.h"
#include "fb/lang/EnableIf.h"
#include "fb/lang/NumericLimits.h"
#include "fb/math/util/IsFinite.h"
#include "fb/string/HeapString.h"
#include "fb/string/StringRef.h"

#include <stdio.h> // For sscanf
#include <cmath> // For std::round

FB_PACKAGE2(string, detail)

static const StringRef trueStr("true");
static const StringRef falseStr("false");

bool parseBool(const StringRef &str, bool &resultOut)
{
	// first try the most usual, expected, case.
	if (parseBoolStrictly(str, resultOut))
		return true;

	// try the case insensitive next...
	if (str.compareCaseInsensitive(trueStr))
	{
		resultOut = true;
		return true;
	}
	else if (str.compareCaseInsensitive(falseStr))
	{
		resultOut = false;
		return true;
	}

	// failure.
	resultOut = false;
	return false;
}


bool parseBoolLoosely(const StringRef &str, bool &resultOut)
{
	// first try the most usual, expected, case.
	if (parseBool(str, resultOut))
		return true;

	CacheHeapString<16> tmp(str);
	tmp.trimWhiteSpace();
	if (tmp.isEmpty())
	{
		// an empty string is considered as false.
		resultOut = false;
		return true;
	}

	if (parseBool(tmp, resultOut))
		return true;

	int32_t intValue = 0;
	if (parseNumber(tmp, intValue))
	{
		// zero integer is considered as false, any other true
		resultOut = (intValue != 0);
		return true;
	}

	// failure.
	resultOut = false;
	return false;
}


bool parseBoolStrictly(const StringRef &str, bool &resultOut)
{
	if (str == trueStr)
	{
		resultOut = true;
		return true;
	}
	else if (str == falseStr)
	{
		resultOut = false;
		return true;
	}
	// failure.
	resultOut = false;
	return false;
}


bool validateDigitsOnly(const StringRef &str)
{
	SizeType length = str.getLength();
	for (SizeType i = 0; i < length; i++)
	{
		if (!(str[i] >= '0' && str[i] <= '9') && (str[i] != '-' || i != 0))
		{
			return false;
		}
	}
	return true;
}

template<typename IntType>
struct IsSigned
{
	static constexpr bool value = IntType(-1) < IntType(0);
};

template<typename IntType>
typename lang::EnableIf<IsSigned<IntType>::value, bool>::type parseIntegerImpl(const StringRef &str, IntType &resultOut)
{
	if (!validateDigitsOnly(str))
	{
		resultOut = (IntType)0;
		return false;
	}
	int64_t resultInternal = 0;
	int got = sscanf(str.getPointer(), "%" FB_FSI64, &resultInternal);
	if (got == 1 && resultInternal <= lang::NumericLimits<IntType>::getMax() && resultInternal >= lang::NumericLimits<IntType>::getMin())
	{
		resultOut = IntType(resultInternal);
		return true;
	}
	resultOut = 0;
	return false;
}


template<typename IntType>
typename lang::EnableIf<!IsSigned<IntType>::value, bool>::type parseIntegerImpl(const StringRef &str, IntType &resultOut)
{
	if (!validateDigitsOnly(str))
	{
		resultOut = (IntType)0;
		return false;
	}
	uint64_t resultInternal = 0;
	int got = sscanf(str.getPointer(), "%" FB_FSU64, &resultInternal);
	if (got == 1 && resultInternal <= lang::NumericLimits<IntType>::getMax() && resultInternal >= lang::NumericLimits<IntType>::getMin())
	{
		resultOut = IntType(resultInternal);
		return true;
	}
	resultOut = 0;
	return false;
}


bool parseNumber(const StringRef &str, int8_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, uint8_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, int16_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, uint16_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, int32_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, uint32_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, int64_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


bool parseNumber(const StringRef &str, uint64_t &resultOut)
{
	return parseIntegerImpl(str, resultOut);
}


template <typename NumberType>
bool parseNumberOperationImpl(const StringRef &str, NumberType &resultOut)
{
	struct Operation
	{
		bool isValid()
		{
			return op != OpInvalid;
		}

		void setOperator(char charOp)
		{
			switch (charOp)
			{
			case '*':
				op = OpMul;
				break;
			case '/':
				op = OpDiv;
				break;
			case '+':
				op = OpAdd;
				break;
			case '-':
				op = OpSub;
				break;
			default:
				op = OpInvalid;
			}
		}

		bool calculate()
		{
			switch (op)
			{
			case OpMul:
				result = leftOperand * rightOperand;
				break;
			case OpDiv:
				result = leftOperand / rightOperand;
				break;
			case OpAdd:
				result = leftOperand + rightOperand;
				break;
			case OpSub:
				result = leftOperand - rightOperand;
				break;
			default:
				return false;
			}
			if (math::util::isFinite(result))
				return true;

			result = 0.0;
			return false;
		}

		enum Operator
		{
			OpInvalid,
			OpMul,
			OpDiv,
			OpAdd,
			OpSub
		};
		double leftOperand = 0.0;
		double rightOperand = 0.0;
		double result = 0.0;
		Operator op = OpInvalid;

	};

	resultOut = NumberType(0);
	TempString trimmedStr(str);
	trimmedStr.trimWhiteSpace();
	/* Ignore first character, as that may be sign */
	for (SizeType i = 1, length = trimmedStr.getLength(); i < length; i++)
	{
		if ((trimmedStr[i] != '*') && (trimmedStr[i] != '/') && (trimmedStr[i] != '+') && (trimmedStr[i] != '-'))
			continue;

		Operation operation;
		operation.setOperator(trimmedStr[i]);

		SmallTempString leftOperandStr(trimmedStr.getPointer(), i);
		if (!parseNumberLoosely(leftOperandStr, operation.leftOperand))
			return false;

		SmallTempString rightOperandStr(trimmedStr.getPointer() + i + 1, length - (i + 1));
		if (!parseNumberLoosely(rightOperandStr, operation.rightOperand))
			return false;

		if (!operation.calculate())
			return false;

		if (operation.result >= lang::NumericLimits<NumberType>::getLowest() && operation.result <= lang::NumericLimits<NumberType>::getMax())
		{
			if (std::is_integral<NumberType>())
			{
				/* Must be rounded rather than floored to avoid unfortunate edge cases */
				resultOut = NumberType(std::round(operation.result));
			}
			else
			{
				resultOut = NumberType(resultOut);
			}
			return true;
		}
	}
	return false;
}


bool parseNumberOperation(const StringRef &str, int8_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, uint8_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, int16_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, uint16_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, int32_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, uint32_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, int64_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, uint64_t &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, float &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool parseNumberOperation(const StringRef &str, double &resultOut)
{
	return parseNumberOperationImpl(str, resultOut);
}


bool validateHexOnly(const StringRef &str)
{
	SizeType length = str.getLength();
	if (length < 3)
		return false;

	if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
		return false;

	for (SizeType i = 2; i < length; i++)
	{
		if (!(str[i] >= '0' && str[i] <= '9') &&
			!(str[i] >= 'a' && str[i] <= 'f') &&
			!(str[i] >= 'A' && str[i] <= 'F'))
		{
			return false;
		}
	}
	return true;
}


template <typename IntType>
bool parseHexImpl(const StringRef &str, IntType &resultOut)
{
	if (!validateHexOnly(str))
	{
		resultOut = IntType(0);
		return false;
	}

	uint64_t resultInternal = 0;
	int got = sscanf(str.getPointer(), "%" FB_FSX64, &resultInternal);
	if (got == 1)
	{

		if (resultInternal <= lang::NumericLimits<IntType>::getMax())
		{
			resultOut = IntType(resultInternal);
			return true;
		}
	}
	resultOut = 0;
	return false;
}


bool parseNumberAsHex(const StringRef &str, uint8_t &resultOut)
{
	return parseHexImpl(str, resultOut);
}


bool parseNumberAsHex(const StringRef &str, uint16_t &resultOut)
{
	return parseHexImpl(str, resultOut);
}


bool parseNumberAsHex(const StringRef &str, uint32_t &resultOut)
{
	return parseHexImpl(str, resultOut);
}


bool parseNumberAsHex(const StringRef &str, uint64_t &resultOut)
{
	return parseHexImpl(str, resultOut);
}


template <typename IntType>
bool parseStringAsIntLooselyImpl(const StringRef &str, IntType &resultOut)
{
	// first try the most usual, expected, case.
	if (parseNumber(str, resultOut))
		return true;

	CacheHeapString<16> tmp(str);
	tmp.trimWhiteSpace();
	if (tmp.isEmpty())
	{
		// an empty string is considered as 0.
		resultOut = 0;
		return true;
	}

	// maybe trimming helped?
	if (parseNumber(tmp, resultOut))
		return true;

	// Try parsing single arithmetic operation
	if (parseNumberOperation(tmp, resultOut))
		return true;

	if (!IsSigned<IntType>::value)
	{
		// try and see if it happens to be a hex value?
		uint64_t resultInternal = 0;
		/* Suppress signed / unsigned mismatch warnings */
		#pragma warning(suppress: 4018 4388)
		if (parseNumberAsHex(tmp, resultInternal) && resultInternal <= lang::NumericLimits<IntType>::getMax())
		{
			resultOut = IntType(resultInternal);
			return true;
		}
	}

	// failure.
	resultOut = 0;
	return false;
}


bool parseNumberLoosely(const StringRef &str, int8_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, uint8_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, int16_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, uint16_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, int32_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, uint32_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, int64_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, uint64_t &resultOut)
{
	return parseStringAsIntLooselyImpl(str, resultOut);
}


bool validateFloatOnly(const StringRef &str)
{
	SizeType length = str.getLength();
	bool exponentFound = false;
	bool dotFound = false;
	for (SizeType i = 0; i < length; i++)
	{
		if (str[i] >= '0' && str[i] <= '9')
			continue;

		// valid cases:
		// -1
		// -1e-1
		// -1E-1
		if (str[i] == '-')
		{
			if (i == 0)
				continue;

			if (str[i - 1] == 'e' || str[i - 1] == 'E')
				continue;

			return false;
		}

		// valid cases:
		// .1
		// 1.1
		// -.1
		// 1.
		if (str[i] == '.')
		{
			if (exponentFound)
				return false;
			if (dotFound)
				return false;
			dotFound = true;
			continue;
		}

		// valid cases:
		// 1e2
		// 1.1e2
		// 1E2
		// 1.1E2
		if (str[i] == 'e' || str[i] == 'E')
		{
			if (i == 0 || i == length - 1)
				return false;
			if (exponentFound)
				return false;
			exponentFound = true;
			continue;
		}

		// can end in f only if there is a dot or exponent
		if (str[i] == 'f' && i == length - 1 && (dotFound || exponentFound))
			continue;

		// unknown character
		return false;
	}
	return true;
}


bool parseNumber(const StringRef &str, float &resultOut)
{
	if (!validateFloatOnly(str))
	{
		resultOut = 0.0f;
		return false;
	}

	float resultInternal = 0.0f;
	int got = sscanf(str.getPointer(), "%f", &resultInternal);
	if (got == 1)
	{
		resultOut = resultInternal;
		return true;
	}

	resultOut = 0.0f;
	return false;
}


bool parseNumber(const StringRef &str, double &resultOut)
{
	if (!validateFloatOnly(str))
	{
		resultOut = 0.0;
		return false;
	}

	double resultInternal = 0.0;
	int got = sscanf(str.getPointer(), "%lf", &resultInternal);
	if (got == 1)
	{
		resultOut = resultInternal;
		return true;
	}

	resultOut = 0.0;
	return false;
}


template <typename NumberType>
bool parseFloatingPointLooselyImpl(const StringRef &str, NumberType &resultOut)
{
	// first try the most usual, expected, case.
	if (parseNumber(str, resultOut))
		return true;

	CacheHeapString<16> tmp(str);
	tmp.trimWhiteSpace();
	if (tmp.isEmpty())
	{
		// an empty string is considered as 0.
		resultOut = NumberType(0);
		return true;
	}

	if (parseNumber(tmp, resultOut))
		return true;

	// failure.
	resultOut = NumberType(0);
	return false;

}


bool parseNumberLoosely(const StringRef &str, float &resultOut)
{
	return parseFloatingPointLooselyImpl(str, resultOut);
}


bool parseNumberLoosely(const StringRef &str, double &resultOut)
{
	return parseFloatingPointLooselyImpl(str, resultOut);
}


FB_END_PACKAGE2()
