#pragma once

#include "fb/lang/Types.h"
#include "fb/string/Common.h"

#if FB_BUILD == FB_DEBUG
#define FB_STRING_REF_VALIDATION_ENABLED FB_TRUE
#else
#define FB_STRING_REF_VALIDATION_ENABLED FB_FALSE
#endif

FB_PACKAGE0()

/**
 * This class wraps any string type so that they can be used by the utilities.
 * (Without having to make duplicate function calls for different types of interfaces.)
 *
 * NOTICE, this class does NOT create a copy of the given string parameter!!!
 * Nor does it take the ownership of the string. You'll still have to manage the string
 * deletion yourself. You are not supposed to use this as a general purpose string container
 * class - it will not work as such.
 *
 * ALSO NOTICE, this is not a thread safe class (or even safe within the thread) if you
 * intend to modify the pointer you have given as a parameter.
 *
 */

class StringRef
{
	//
	//               == About this implementation ==
	// Excessive amount of template metaprogramming used in this class
	// It's stupid and ugly but has a purpose
	//
	// This SFINAE:
	//     template<class StringT, typename Valid = typename StringT::ImplementsStringRef>
	//     FB_FORCEINLINE StringRef(const StringT &other)
	// prevents c-strings, string-literals and char-buffers from picking this overload,
	// which would lead to a confusing and verbose template error ("... must be an object
	// ... doesn't have .getPointer() ..." etc.) With this a normal error is produced.
	// This WOULD use DynamicString and HeapString directly instead of templates but
	// doing so causes a dependency loop which would prevent either inlining or
	// compilation.
	//
	// This:
	//     template<size_t N>
	//     FB_FORCEINLINE StringRef(const char(&other)[N]);
	// extracts string length from a string-literal at compile-time. It asserts that a
	// string doesn't have a length differing from the length of the buffer.
	//
	// This monstrosity (along with the lines above it):
	//    template< typename CString
	//            , typename SFINAE_HACK = typename is_either<CString, const char*, char*>::yes >
	//    explicit inline StringRef(CString other);
	// makes sure that only const and non-const c-strings are accepted by this particular
	// overload. It has to be a template function to prevent string-literals from
	// preferring it instead of the above compile-time constant overload. This does a
	// strlen meaning it has runtime overhead which is why it's marked explicit.
	//
	// This:
	//     template<size_t N>
	//     explicit FB_FORCEINLINE StringRef(char(&other)[N]);
	// prevents mutable string-buffers from being handled by the overload meant for
	// string-literals. It doesn't use the template parameter N for anything relevant.
	// It just checks that the string length isn't invalid. This too has overhead and
	// is thus marked explicit.
	//
	// FB_FORCEINLINE is used to make stepping code less painful in debug mode
	//
	//   - Riku R. 2018-04-18
	//

public:
	FB_FORCEINLINE StringRef(const StringRef &other);
	inline StringRef(const char *ptr, SizeType length);

	// Template hacks to prevent confusing error messages
	template<class StringT, typename Valid = typename StringT::ImplementsStringRef>
	FB_FORCEINLINE StringRef(const StringT &other);

	template<size_t N>
	FB_FORCEINLINE StringRef(const char(&other)[N]);

	// Template hacks to separate "(const) char *" from invalid, non-string types
	template<class A, class B, class C>
	struct is_either { };
	template<class A, class B>
	struct is_either<A, A, B> { typedef void yes; };
	template<class A, class B>
	struct is_either<A, B, A> { typedef void yes; };

	// This calls strlen() meaning it has overhead and is hence explicit
	// "(const) char *" overload must be templated so that "(const) char[]" can be preferred by
	// the overloads meant for string-literals.
	// C++ is dumb
	template< typename CString, typename SFINAE_HACK = typename is_either<CString, const char*, char*>::yes >
	explicit inline StringRef(CString other);

	// Template hacks to separate "char[]" (aka. string buffer) from "const char[]" (typically string-literal)
	template<size_t N>
	explicit FB_FORCEINLINE StringRef(char(&other)[N]);

	inline static StringRef make(const char *ptr);

	inline const char *getPointer() const;

	inline char operator[] (SizeType index) const;

	inline SizeType getLength() const;

	inline bool isEmpty() const;

	bool operator== (const StringRef &other) const;
	bool operator== (const char *other) const;
	bool operator!= (const StringRef &other) const;
	bool operator!= (const char *other) const;
	bool operator< (const StringRef &other) const;
	bool operator< (const char *other) const;

#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	inline ~StringRef();
#endif
	template<size_t N>
	static StringRef stringLiteral(const char(&arr)[N]);
	template<size_t N>
	static StringRef stringLiteral(char(&arr)[N]);

	static StringRef empty;

	/* Returns length of given C string. Assumes it fits to SizeType (asserts if not) */
	static SizeType strLen(const char *str);

	/* Utils */
	FB_STRING_CASE_DETECTION_DECL();
	FB_STRING_COMPARE_DECL();
	FB_STRING_FIND_DECL();
	FB_STRING_BOOL_PARSE_DECL();
	FB_STRING_NUMBER_PARSE_DECL();

private:
	void operator==(int64_t) {}
	void operator!=(int64_t) {}
	void operator==(uint64_t) {}
	void operator!=(uint64_t) {}
	void operator==(nullptr_t) {}
	void operator!=(nullptr_t) {}
	void operator<(int64_t) {}
	void operator>(int64_t) {}
	void operator<(uint64_t) {}
	void operator>(uint64_t) {}
	void operator<(nullptr_t) {}
	void operator>(nullptr_t) {}
	void operator<=(int64_t) {}
	void operator>=(int64_t) {}
	void operator<=(uint64_t) {}
	void operator>=(uint64_t) {}
	void operator<=(nullptr_t) {}
	void operator>=(nullptr_t) {}

	FB_FORCEINLINE void debugStoreHash();

#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	static FB_FORCEINLINE SizeType debugCalculateHash(const char *ptr, SizeType length);
#endif

	const char *ptr;
	SizeType length;

#if FB_STRING_REF_VALIDATION_ENABLED == FB_TRUE
	uint32_t hash;
#endif
};

FB_END_PACKAGE0()

#ifndef FB_STRINGREF_INLINE_ONCE
#define FB_STRINGREF_INLINE_ONCE
#include "StringRefInline.h"
#endif
