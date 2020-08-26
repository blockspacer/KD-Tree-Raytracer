

#ifndef FB_LANG_BITMASKTYPESINLINE_INCLUDE_ALLOWED
#error "BitmaskTypesInline.h is intended to be included only by BitmaskTypes.h"
#endif

#pragma region ImplExplicitTypedefMacros

#if (FB_EXPLICIT_TYPEDEF_ENABLED == FB_TRUE)

#define FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit) fb_assert((t & p_uninit_bit) == 0 && "flag with an uninitialized value is being used.")

#define FB_EXPLICIT_BITMASK_TYPEDEF_IMPL(p_implementing_type, p_typename, p_uninit_bit) \
	class p_typename##Mask \
	{ \
		/* Use enum type here so the debugger can show bit mask as | separated names, even though it makes implementing things a bit harder */ \
		p_typename t; \
	public: \
		explicit p_typename##Mask(const p_implementing_type t) \
			: t(p_typename(t)) \
		{ \
		} \
		p_typename##Mask() \
			: t(p_typename(p_uninit_bit)) \
		{ \
		} \
		p_typename##Mask(const p_typename & t) \
		: t(p_typename(t)) \
		{ \
		} \
		operator p_implementing_type () const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return t; \
		} \
		operator p_implementing_type& () \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return reinterpret_cast<p_implementing_type&>(t); \
		} \
		bool operator==(const p_typename &bit) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return t == p_implementing_type(bit); \
		} \
		bool operator==(const p_typename##Mask &rhs) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return t == rhs.t; \
		} \
		p_typename operator& (const p_typename &bit) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename(t & p_implementing_type(bit)); \
		} \
		p_typename##Mask operator& (const p_typename##Mask &rhs) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask(t & rhs.t); \
		} \
		p_typename##Mask &operator&= (const p_typename &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t & rhs); \
			return *this; \
		} \
		p_typename##Mask &operator&= (const p_typename##Mask &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t & rhs.t); \
			return *this; \
		} \
		p_typename##Mask operator| (const p_typename &bit) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask(t | p_implementing_type(bit)); \
		} \
		p_typename##Mask operator| (const p_typename##Mask & rhs) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask(t | rhs.t); \
		} \
		p_typename##Mask &operator|= (const p_typename &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t | rhs); \
			return *this; \
		} \
		p_typename##Mask &operator|= (const p_typename##Mask &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t | rhs.t); \
			return *this; \
		} \
		p_typename##Mask operator^ (const p_typename &bit) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask(t ^ p_implementing_type(bit)); \
		} \
		p_typename##Mask operator^ (const p_typename##Mask & rhs) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask(t ^ rhs.t); \
		} \
		p_typename##Mask &operator^= (const p_typename &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t ^ rhs); \
			return *this; \
		} \
		p_typename##Mask &operator^= (const p_typename##Mask &rhs) \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			t = p_typename(t ^ rhs.t); \
			return *this; \
		} \
		p_typename##Mask operator~ () const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return p_typename##Mask((~t) & (~p_uninit_bit)); \
		} \
		void clear() \
		{ \
			t = p_typename(0); \
		} \
		bool isAnySet(p_implementing_type mask) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return (t & mask) != 0; \
		} \
		bool areAllSet(p_implementing_type mask) const \
		{ \
			FB_IMPL_UNINIT_FLAG_CHECK(p_uninit_bit); \
			return (t & mask) == mask; \
		} \
		HeapString &appendToString(HeapString &result) const \
		{ \
			ConcatenateBitMaskStringVector::concatenate(result, t, p_typename##Descriptor::getStringVector(), p_typename##Descriptor::numEntries, #p_typename); \
			return result; \
		} \
		fb_static_assert(p_typename##Descriptor::numEntries <= 31); /* reserve last bit for p_uninit_bit */ \
		fb_static_assert(sizeof(p_typename) == sizeof(p_implementing_type) && "p_implementing type is supposed to be enum sized"); \
	}; \

#define FB_FLAG_COMPARE_TRAITS(p_namespace_and_class) \
	FB_PACKAGE2(algorithm, traits) \
	template<> class CompareTraits<p_namespace_and_class> \
	{ \
	public: \
		static const bool doesSupportCompare = true; \
		static const bool doesSupportSortingCompare = false; \
		static const bool hasSpecializedCompare = false; \
		static const bool hasSpecializedSortingCompare = false; \
	}; \
	FB_END_PACKAGE2(); \

#else

#define FB_FLAG_COMPARE_TRAITS(p_namespace_and_class)
#define FB_EXPLICIT_BITMASK_TYPEDEF_IMPL(p_implementing_type, p_typename, p_uninit_bit) typedef p_implementing_type p_typename##Mask;

#endif

#pragma endregion


FB_PACKAGE0()

#pragma region ImplementingMaskTypes

// these could be done with template classes, but it seems more sensible to allow them to be just simple typedefs to integers.
// (that should ensure maximal optimization in the final release build)

// the simple, single bit true/false bitmasks
//FB_EXPLICIT_TYPEDEF(uint64_t, Flag64Mask);
//FB_EXPLICIT_TYPEDEF(uint32_t, Flag32Mask);
//FB_EXPLICIT_TYPEDEF(uint16_t, Flag16Mask);
//FB_EXPLICIT_TYPEDEF(uint8_t, Flag8Mask);
typedef uint64_t Flag64Mask;
typedef uint32_t Flag32Mask;
typedef uint16_t Flag16Mask;
typedef uint8_t Flag8Mask;

// custom bitmasks, with any number of bits per single value, not just true/false
//FB_EXPLICIT_TYPEDEF(uint64_t, Bitmask64);
//FB_EXPLICIT_TYPEDEF(uint32_t, Bitmask32);
//FB_EXPLICIT_TYPEDEF(uint16_t, Bitmask16);
//FB_EXPLICIT_TYPEDEF(uint8_t, Bitmask8);

// use of specific number of integer
//template<typename T, int N> class TemplatedBitmask

#pragma endregion


#pragma region ImplementingHelperMacros

class FlagDefBaseClassDescriptor
{
public:
	static const SizeType numEntries = 0;
	typedef StaticVector<StaticString, 31> StringVector;
	static const StringVector &getStringVector()
	{
		static StringVector vector;
		return vector;
	}
};

// the flags will now always use the 32 bit int (with the highest bit reserved for uninit check)
#define FB_IMPL_FLAG_BITCOUNT 32
#define FB_IMPL_FLAG_TYPE fb::Flag32Mask

#define FB_FLAG_DEF_IMPL(p_typename, p_extendtype, p_newentries, p_enums, p_strings) \
	enum p_typename : uint32_t \
	{ \
		p_typename##None, \
		FB_PP_UNPACK(p_enums) \
	}; \
	class p_typename##Descriptor \
	{ \
	public: \
		typedef p_extendtype##Descriptor::StringVector StringVector; \
		static const SizeType numEntries = p_extendtype##Descriptor::numEntries + p_newentries; \
	protected: \
		static StringVector getStringVectorImpl() \
		{ \
			StringVector strings; \
			/* First all inherited bits */ \
			for (const StaticString &str : p_extendtype##Descriptor::getStringVector()) \
			{ \
				strings.pushBack(str); \
			} \
			/* New bits */ \
			const char * newStrings[p_newentries] = { FB_PP_UNPACK(p_strings) }; \
			for (SizeType i = 0; i < p_newentries; ++i) \
			{ \
				strings.pushBack(StaticString(newStrings[i])); \
			} \
			return strings; \
		} \
	public: \
		static const StringVector &getStringVector() \
		{ \
			static const StringVector strings = getStringVectorImpl(); \
			return strings; \
		} \
		static const StaticString &getBitName(p_typename flag) \
		{ \
			for (SizeType i = 0; i < numEntries; ++i) \
			{ \
				if (flag == (1U << i)) \
				{ \
					return getStringVector()[i]; \
				} \
			} \
			fb_assertf(false, "Could not find bit with value %d from " #p_typename, flag); \
			return StaticString::empty; \
		} \
	}; \
	FB_EXPLICIT_BITMASK_TYPEDEF_IMPL(FB_IMPL_FLAG_TYPE, p_typename, (1 << (FB_IMPL_FLAG_BITCOUNT - 1))) \

#define FB_FLAG_DEF_ENUM_IMPL_OPER_DONE(p_count, p_index, p_typename, p_extendtype, p_input, p_output) FB_PP_UNPACK(FB_PP_POP_FIRST(p_output))
#define FB_FLAG_DEF_ENUM_IMPL_OPER_LOOP(p_count, p_index, p_typename, p_extendtype, p_input, p_output) p_count, p_index, p_typename, p_extendtype, p_input, p_output
#define FB_FLAG_DEF_ENUM_IMPL_OPER(p_count, p_index, p_typename, p_extendtype, p_input, p_output) \
	FB_PP_CONCAT(FB_FLAG_DEF_ENUM_IMPL_OPER_, FB_PP_LOOP_STATE(p_count)) (FB_PP_DECREASE(p_count), FB_PP_INCREASE(p_index), p_typename, p_extendtype, FB_PP_POP_FIRST(p_input), \
	(FB_PP_UNPACK(p_output), FB_PP_CONCAT(p_typename, FB_PP_FIRST(p_input)) = 1 << (p_extendtype##Descriptor::numEntries + p_index) ))

#define FB_FLAG_DEF_ENUM_IMPL_3(p_typename, p_extendtype, p_args) \
	FB_PP_FOR(FB_PP_NARG p_args, FB_FLAG_DEF_ENUM_IMPL_OPER, (FB_PP_NARG p_args, 0, p_typename, p_extendtype, (FB_PP_UNPACK(p_args), input), (output)))

#define FB_FLAG_DEF_ENUM_IMPL_2(p_typename, p_extendtype, p_args) FB_FLAG_DEF_ENUM_IMPL_3(p_typename, p_extendtype, p_args)
#define FB_FLAG_DEF_ENUM_IMPL_1(p_typename, p_extendtype, p_args) FB_FLAG_DEF_ENUM_IMPL_2(p_typename, p_extendtype, p_args)
#define FB_FLAG_DEF_ENUM_IMPL(p_typename, p_extendtype, ...) \
	FB_FLAG_DEF_ENUM_IMPL_1(p_typename, p_extendtype, (__VA_ARGS__))

#pragma endregion

// --- the actual macros you are supposed to use :) ---

#define FB_FLAG_DEF_EXTEND(p_typename, p_extendtype, ...) \
	FB_FLAG_DEF_IMPL(p_typename, p_extendtype, FB_PP_NARG(__VA_ARGS__), (FB_FLAG_DEF_ENUM_IMPL(p_typename, p_extendtype, __VA_ARGS__)), (FB_PP_FOREACH(FB_PP_TOSTR, __VA_ARGS__)))

#define FB_FLAG_DEF(p_typename, ...) FB_FLAG_DEF_EXTEND(p_typename, FlagDefBaseClass, __VA_ARGS__)

FB_END_PACKAGE0()


