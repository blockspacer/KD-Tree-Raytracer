#pragma once

FB_DECLARE0(StringRef)

FB_PACKAGE2(string, detail)

/**
 * Note:
 * 
 * As definitions here change, they should be updated to common.h too.
 *
 * "parseStringAs..."  
 *   are the normal string to number conversions that quite closely match the C language number parsing
 *   some minor formatting variations may be accepted, but nothing that could be ambiguously interpreted as different value 
 *
 * "parseStringAs...Loosely"  
 *   are human-readable type of number parsing, with whitespace removals, and generally accepting small formatting errors,
 *   trying to interpret it as intended number type. (may accept a different number format and convert it to requested one)
 *
 * "parseStringAs...Strictly"  
 *   assume a very specific format - possibly more strictly that C language, for example.
 *
 * All boolean parsing functions set resultOut to false, if parsing fails
 * All number parsing functions set resultOut to zero, if parsing fails
 */

/* Accepts "true", "false" (case insensitively) */
bool parseBool(const StringRef &str, bool &resultOut);
/* Accepts "true", "false" (case insensitively) as well as "0" as false and any non-zero integer as true. An empty
 * string is also accepted as false. Will ignore whitespaces. */
bool parseBoolLoosely(const StringRef &str, bool &resultOut);
/* Accepts only "true" or "false" */
bool parseBoolStrictly(const StringRef &str, bool &resultOut);

bool parseNumber(const StringRef &str, int8_t &resultOut);
bool parseNumber(const StringRef &str, uint8_t &resultOut);
bool parseNumber(const StringRef &str, int16_t &resultOut);
bool parseNumber(const StringRef &str, uint16_t &resultOut);
bool parseNumber(const StringRef &str, int32_t &resultOut);
bool parseNumber(const StringRef &str, uint32_t &resultOut);
bool parseNumber(const StringRef &str, int64_t &resultOut);
bool parseNumber(const StringRef &str, uint64_t &resultOut);

/* Accept simple arithmetic operations. Everything is parsed and operated as double and finally converted to type of 
 * resultOut. If result is outside bounds, false will be returned. Input is handled loosely, meaning there may be 
 * extra whitespace. Note that parsing is pretty simple, so don't try anything too fancy. It will fail e.g. on 
 * scientific notation and doesn't understand parenthesis (consider using Lua for serious parsing) */
bool parseNumberOperation(const StringRef &str, int8_t &resultOut);
bool parseNumberOperation(const StringRef &str, uint8_t &resultOut);
bool parseNumberOperation(const StringRef &str, int16_t &resultOut);
bool parseNumberOperation(const StringRef &str, uint16_t &resultOut);
bool parseNumberOperation(const StringRef &str, int32_t &resultOut);
bool parseNumberOperation(const StringRef &str, uint32_t &resultOut);
bool parseNumberOperation(const StringRef &str, int64_t &resultOut);
bool parseNumberOperation(const StringRef &str, uint64_t &resultOut);
bool parseNumberOperation(const StringRef &str, float &resultOut);
bool parseNumberOperation(const StringRef &str, double &resultOut);

/* Requires the string to start with 0x */
bool parseNumberAsHex(const StringRef &strHex, uint8_t &resultOut);
bool parseNumberAsHex(const StringRef &strHex, uint16_t &resultOut);
bool parseNumberAsHex(const StringRef &strHex, uint32_t &resultOut);
bool parseNumberAsHex(const StringRef &strHex, uint64_t &resultOut);

/* Accepts base 16 or 10 integers (base 16 must start with 0x or 0X) and an empty string as zero. Will ignore whitespaces.
 * Accepts simple arithmetic operations */
bool parseNumberLoosely(const StringRef &str, int8_t &resultOut);
bool parseNumberLoosely(const StringRef &str, uint8_t &resultOut);
bool parseNumberLoosely(const StringRef &str, int16_t &resultOut);
bool parseNumberLoosely(const StringRef &str, uint16_t &resultOut);
bool parseNumberLoosely(const StringRef &str, int32_t &resultOut);
bool parseNumberLoosely(const StringRef &str, uint32_t &resultOut);
bool parseNumberLoosely(const StringRef &str, int64_t &resultOut);
bool parseNumberLoosely(const StringRef &str, uint64_t &resultOut);

bool parseNumber(const StringRef &str, float &resultOut);
bool parseNumber(const StringRef &str, double &resultOut);

/* Ignores whitespaces, accepts empty string as zero */
bool parseNumberLoosely(const StringRef &str, float &resultOut);
bool parseNumberLoosely(const StringRef &str, double &resultOut);


FB_END_PACKAGE2()
