#include "Precompiled.h"
#include "BitmaskTypes.h"

#include "fb/string/HeapString.h"

FB_PACKAGE0()

void ConcatenateBitMaskStringVector::concatenate(HeapString &result, uint64_t value, const Vector<StaticString> &stringVector, SizeType numEntries, const char *bitMaskTypeName)
{
	if (value == 0)
	{
		result << bitMaskTypeName << "(0)";
		return;
	}

	bool first = true;
	for (SizeType i = 0; i < numEntries; ++i)
	{
		if ((value & (1LLU << i)) != 0)
		{
			if (!first)
				result << "|";

			first = false;
			result << stringVector[i];
		}
	}
	return;
}

FB_END_PACKAGE0()
