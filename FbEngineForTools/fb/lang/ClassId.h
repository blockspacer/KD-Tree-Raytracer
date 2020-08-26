#pragma once

#include "fb/lang/platform/FourCC.h"
#include "fb/lang/Types.h"

FB_PACKAGE0()

struct ClassId
{
	ClassId() : value(0) {}
	ClassId(int32_t value) : value(value) {}
	operator int32_t() const { return value; }
	int32_t value;

	static ClassId getStaticClassId() { return FB_FOURCC('C', 'L', 'I', 'D'); }

	template<class T>
	T toString() const
	{
		const char fourCCString[] = { (char)(value & 0xFF), (char)((value >> 8) & 0xFF), (char)((value >> 16) & 0xFF), (char)((value >> 24) & 0xFF), 0 };
		return T(fourCCString);
	}

	template<class T>
	static ClassId fromString(const T &classIdString)
	{
		const SizeType length = classIdString.getLength();
		int32_t i = 0;
		if (length > 0)
			i |= (int32_t)classIdString[0];
		if (length > 1)
			i |= ((int32_t)classIdString[1]) << 8;
		if (length > 2)
			i |= ((int32_t)classIdString[2]) << 16;
		if (length > 3)
			i |= ((int32_t)classIdString[3]) << 24;
		return ClassId(i);
	}
};

FB_END_PACKAGE0()
