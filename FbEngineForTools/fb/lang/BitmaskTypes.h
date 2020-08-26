#pragma once

// (these use int types internally, so it is required)
#include "IntTypes.h"
#include "Config.h"
#include "FBExplicitTypedef.h"
#include "FBStaticAssert.h"

#include "fb/container/Vector.h"
#include "fb/string/StaticString.h"

FB_DECLARE0(HeapString)

FB_PACKAGE0()

// there are macros such as:

// FB_FLAG_DEF(p_typename, p_e1)
// FB_FLAG_DEF(p_typename, p_e1, p_e2)
// ...

// which will create you types for handling bitmasks.
// the macro will expand into following kind of (explicit) datatypes:
//
// - enum p_typename  (containing all the entries, each representing a single flag bit)
// - int p_typename##Mask  (a datatype that can represent all of the flag bits - it an be manipulated with OR/AND operations)
// - class p_typename##Descriptor  (a class that has some static methods for getting a list of the enums as strings and such)
//
// NOTICE: these generated data types cannot be used for RTTI nor for template specialization purposes, because they
// may be directly typedeffed to primitive int, etc. types in certain final build types - and therefore, they will lose 
// the explicit (aka strong) type qualities - you won't be able to identify different types of flags or int types from each other.


// for example:
//
// FB_FLAG_DEF(MyFlagType
//   , MyFirstBit
//   , MySecondBit
//   , MyThirdBit
// );
//
// MyFlagType flag = MyFlagType::MyFlagTypeMyFirstBit;
// MyFlagTypeMask theMask = MyFlagType::MyFlagTypeMyFirstBit | MyFlagType::MyFlagTypeMySecondBit;
// MyFlagTypeMask anotherMask = MyFlagType::MyFlagTypeMyThirdBit;
//
// MyFlagTypeMask someFlagValue = MyFlagTypeMask(0xffff);
// fb_assert((someFlagValue & theMask & anotherMask) == 0); // this must be zero, since the mask bits don't overlap
// fb_assert(((someFlagValue & ~theMask) & theMask) == 0);
// fb_assert((~theMask & anotherMask) == MyFlagType::MyFlagTypeMyThirdBit);
// fb_assert(((~theMask | ~anotherMask) & (theMask | anotherMask)) == 0);
//
// fb_assert(MyFlagTypeDescriptor:getStringsAmount() == 3);
// fb_assert(strcmp(MyFlagTypeDescriptor:getStrings()[0], "MyFirstBit") == 0);
//

// use those to create new bitmask types.
// (if you really must see the messy implementing macros, go see the "BitmaskTypesInline.h")

struct ConcatenateBitMaskStringVector
{
	static void concatenate(HeapString &result, uint64_t value, const Vector<StaticString> &stringVector, SizeType numEntries, const char *bitMaskTypeName);
};

FB_END_PACKAGE0()

#define FB_LANG_BITMASKTYPESINLINE_INCLUDE_ALLOWED
#include "BitmaskTypesInline.h"
#undef FB_LANG_BITMASKTYPESINLINE_INCLUDE_ALLOWED
