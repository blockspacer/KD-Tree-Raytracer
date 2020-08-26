#include "Precompiled.h"
#include "StringLiteral.h"

FB_PACKAGE1(string)

void StringLiteral::checkLength() const
{
	if (FB_ASSERT_ENABLED != FB_TRUE)
		return;
	
	fb_assert(ptr);
	fb_assert(length < 100000 && "String-literal over 100000 characters long. Really?");
	fb_assert(ptr[length] == 0 && "Invalid string-literal. Not nul-terminated.");
	for (SizeType i = 0; i < length; ++i)
	{
		fb_assert(ptr[i] != 0 && "Extra nuls found in a string-literal.");
	}
}

void testStringLiteral()
{
	// Ok!
	StringLiteral s0("asdf");

	// Runtime error
	//const char trollString[128] = {};
	//StringLiteral s1(trollString);

	// Compile error
	//char trollString2[128] = {};
	//StringLiteral s2(trollString2);

	// Runtime error
	//StringLiteral s3("asdf\0asdf\0asdf");

	// Compile error
	//const char *trollString3 = "asdf";
	//StringLiteral s4(trollString3);
}

FB_END_PACKAGE1()
