#pragma once

#include "StringImp.h"
#include "fb/algorithm/traits/CompareTraits.h"
#include "fb/algorithm/Comparator.h"
#include "fb/lang/hash/Hash.h"
#include "fb/string/StringRef.h"
#include "fb/string/IsStringLiteral.h"

FB_DECLARE0(StaticString)

FB_PACKAGE0()

	// Ref counted static strings, which can get deallocated.
	// Thread safe, but with overhead.
	// If constructed from StaticString, thread safeness comes without overhead.

	// If StaticString with same internal string is constructed after SemiStaticString, SemiStatic instances
	// will be forced to be static.

	class DynamicString
	{
	public:
		typedef void ImplementsStringRef;

		static const SizeType maxSupportedLength = string::StringImp::maxSupportedLength;

		DynamicString() { }
		explicit DynamicString(const char *ptr);
		explicit DynamicString(char *ptr);
		DynamicString(const char *ptr, SizeType length);
		DynamicString(const char *ptr, SizeType length, bool createAsStatic);
		DynamicString(const DynamicString &str);
		DynamicString(DynamicString &&str) { imp.moveImp(str.imp); }
		explicit DynamicString(const StringRef &other) : imp(other.getPointer(), other.getLength(), false) { }
		template<typename S, FB_IS_STRING_LITERAL(S)>
		explicit DynamicString(const S &other) : DynamicString(StringRef::stringLiteral(other)) { }
		static DynamicString createFromConstChar(const char *str)
		{
			return DynamicString(str, StringRef::strLen(str));
		}
		static DynamicString createFromAnyString(const StringRef &str)
		{
			return DynamicString(str.getPointer(), str.getLength());
		}

		~DynamicString() { }

		static DynamicString createAsStatic(const char* ptr);
		static DynamicString createAsStatic(const StringRef &other) { return DynamicString(other.getPointer(), other.getLength(), true); }

		FB_FORCEINLINE const char *getPointer() const { return imp.getPointer(); }
		FB_FORCEINLINE SizeType getLength() const { return imp.getLength(); }
		FB_FORCEINLINE bool isEmpty() const { return getLength() == 0; }
		void clear() { *this = empty; }

		bool isStatic() const;
		void convertToStatic() const;

		char operator[](SizeType index) const { fb_expensive_assert(index <= imp.getLength()); return imp.getPointer()[index]; }
		void operator= (const DynamicString &other);
		void operator= (DynamicString &&other) { imp.moveImp(other.imp); }
		void operator= (const char *ptr);
		void operator= (const StringRef &other) { *this = DynamicString(other.getPointer(), other.getLength()); }
		inline bool operator== (const DynamicString &other) const { return imp == other.imp; }
		inline bool operator== (const StringRef &other) const { return operator==(other.getPointer()); }
		bool operator== (const StaticString &other) const;
		bool operator== (const char *other) const;
		inline bool operator!= (const StringRef &other) const { return !(operator==(other)); }
		inline bool operator!= (const DynamicString &other) const { return imp != other.imp; }
		bool operator!= (const char *other) const;
		inline bool operator< (const DynamicString &other) const { return imp < other.imp; }
		bool operator< (const char *other) const;
		inline bool operator> (const DynamicString &other) const { return imp > other.imp; }
		bool operator> (const char *other) const;
		inline bool operator<= (const DynamicString &other) const { return imp <= other.imp; }
		bool operator<= (const char *other) const;
		inline bool operator>= (const DynamicString &other) const { return imp >= other.imp; }
		bool operator>= (const char *other) const;
		operator StringRef() const;

		static const DynamicString empty;
		/* Use this if you need empty string during static initialization */
		static const DynamicString &getEmpty();

		uint32_t getHash() const { return getNumberHashValue((uintptr_t)getPointer()); }

		/* Utils */
		FB_STRING_CASE_DETECTION_DECL();
		FB_STRING_COMPARE_DECL();
		FB_STRING_FIND_DECL();
		FB_STRING_BOOL_PARSE_DECL();
		FB_STRING_NUMBER_PARSE_DECL();

	protected:
		/* Special constructor for creating (static) DynamicString from given special buffer. Use wrapper to separate 
		 * from other constructors */
		struct CreateInPlaceWrapper
		{
			CreateInPlaceWrapper(char *ptr, SizeType rawLength);

			char *ptr;
		};
		DynamicString(CreateInPlaceWrapper &createInPlaceWrapper);

		string::StringImp imp;

		bool operator== (int dontCompareWithNull) const { fb_assert(!"Compare with NULL constant is always inequal. Use isEmpty() instead."); return false; }
		bool operator!= (int dontCompareWithNull) const { fb_assert(!"Compare with NULL constant is always inequal. Use isEmpty() instead."); return true;  }
	};

static inline uint32_t global_default_hash_function(const DynamicString& str) { return str.getHash(); }

FB_END_PACKAGE0()

FB_COMPARABLE(fb::DynamicString);
FB_DEFAULT_COMPARATOR_FOR(fb::DynamicString);
