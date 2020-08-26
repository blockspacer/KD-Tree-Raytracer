#pragma once

#include "fb/lang/FBAssert.h"
#include "fb/lang/Swap.h"
#include "fb/lang/Types.h"

FB_PACKAGE1(string)

/* Unaligned access is only needed with constructed in place strings that may not be aligned at all */
#define FB_PLATFORM_ALLOWS_UNALIGNED_ACCESS FB_TRUE

#define FB_STRING_IMP_USE_ATOMICS FB_FALSE

// Yeah, leaks implementation details but allows inlining

// Buffer format
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
// [ID][RefCount][StringLength][StaticFlag][Actual string]
#else
// [RefCount][StringLength][StaticFlag][Actual string]
#endif

struct StringImpHelper
{
	// Reserved extra space
#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
	static const SizeType IdBytes = 4;
#else
	static const SizeType IdBytes = 0;
#endif

	static const SizeType RefCountBytes = 4;
	static const SizeType StringLengthBytes = 2;
	static const SizeType StaticFlagBytes = 1;
	// Overall overhead
	static const SizeType ExtraOverhead = 1; // Nul
	static const SizeType OverheadBytes = IdBytes + RefCountBytes + StringLengthBytes + StaticFlagBytes + ExtraOverhead;
	static const SizeType ShiftBytes = OverheadBytes - ExtraOverhead;
	// Offsets from string data
	static const SizeType StaticFlagOffset = StaticFlagBytes;
	static const SizeType StringLengthOffset = StaticFlagOffset + StringLengthBytes;
	static const SizeType RefcountOffset = StaticFlagOffset + StringLengthBytes + RefCountBytes;
	static const SizeType IdOffset = StaticFlagOffset + StringLengthBytes + RefCountBytes + IdBytes;

	static const SizeType refcountStaticConversionBoundary = 30000;
	static const SizeType maxSupportedLength = (1U << (StringLengthBytes * 8 - 1)) - 1;

	/* Helpers */
	static SizeType getLength(const char *pointer)
	{
#if FB_PLATFORM_ALLOWS_UNALIGNED_ACCESS == FB_TRUE
		const uint16_t *lengthPtr = reinterpret_cast<const uint16_t*>(pointer - StringLengthOffset);
		return *lengthPtr;
#else
		const uint8_t *lengthPtr = reinterpret_cast<const uint8_t*>(pointer - StringLengthOffset);
		SizeType length = SizeType((*lengthPtr) << 8) + (*(lengthPtr + 1));
		return length;
#endif
	}

	static void setLength(char *pointer, SizeType length)
	{
		fb_assert(length < maxSupportedLength);
#if FB_PLATFORM_ALLOWS_UNALIGNED_ACCESS == FB_TRUE
		uint16_t *lengthPtr = reinterpret_cast<uint16_t*>(pointer - StringLengthOffset);
		*lengthPtr = uint16_t(length);
#else
		uint8_t *lengthPtr = reinterpret_cast<uint8_t*>(pointer - StringLengthOffset);
		(*lengthPtr) = uint8_t(length >> 8);
		(*(lengthPtr + 1)) = uint8_t(length & 0xFF);
#endif
	}


	static bool isStatic(const char *pointer)
	{
		char staticFlag = *(pointer - StaticFlagOffset);
		if ((staticFlag & 0x1) == 1)
			return true;

		return false;
	}

	static bool isInPlace(const char *pointer)
	{
		char staticFlag = *(pointer - StaticFlagOffset);
		/* In place constructed strings are also always static */
		if (staticFlag == 3)
			return true;

		return false;
	}

	static uint32_t getId(const char *pointer)
	{
		#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			const uint32_t *id = reinterpret_cast<const uint32_t *> (pointer - IdOffset);
			return *id;
		#else
			return 0;
		#endif
	}


	static void setId(char *pointer, uint32_t id)
	{
		#if FB_STRING_IMP_USE_ATOMICS == FB_TRUE
			uint32_t *idPtr = reinterpret_cast<uint32_t *> (pointer - IdOffset);
			*idPtr = id;
		#endif
	}

	static bool setStatic(char *pointer, bool newStatic)
	{
		fb_assert(!isInPlace(pointer) && "Trying to set static flag for in place constructed string");
		char *staticFlag = pointer - StaticFlagOffset;
		*staticFlag = newStatic ? 1 : 0;

		return false;
	}

	static void setReferenceCount(char *pointer, int32_t newCount)
	{
		fb_assert(newCount >= 0);
		int32_t *refCount = reinterpret_cast<int32_t *> (pointer - RefcountOffset);
		*refCount = newCount;
	}

#if FB_ENGINE_FOR_TOOLS == FB_TRUE
	static void initializeData();
#endif
};

class StringImp
{
public:
	StringImp() : pointer(getEmptyStringPointer()) { }
	StringImp(const char *ptr, SizeType length, bool isStatic);
	/* Create in place constructor */
	StringImp(char *ptr);
	StringImp(const StringImp &other);

	~StringImp()
	{
		if (!isStatic())
			reset();
	}

	const char *getPointer() const { return pointer; }
	SizeType getLength() const { return StringImpHelper::getLength(pointer); }
	bool isStatic() const { return StringImpHelper::isStatic(pointer); }

	void unsafeCopy(const StringImp &other);
	void moveImp(StringImp &other)
	{
		lang::swap(pointer, other.pointer);
	}

	void operator= (const StringImp &other);
	inline bool operator== (const StringImp &other) const { return pointer == other.pointer; }
	inline bool operator!= (const StringImp &other) const { return pointer != other.pointer; }
	bool operator< (const StringImp &other) const;
	bool operator> (const StringImp &other) const;
	bool operator<= (const StringImp &other) const;
	bool operator>= (const StringImp &other) const;

	static const SizeType maxSupportedLength = StringImpHelper::maxSupportedLength;

	static char *getEmptyStringData()
	{
		return emptyStringData;
	}
private:
	void create(const char *ptr, SizeType length, bool isStatic, char *isInPlace);
	void copyFrom(const StringImp &other);
	void reset();

	char *pointer;

	static char emptyStringData[StringImpHelper::ShiftBytes + 1];
	static char *getEmptyStringPointer()
	{
		return emptyStringData + StringImpHelper::ShiftBytes;
	}
};

FB_END_PACKAGE1()
