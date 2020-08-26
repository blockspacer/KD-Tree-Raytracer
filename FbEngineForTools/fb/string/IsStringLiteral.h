#pragma once

FB_PACKAGE1(string)

struct IsStringLiteral
{
	template<typename S, typename X = void>
	struct is_string_literal {};

	template<bool B, class T = void>
	struct enable_if {};

	template<class T>
	struct enable_if<true, T> { typedef T type; };

	template<class T, class U>
	struct is_same { static const bool value = false; };

	template<class T>
	struct is_same<T, T> { static const bool value = true; };

	template<typename S, unsigned N>
	struct is_string_literal<S[N], typename enable_if<is_same<S, char>::value >::type > { };

	template<typename S, unsigned N>
	struct is_string_literal<S[N], typename enable_if<is_same<S, const char>::value >::type > { typedef void yes; };
};


#define FB_IS_STRING_LITERAL(p_S) typename Valid = typename fb::string::IsStringLiteral::is_string_literal<p_S>::yes

FB_END_PACKAGE1()
