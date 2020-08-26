#pragma once

#include "fb/lang/GlobalFixedAllocateFunctions.h"
#include "fb/lang/IsConvertible.h"
#include "fb/lang/IsSame.h"
#include "fb/lang/IsTriviallyCopyable.h"
#include "fb/lang/MemTools.h"
#include "fb/lang/Cstrlen.h"
#include "fb/container/PodVector.h"
#include "fb/container/Pair.h"
#include "fb/container/ArraySlice.h"

#pragma warning(push)
/* C4548: expression before comma has no effect; expected expression with side-effect */
#pragma warning(disable: 4548)
#include <tuple>
#pragma warning(pop)

FB_PACKAGE2(container, database);

enum Column_Type
{
	Column_Type_U64,
	Column_Type_U32,
	Column_Type_U16,
	Column_Type_U8,
	Column_Type_I64,
	Column_Type_I32,
	Column_Type_I16,
	Column_Type_I8,
	Column_Type_String,
	Column_Type_Float,
	Column_Type_Double,
	Column_Type_Char,
	Column_Type_Bool,
};

typedef uint32_t StringIndexType;

template<typename T> static Column_Type getColumnType();

template<> Column_Type getColumnType<uint64_t>() { return Column_Type_U64; }
template<> Column_Type getColumnType<uint32_t>() { return Column_Type_U32; }
template<> Column_Type getColumnType<uint16_t>() { return Column_Type_U16; }
template<> Column_Type getColumnType<uint8_t>() { return Column_Type_U8; }
template<> Column_Type getColumnType<int64_t>() { return Column_Type_I64; }
template<> Column_Type getColumnType<int32_t>() { return Column_Type_I32; }
template<> Column_Type getColumnType<int16_t>() { return Column_Type_I16; }
template<> Column_Type getColumnType<int8_t>() { return Column_Type_I8; }
template<> Column_Type getColumnType<float>() { return Column_Type_Float; }
template<> Column_Type getColumnType<double>() { return Column_Type_Double; }
template<> Column_Type getColumnType<char>() { return Column_Type_Char; }
template<> Column_Type getColumnType<bool>() { return Column_Type_Bool; }
template<> Column_Type getColumnType<const char*>() { return Column_Type_String; }
//template<> Column_Type getColumnType<AnyString>() { return Column_Type_String; }
//template<> Column_Type getColumnType<DynamicString>() { fb_assertf(false, "Use AnyString when passing values to database. This avoids unnecessary passing by value."); return Column_Type_String; }
//template<> Column_Type getColumnType<HeapString>() { fb_assertf(false, "Use AnyString when passing values to database. This avoids unnecessary passing by value."); return Column_Type_String; }

#if FB_COMPILER == FB_CLANG
//template<typename T> Column_Type getColumnType() { fb_assertf(false, "Unknown column type."); return Column_Type_U64; }
#elif FB_COMPILER == FB_GNUC
//template<typename T> Column_Type getColumnType() { fb_static_assertf((false), "Unknown column type."); return Column_Type_U64; }
#else
template<typename T> Column_Type getColumnType() { fb_static_assertf((false), "Unknown column type."); return Column_Type_U64; }
#endif

template<typename T> static Column_Type getColumnType(T val) { return getColumnType<T>(); }

static FB_FORCEINLINE SizeType getColumnTypeSize(Column_Type t)
{
	switch (t)
	{
	case Column_Type_U64: return 64;
	case Column_Type_U32: return 32;
	case Column_Type_U16: return 16;
	case Column_Type_U8: return 8;
	case Column_Type_I64: return 64;
	case Column_Type_I32: return 32;
	case Column_Type_I16: return 16;
	case Column_Type_I8: return 8;
	case Column_Type_Float: return 32;
	case Column_Type_Double: return 64;
	case Column_Type_Char: return 8;
	case Column_Type_Bool: return 1;
	case Column_Type_String: return sizeof(StringIndexType) * 8;
	default:
		fb_assertf(false, "Invalid column type: %d", t);
		return 64;
	};
}
template<typename T> static SizeType getColumnTypeSize() { return getColumnTypeSize(getColumnType<T>()); }

static const char* getColumnTypeString(Column_Type t)
{
	switch (t)
	{
	case Column_Type_U64   : return "uint64_t";
	case Column_Type_U32   : return "uint32_t";
	case Column_Type_U16   : return "uint16_t";
	case Column_Type_U8    : return "uint8_t";
	case Column_Type_I64   : return "int64_t";
	case Column_Type_I32   : return "int32_t";
	case Column_Type_I16   : return "int16_t";
	case Column_Type_I8    : return "int8_t";
	case Column_Type_Float : return "float";
	case Column_Type_Double: return "double";
	case Column_Type_Char  : return "char";
	case Column_Type_Bool  : return "char";
	case Column_Type_String: return "string";
	default:
		fb_assertf(false, "Invalid column type: %d", t);
		return "unknown";
	};
}
template<typename T> static const char* getColumnTypeString() { return getColumnTypeString(getColumnType<T>()); }
template<typename T> static const char* getColumnTypeString(T val) { return getColumnTypeString<T>(); }

template<typename T> static bool validate(Column_Type t) { return getColumnType<T>() == t; }
template<typename T> static bool validate(Column_Type t, T) { return getColumnType<T>() == t; }

enum { DefaultDataBufferSize = 256 };

FB_END_PACKAGE2()

FB_PACKAGE0()

class DataBase
{
public:
	friend class ColumnIterator;

	SizeType rowCount;
	PodVector<container::database::Column_Type> columnTypes;

	SizeType bufferSize; // in bytes
	PodVector<uint8_t*> dataBuffers;
	PodVector< Pair<SizeType, SizeType> > columnBufferMap;
	PodVector<char> stringStack;

	PodVector<SizeType> unusedDataBuffers;
	PodVector<uint8_t*> ownedPointers;
	SizeType ownedPointersTotalSize;

	static const SizeType ReserveIncrement = 4 * 1024;

	~DataBase()
	{
		for (SizeType i = 0; i < ownedPointers.getSize(); ++i)
		{
			fb_assert(ownedPointers[i] != nullptr);
			lang::freeFixed(ownedPointers[i], ReserveIncrement);
			ownedPointers[i] = nullptr;
		}
	}
	DataBase(const ArraySlice<container::database::Column_Type>& theColumnTypes, SizeType theDataBufferSize = container::database::DefaultDataBufferSize)
		: rowCount(0)
		, bufferSize(theDataBufferSize)
		, ownedPointersTotalSize(0)
	{
		columnTypes.reserve(theColumnTypes.getSize());
		for (container::database::Column_Type t : theColumnTypes)
		{
			columnTypes.pushBack(t);
		}
	}
	DataBase(SizeType theDataBufferSize = container::database::DefaultDataBufferSize)
		: rowCount(0)
		, bufferSize(theDataBufferSize)
		, ownedPointersTotalSize(0)
	{
	}

	void clear()
	{
		rowCount = 0;
		stringStack.clear();
	}

	SizeType getColumnCount() const { return columnTypes.getSize(); }
	SizeType getRowCount() const { return rowCount; }

	void reserveSpaceForColumn(SizeType column, SizeType count = 1);
	void reserveImpl(SizeType size);


	uint8_t* getCellImpl(SizeType column, SizeType row) const;
	uint8_t* getNextCellImpl(uint8_t* currentCell, SizeType column, SizeType currentRow, SizeType nextRow) const;
	
	template<typename T> inline T get(SizeType column, SizeType row) const
	{
		fb_assert(validate(columnTypes[column], T()));

		// NOTE: String results explicitly casted to T to prevent VC from complaining

		uint8_t* cell = getCellImpl(column, row);

		if (lang::IsSame<T, bool>::value)
		{
			uint8_t temp = *(const uint8_t*)cell;
			uint8_t bit = row % 8U;
			bool result = ((temp & (1U << bit)) != 0U);
			return *(T*)&result;
		}
		else if (lang::IsSame<T, const char*>::value || lang::IsSame<T, char*>::value)
		{
			container::database::StringIndexType stringStackIndex = *(container::database::StringIndexType*)cell;
			fb_assert(stringStackIndex < stringStack.getSize());
			const char* result = &stringStack[stringStackIndex];
			return *(T*)&result;
		}

		return *(const T*)cell;
	}

	uint64_t        getU64   (SizeType column, SizeType row) const { return get<uint64_t   >(column, row); }
	uint32_t        getU32   (SizeType column, SizeType row) const { return get<uint32_t   >(column, row); }
	uint16_t        getU16   (SizeType column, SizeType row) const { return get<uint16_t   >(column, row); }
	uint8_t         getU8    (SizeType column, SizeType row) const { return get<uint8_t    >(column, row); }
	int64_t         getI64   (SizeType column, SizeType row) const { return get<int64_t    >(column, row); }
	int32_t         getI32   (SizeType column, SizeType row) const { return get<int32_t    >(column, row); }
	int16_t         getI16   (SizeType column, SizeType row) const { return get<int16_t    >(column, row); }
	int8_t          getI8    (SizeType column, SizeType row) const { return get<int8_t     >(column, row); }
	float           getFloat (SizeType column, SizeType row) const { return get<float      >(column, row); }
	double          getDouble(SizeType column, SizeType row) const { return get<double     >(column, row); }
	char            getChar  (SizeType column, SizeType row) const { return get<char       >(column, row); }
	bool            getBool  (SizeType column, SizeType row) const { return get<bool       >(column, row); }
	container::database::StringIndexType getStringIndex(SizeType column, SizeType row) const { return *(container::database::StringIndexType*)getCellImpl(column, row); }
	const char*     getString(SizeType column, SizeType row) const;

	template<typename... Targs>
	void appendRow(Targs... Fargs)
	{
		fb_assertf(sizeof...(Fargs) == getColumnCount(), "Invalid number of columns. Expected: %d, received: %d", getColumnCount(), uint32_t(sizeof...(Fargs)));

		setRowRecursiveImpl<0>(rowCount, Fargs...);
		++rowCount;
	}

	template<SizeType column, typename T, typename... Targs> FB_FORCEINLINE void setRowRecursiveImpl(SizeType row, T value, Targs... Fargs)
	{
		setCellImpl(column, row, value);
		setRowRecursiveImpl<column + 1>(row, Fargs...);
	}

	template<SizeType column, typename T> FB_FORCEINLINE void setRowRecursiveImpl(SizeType row, T value)
	{
		setCellImpl(column, row, value);
	}

	void setCellImpl(SizeType column, SizeType row, bool value)                         { setBool(column, row, value); }
	void setCellImpl(SizeType column, SizeType row, char* value)                        { setString(column, row, value, cstrlen(value)); }
	void setCellImpl(SizeType column, SizeType row, const char* value)                  { setString(column, row, value, cstrlen(value)); }
	//void setCellImpl(SizeType column, SizeType row, const AnyString& value)         { setString(column, row, value.getPointer(), value.getLength()); }
	//void setCellImpl(SizeType column, SizeType row, const DynamicString& value) { setString(column, row, value.getPointer(), value.getLength()); }
	//void setCellImpl(SizeType column, SizeType row, const HeapString& value)    { setString(column, row, value.getPointer(), value.getLength()); }

	void trimStrings(); // Call after changing or removing lots of strings

	template<typename T>
	void setCellImpl(SizeType column, SizeType row, const T value)
	{
		fb_static_assertf((!lang::IsSame<T, bool>::value), "setCellImpl doesn't do boolean values use setBool");
		fb_static_assertf((!lang::IsSame<T, const char*>::value && !lang::IsSame<T, char*>::value), "setCellImpl doesn't do string values use setString");

		fb_assertf(column < columnTypes.getSize(), "Column number out of bounds. Count: %d, trying to set: %d", columnTypes.getSize(), column);
		fb_assertf(row <= rowCount, "Attempting to set row beyond current row count. Count: %d, trying to set: %d", rowCount, row);
		fb_assertf(validate(columnTypes[column], value), "Invalid type for column %d. Expected: %s (%d), received %s type.", 
			column, container::database::getColumnTypeString(columnTypes[column]), columnTypes[column], container::database::getColumnTypeString(value));

		const SizeType valueBitSize = container::database::getColumnTypeSize(container::database::getColumnType(value));
		const SizeType valueByteSize = valueBitSize / 8;

		if (row == rowCount && row % (bufferSize * 8 / valueBitSize) == 0)
		{
			reserveSpaceForColumn(column, 1);
		}

		uint8_t* cell = getCellImpl(column, row);
		lang::MemCopy::copy(cell, &value, valueByteSize);
	}
	
	void setU64   (SizeType column, SizeType row, uint64_t value)    { setCellImpl(column, row, value); }
	void setU32   (SizeType column, SizeType row, uint32_t value)    { setCellImpl(column, row, value); }
	void setU16   (SizeType column, SizeType row, uint16_t value)    { setCellImpl(column, row, value); }
	void setU8    (SizeType column, SizeType row, uint8_t  value)    { setCellImpl(column, row, value); }
	void setI64   (SizeType column, SizeType row, int64_t  value)    { setCellImpl(column, row, value); }
	void setI32   (SizeType column, SizeType row, int32_t  value)    { setCellImpl(column, row, value); }
	void setI16   (SizeType column, SizeType row, int16_t  value)    { setCellImpl(column, row, value); }
	void setI8    (SizeType column, SizeType row, int8_t   value)    { setCellImpl(column, row, value); }
	void setFloat (SizeType column, SizeType row, float value)       { setCellImpl(column, row, value); }
	void setDouble(SizeType column, SizeType row, double value)      { setCellImpl(column, row, value); }
	void setChar  (SizeType column, SizeType row, char value)        { setCellImpl(column, row, value); }
	void setString(SizeType column, SizeType row, const char* value) { setString(column, row, value, cstrlen(value)); }
	void setString(SizeType column, SizeType row, const char* value, SizeType length);
	void setBool  (SizeType column, SizeType row, bool value);

	void removeRow(SizeType startingRow, SizeType count = 1);
	void removeRowOrdered(SizeType startingRow, SizeType count = 1);
};

class ColumnIterator
{
	// Browsing a column with iterator is faster than repeatedly using DataBase::get... or set... functions

	friend class DataBase;
public:

	// NOTE: Clang and MSCV disagree if an object is trivially copyable if it has const members or not

	DataBase* dataBase;
	/*const*/ SizeType column;
	uint8_t* currentCell;
	SizeType currentRow;
	bool should_be_const = false;
	/*const*/ SizeType byteSize;        // const after constructor
	/*const*/ SizeType valuesPerBuffer; // const after constructor

	ColumnIterator(const DataBase* dataBase, SizeType column, SizeType startingRow)
		: ColumnIterator(const_cast<DataBase*>(dataBase), column, startingRow)
	{
		should_be_const = true;
	}
	ColumnIterator(DataBase* dataBase, SizeType column, SizeType startingRow)
		: dataBase(dataBase)
		, column(column)
		, currentRow(startingRow)
		, byteSize(getColumnTypeSize(dataBase->columnTypes[column]) / 8)
		, valuesPerBuffer(dataBase->bufferSize * 8 / getColumnTypeSize(dataBase->columnTypes[column]))
	{
		currentCell = dataBase->getRowCount() > 0 ? (uint8_t*)dataBase->getCellImpl(column, startingRow) : nullptr;
	}

	void jump(SizeType row)
	{
		if (currentRow / valuesPerBuffer == row / valuesPerBuffer)
		{
			fb_assert(nullptr != currentCell);
			if (byteSize > 0)
				currentCell = currentCell - currentRow * byteSize + row * byteSize;
			else
				currentCell = currentCell - currentRow / 8 + row / 8;
		}
		else
		{
			currentCell = (uint8_t*)dataBase->getCellImpl(column, row);
		}
		currentRow = row;
	}

	void next()
	{
		++currentRow;
		if (currentRow % valuesPerBuffer != 0)
		{
			fb_assert(nullptr != currentCell);
			if (byteSize > 0)
				currentCell += byteSize;
			else if (currentRow % 8 == 0) // for booleans
				currentCell += 1;
		}
		else if (currentRow < dataBase->rowCount)
		{
			currentCell = (uint8_t*)dataBase->getCellImpl(column, currentRow);
		}
	}

	template<typename T> inline T get() const
	{
		fb_static_assertf((!lang::IsSame<T, bool>::value), "Don't call get<bool>() use getBool().");
		fb_assert(validate(dataBase->columnTypes[column], T()));
		fb_assertf(currentRow < dataBase->getRowCount(), "%d < %d", currentRow, dataBase->getRowCount());

		// NOTE: String and bool results explicitly casted to T to prevent VC from complaining

		if (lang::IsSame<T, const char*>::value || lang::IsSame<T, char*>::value)
		{
			container::database::StringIndexType stringStackIndex = *(container::database::StringIndexType*)currentCell;
			fb_assert(stringStackIndex < dataBase->stringStack.getSize());
			const char* result = &dataBase->stringStack[stringStackIndex];
			return *(T*)&result;
		}

		return *(const T*)currentCell;
	}

	uint64_t    getU64()    const { return get<uint64_t   >(); }
	uint32_t    getU32()    const { return get<uint32_t   >(); }
	uint16_t    getU16()    const { return get<uint16_t   >(); }
	uint8_t     getU8()    const { return get<uint8_t    >(); }
	int64_t     getI64()    const { return get<int64_t    >(); }
	int32_t     getI32()    const { return get<int32_t    >(); }
	int16_t     getI16()    const { return get<int16_t    >(); }
	int8_t      getI8()    const { return get<int8_t     >(); }
	const char* getString() const { return get<const char*>(); }
	float       getFloat()  const { return get<float      >(); }
	double      getDouble()	const { return get<double     >(); }
	char        getChar()   const { return get<char       >(); }
	bool        getBool()   const
	{
		fb_assert(validate(dataBase->columnTypes[column], bool()));
		uint8_t temp = *(const uint8_t*)currentCell;
		uint8_t bit = currentRow % 8U;
		bool result = ((temp & (1U << bit)) != 0U);
		return result;
	}

	template<typename T> inline void set(const T& value)
	{
		fb_static_assert((lang::IsTriviallyCopyable<T>::value));
		fb_assert(validate(dataBase->columnTypes[column], T()));
		fb_assert(false == ColumnIterator::should_be_const && "ColumnIterator database is marked as const, but still trying to set using it.");

		// NOTE: String and bool values explicitly casted to T to prevent VC from complaining

		if (lang::IsSame<T, bool>::value)
		{
			uint8_t b = *reinterpret_cast<const uint8_t*>(&value);
			fb_assert(b == 1 || b == 0);

			// For booleans
			uint8_t temp = *(const uint8_t*)currentCell;
			uint8_t place = currentRow % 8U;
			uint8_t negativeMask = ~(1U << place);
			uint8_t valueMask = uint8_t(b << place);
			temp &= negativeMask;
			temp |= valueMask;
			*(uint8_t*)currentCell = temp;
		}
		else if (lang::IsSame<T, const char*>::value || lang::IsSame<T, char*>::value)
		{
			const char* str = *(const char**)(&value);
			setString(str, cstrlen(str));
		}
		else
		{
			*(T*)currentCell = value;
		}
	}

	void setU64(uint64_t value) { set(value); }
	void setU32(uint32_t value) { set(value); }
	void setU16(uint16_t value) { set(value); }
	void setU8(uint8_t  value) { set(value); }
	void setI64(int64_t  value) { set(value); }
	void setI32(int32_t  value) { set(value); }
	void setI16(int16_t  value) { set(value); }
	void setI8(int8_t   value) { set(value); }
	void setString(const char* value) { set(value); }
	void setFloat(float value) { set(value); }
	void setDouble(double value) { set(value); }
	void setChar(char value) { set(value); }
	void setBool(bool value) { set(value); }

	void setString(const char* value, SizeType length) { dataBase->setString(column, currentRow, value, length); }

protected:
	uint8_t* getImpl() { return currentCell; }
	const uint8_t* getImpl() const { return currentCell; }
};

template<typename T>
class ColumnIteratorTemplated : public ColumnIterator
{
public:
	T getValue() { return get<T>(); }
	ColumnIteratorTemplated(DataBase* dataBase, SizeType column, SizeType startingRow) : ColumnIterator(dataBase, column, startingRow) {  }
};

template<typename DB, SizeType Column>
class ColumnIteratorTemplated2 : public ColumnIteratorTemplated<typename DB::template Get_Type<Column>::type>
{
	typedef ColumnIteratorTemplated<typename DB::template Get_Type<Column>::type> BaseClass;

public:
	ColumnIteratorTemplated2(DB* dataBase, SizeType startingRow) : BaseClass(dataBase, Column, startingRow) {  }
	ColumnIteratorTemplated2(const DB* dataBase, SizeType startingRow) : BaseClass(const_cast<DB*>(dataBase), Column, startingRow) { ColumnIterator::should_be_const = true; }
};


template<typename... Targs>
class TemplatedDataBase : public DataBase
{
public:
	fb_static_assertf((sizeof...(Targs) > 0), "Must have atleast one templated argument type");

	template<SizeType column> class Get_Type : public std::tuple_element<column, std::tuple<Targs...> > { };

	enum { ColumnCount = sizeof...(Targs) };

	FB_FORCEINLINE SizeType getColumnCount() const { return SizeType(sizeof...(Targs)); }

	template<typename... Sargs>
	FB_FORCEINLINE void copy(const TemplatedDataBase<Sargs...>& source)
	{
		fb_static_assertf((sizeof...(Targs) == sizeof...(Sargs)), "Column count mismatch.");
		fb_static_assertf((sizeof...(Targs) != sizeof...(Sargs) || lang::IsSame<std::tuple<Targs...>, std::tuple<Sargs...> >::value), "Column type mismatch.");
		copyImpl(source);
	}
	FB_FORCEINLINE void copy(const DataBase& source)
	{
		fb_assertf(source.getColumnCount() == getColumnCount(), "Column count mismatch. Template argument count: %d, source column count: %d", getColumnCount(), source.getColumnCount());

		validateRecursively<0, Targs...>(source.columnTypes);

		copyImpl(source);
	}
	void copyImpl(const DataBase& source)
	{
		// TODO
	}

	TemplatedDataBase(SizeType theDataBufferSize = container::database::DefaultDataBufferSize)
		: DataBase(theDataBufferSize)
	{
		this->columnTypes.reserve(sizeof...(Targs));
		this->columnTypes.clear();
		pushColumnTypesRecursive<Targs...>();
	}

	FB_FORCEINLINE void appendRow(Targs... Fargs)
	{
		DataBase::appendRow(Fargs...);
	}

	template<SizeType column> FB_FORCEINLINE typename Get_Type<column>::type get(SizeType row) const
	{
		//fb_static_assertf((lang::IsConvertible<typename Get_Type<column>::type, T>::value), "Invalid value type for database column.");
		return DataBase::get<typename Get_Type<column>::type>(column, row);
	}
	template<SizeType column> FB_FORCEINLINE void set(SizeType row, const typename Get_Type<column>::type& value)
	{
		DataBase::setCellImpl(column, row, value);
	}


private:

	template<SizeType column, typename T, typename... Fargs> FB_FORCEINLINE void validateRecursively(const PodVector<container::database::Column_Type>& ct)
	{
		fb_assertf(container::database::validate<T>(ct[column]), "Column type mismatch at %d. Expected: %s, got: %s (%d)", 
			column, container::database::getColumnTypeString<T>(), container::database::getColumnTypeString(ct[column]), ct[column]);
		validateRecursively<column + 1, Fargs...>(ct);
	}
	template<SizeType column, int T = 0> FB_FORCEINLINE void validateRecursively(const PodVector<container::database::Column_Type>& ct) { }

	template<typename T, typename... Fargs>
	FB_FORCEINLINE void pushColumnTypesRecursive()
	{
		//fb_static_assert((!lang::IsSame<T, HeapString>::value));
		//fb_static_assert((!lang::IsSame<T, DynamicString>::value));
		//fb_static_assert((!lang::IsSame<T, StaticString>::value));

		this->columnTypes.pushBack(container::database::getColumnType<T>());
		pushColumnTypesRecursive<Fargs...>();
	}
	template<int T = 0> FB_FORCEINLINE  void pushColumnTypesRecursive() const { /* Used to terminate recursive template resolving */ }
};

extern void testDataBase();

FB_END_PACKAGE0();
