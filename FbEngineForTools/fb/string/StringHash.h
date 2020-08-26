#pragma once

#include "fb/lang/hash/Hash.h"
#include "DynamicString.h"
#include "fb/lang/Cstrlen.h"

FB_PACKAGE1(string)

struct StringHashFunctor
{
	// Equal dynamic/static strings have matching pointers
	uint32_t operator () (const DynamicString &str) const
	{
		return getNumberHashValue((uintptr_t) str.getPointer());
	}

	// C-style string
	uint32_t operator () (const char *str) const 
	{
		return getHashValue(str, (uint32_t) cstrlen(str));
	} 

	// Custom string class (HeapString variants, mainly)
	uint32_t operator () (const StringRef &str) const
	{
		return getHashValue(str.getPointer(), (uint32_t) str.getLength());
	} 
};

FB_END_PACKAGE1()
