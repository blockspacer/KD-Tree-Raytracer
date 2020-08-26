#include "Precompiled.h"
#include "DataBase.h"

#include "fb/lang/Swap.h"

FB_PACKAGE0();

void DataBase::reserveSpaceForColumn(SizeType column, SizeType count)
{
	if (unusedDataBuffers.getSize() < count)
	{
		reserveImpl(bufferSize * count);
	}

	fb_assert(count <= unusedDataBuffers.getSize());

	while (count > 0)
	{
		SizeType bufferIndex = unusedDataBuffers.getBack();
		unusedDataBuffers.popBack();
		columnBufferMap.pushBack(Pair<SizeType, SizeType>(column, bufferIndex));
		--count;
	}
}

void DataBase::reserveImpl(const SizeType theSize)
{
	if (theSize == 0)
		return;

	SizeType oldUnusedBuffersSize = unusedDataBuffers.getSize();
	SizeType size = theSize;
	if (size < ReserveIncrement)
		size = ReserveIncrement;

	while (size > 0)
	{
		uint8_t* ptr = (uint8_t*)lang::allocateFixed(ReserveIncrement);
		fb_assert(ptr);
		ownedPointers.pushBack(ptr);
		ownedPointersTotalSize += ReserveIncrement;

		for (SizeType i = 0; i < size; i += bufferSize)
		{
			fb_assert(!findIfContains(unusedDataBuffers, dataBuffers.getSize()));
			unusedDataBuffers.pushBack(dataBuffers.getSize());
			dataBuffers.pushBack(ptr + i);
		}

		size = size > ReserveIncrement ? size - ReserveIncrement : 0U;
	}

	// Reverse unused data buffer order so that dataBuffers are used in order.
	auto first = unusedDataBuffers.getBegin() + oldUnusedBuffersSize;
	auto last = unusedDataBuffers.getEnd();
	while (first < last)
		lang::swap(*first++, *--last);
}

uint8_t* DataBase::getCellImpl(SizeType column, SizeType row) const
{
	SizeType bitSize = getColumnTypeSize(columnTypes[column]);
	SizeType valuesPerBuffer = bufferSize * 8 / bitSize;
	SizeType targetBuffer = row / valuesPerBuffer;
	SizeType positionInBuffer = (row % valuesPerBuffer) * bitSize / 8;
	SizeType bufferIndexResult = ~0U;

	for (const Pair<SizeType, SizeType>& p : columnBufferMap)
	{
		if (p.first != column)
			continue;
		if (targetBuffer > 0)
		{
			--targetBuffer;
			continue;
		}

		bufferIndexResult = p.second;
		break;
	};

	fb_assertf(bufferIndexResult != ~0U, "Buffer index not found. Row out of bounds? Row: %d, rowCount: %d", row, rowCount);

	return dataBuffers[bufferIndexResult] + positionInBuffer;
}

uint8_t* DataBase::getNextCellImpl(uint8_t* currentCell, SizeType column, SizeType currentRow, SizeType nextRow) const
{
	SizeType bitSize = getColumnTypeSize(columnTypes[column]);
	SizeType valuesPerBuffer = bufferSize * 8 / bitSize;

	if (currentRow / valuesPerBuffer == nextRow / valuesPerBuffer)
	{
		SizeType currentPos = (currentRow % valuesPerBuffer) * bitSize / 8;
		SizeType nextPos = (nextRow    % valuesPerBuffer) * bitSize / 8;

		return ((uint8_t*)currentCell) - currentPos + nextPos;
	}

	return getCellImpl(column, nextRow);
}

void DataBase::setBool(SizeType column, SizeType row, bool value)
{
	fb_assert(row <= rowCount);
	fb_assertf(columnTypes[column] == container::database::Column_Type_Bool, "Invalid column type for a bool. Expected: %d, received: bool (%d)", columnTypes[column], container::database::Column_Type_Bool);

	const SizeType valueBitSize = getColumnTypeSize(container::database::getColumnType(value));
	if (row == rowCount && row % (bufferSize * 8 / valueBitSize) == 0)
	{
		reserveSpaceForColumn(column, 1);
	}

	uint8_t* cell = getCellImpl(column, row);
	fb_assert(value == 1 || value == 0);

	// For booleans
	uint8_t temp = *(uint8_t*)cell;
	uint8_t place = row % 8U;
	uint8_t negativeMask = ~(1U << place);
	uint8_t valueMask = uint8_t(value << place);
	temp &= negativeMask;
	temp |= valueMask;
	*(uint8_t*)cell = temp;
}

const char* DataBase::getString(SizeType column, SizeType row) const
{
	const char* result = nullptr;
	fb_assert(validate(columnTypes[column], result));

	uint8_t* cell = getCellImpl(column, row);
	container::database::StringIndexType stringStackIndex = *(container::database::StringIndexType*)cell;
	fb_assert(stringStackIndex < stringStack.getSize());
	result = &stringStack[stringStackIndex];
	return result;
}

void DataBase::setString(SizeType column, SizeType row, const char* value, SizeType length)
{
	fb_assert(row <= rowCount);
	fb_assertf(columnTypes[column] == container::database::Column_Type_String, "Invalid column type for a string. Expected: %d, received: string (%d)", 
		columnTypes[column], container::database::Column_Type_String);

	const SizeType valueBitSize = container::database::getColumnTypeSize(container::database::getColumnType(value));
	if (row == rowCount && row % (bufferSize * 8 / valueBitSize) == 0)
	{
		reserveSpaceForColumn(column, 1);
	}

	if (stringStack.getSize() + length + 1 > stringStack.getCapacity())
	{
		fb_assert(stringStack.getSize() + length + 1 < 10 * 1024 * 1024 && "String stack in DataBase is over 10 MB. Probably should look into that. Implement trimming?");
		stringStack.reserve(stringStack.getSize() + length + 1);
	}

	container::database::StringIndexType stringStartPos = stringStack.getSize();
	stringStack.insert(stringStack.getEnd(), value, length);
	stringStack.pushBack('\0');

	container::database::StringIndexType* cell = (container::database::StringIndexType*)getCellImpl(column, row);
	*cell = stringStartPos;
}

void DataBase::trimStrings()
{
	// TODO
}

void DataBase::removeRow(SizeType startingRow, SizeType count)
{
	// Swap out the row

	fb_assert(count <= rowCount);
	fb_assert(startingRow + count <= rowCount);

	if (startingRow + count == rowCount)
	{
		if (rowCount >= count)
			rowCount -= count;
		return;
	}

	// TODO, OPTMIZE: This shifting could be optimized harnessing the knowledge that the cells are somewhat sequentially in memory

	for (SizeType col = 0; col < getColumnCount(); ++col)
	{
		const SizeType valueBitSize = getColumnTypeSize(columnTypes[col]);
		const SizeType valueByteSize = valueBitSize / 8;

		ColumnIterator it(this, col, startingRow);
		ColumnIterator itShift(this, col, startingRow + count);

		for (; itShift.currentRow < rowCount; it.next(), itShift.next())
		{
			if (valueBitSize == 1)
			{
				it.setBool(itShift.getBool());
			}
			else
			{
				uint8_t* dstCell = it.currentCell;
				uint8_t* srcCell = itShift.currentCell;

				lang::MemCopy::copy(dstCell, srcCell, valueByteSize);
			}
		}
	}
	rowCount -= count;
}

void DataBase::removeRowOrdered(SizeType startingRow, SizeType count)
{
	// Shift all rows back one space

	fb_assert(count <= rowCount);
	fb_assert(startingRow + count <= rowCount);

	if (startingRow + count == rowCount)
	{
		if (rowCount >= count)
			rowCount -= count;
		return;
	}

	// TODO, OPTMIZE: This shifting could be optimized harnessing the knowledge that the cells are somewhat sequentially in memory

	for (SizeType col = 0; col < getColumnCount(); ++col)
	{
		const SizeType valueBitSize = getColumnTypeSize(columnTypes[col]);
		const SizeType valueByteSize = valueBitSize / 8;

		ColumnIterator it(this, col, startingRow);
		ColumnIterator itShift(this, col, startingRow + count);

		for (; itShift.currentRow < rowCount; it.next(), itShift.next())
		{
			if (valueBitSize == 1)
			{
				it.setBool(itShift.getBool());
			}
			else
			{
				uint8_t* dstCell = it.currentCell;
				uint8_t* srcCell = itShift.currentCell;

				lang::MemCopy::copy(dstCell, srcCell, valueByteSize);
			}
		}
	}
	rowCount -= count;
}


#if 1 // Requires bad dependencies so removed

void testDataBase() {}

#else

void testDataBase()
{
	Column_Type columnsTypes[] = {
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
		Column_Type_Bool
	};

	DataBase d(sliceFromArray(columnsTypes));

	for (SizeType i = 0; i < 100; ++i)
	{
		d.appendRow(
			uint64_t(0xFFFFFFFFFFFFFFFF)
			, uint32_t(0xFFFFFFFF)
			, uint16_t(0xFFFF)
			, uint8_t(0xFF)
			, int64_t(0x8000000000000000)
			, int32_t(0x80000000)
			, int16_t(-0x7FFF)
			, int8_t(-0x7F)
			, "ASDF"
			, float(3.14f)
			, double(5.5)
			, char('A' + 2 * (i % 10))
			, true
		);
		d.appendRow(
			uint64_t(0x1)
			, uint32_t(0x2)
			, uint16_t(0x3)
			, uint8_t(0x4)
			, int64_t(-0x5)
			, int32_t(-0x6)
			, int16_t(-0x7)
			, int8_t(-0x8)
			, "lolmfao"
			, float(1.23f)
			, double(4.56)
			, char('B' + 2 * (i % 10))
			, false
		);

	}

	d.setString(8, 0, "JOUUUUU");
	FB_PRINTF("\nString test: %s\n", d.getString(8, 0));

	d.setString(8, 0, "OLOLOLO");
	FB_PRINTF("String test: %s\n", d.getString(8, 0));

	d.setBool(12, 3, true);
	FB_PRINTF("Bool test: %s\n", d.getBool(12, 3) ? "true" : "false");

	d.setBool(12, 3, false);
	FB_PRINTF("Bool test: %s\n", d.getBool(12, 3) ? "true" : "false");


	ColumnIteratorTemplated<uint64_t>(&d, 0, 0);

	struct LOLMBDA
	{
		static void doSprintf(ColumnIterator& self, HeapString& str, SizeType width = 16)
		{
			switch (self.dataBase->columnTypes[self.column])
			{
			case Column_Type_U64:    str.doSprintf("%*llu", width, self.getU64()); break;
			case Column_Type_U32:    str.doSprintf("%*u", width, self.getU32()); break;
			case Column_Type_U16:    str.doSprintf("%*hu", width, self.getU16()); break;
			case Column_Type_U8:     str.doSprintf("%*hhu", width, self.getU8()); break;
			case Column_Type_I64:    str.doSprintf("%*lli", width, self.getI64()); break;
			case Column_Type_I32:    str.doSprintf("%*i", width, self.getI32()); break;
			case Column_Type_I16:    str.doSprintf("%*hi", width, self.getI16()); break;
			case Column_Type_I8:     str.doSprintf("%*hhi", width, self.getI8()); break;
			case Column_Type_String: str.doSprintf("%*s", width, self.getString()); break;
			case Column_Type_Float:  str.doSprintf("%*f", width, self.getFloat()); break;
			case Column_Type_Double: str.doSprintf("%*f", width, self.getDouble()); break;
			case Column_Type_Char:   str.doSprintf("%*c", width, self.getChar()); break;
			case Column_Type_Bool:   str.doSprintf("%*s", width, self.getBool() ? "true" : "false"); break;
			default:
				return;
			}
		}

		static void print(const std::initializer_list<const char*>& names, DataBase& d)
		{
			TempString str;
			for (SizeType i = 0; i < names.size(); ++i)
			{
				const char* name = names.begin()[i];
				if (i == 0)
					str.doSprintf("%21s, ", name);
				else if (i + 1 == names.size())
					str.doSprintf("%16s", name);
				else
					str.doSprintf("%16s, ", name);
			}
			FB_PRINTF("%s\n", str.getPointer());

			print(d);
		}
		static void print(Vector<StaticString>& names, DataBase& d)
		{
			TempString str;
			for (SizeType i = 0; i < names.getSize(); ++i)
			{
				if (i == 0)
					str.doSprintf("%21s, ", names[i].getPointer());
				else if (i + 1 == names.getSize())
					str.doSprintf("%16s", names[i].getPointer());
				else
					str.doSprintf("%16s, ", names[i].getPointer());
			}
			FB_PRINTF("%s\n", str.getPointer());

			print(d);
		}
		static void print(DataBase& d)
		{
			PodVector<ColumnIterator> columnIterators;
			for (SizeType i = 0; i < d.getColumnCount(); ++i)
			{
				columnIterators.pushBack(ColumnIterator(&d, i, 0));
			}

			for (SizeType i = 0; i < d.rowCount; ++i)
			{
				TempString str;
				for (SizeType col = 0; col < columnIterators.getSize(); ++col)
				{
					columnIterators[col].jump(i);
					doSprintf(columnIterators[col], str);

					if (col + 1 < columnIterators.getSize())
						str += ",";
				}
				FB_PRINTF("%3d> %s\n", i, str.getPointer());
			}
		}
	};

	{
		// Stream tests

		//fb::stream::OutputStream<lang::BigEndian> outStrm;
		//d.write(outStrm);
		//
		//fb::stream::InputStream<lang::BigEndian> inStrm(outStrm.getData(), outStrm.getSize());
		//DataBase e;
		//e.read(inStrm);
		//
		//FB_PRINTF("Source: \n");
		//LOLMBDA::print(d);
		//FB_PRINTF("Result: \n");
		//LOLMBDA::print(e);
	}

	typedef TemplatedDataBase<int16_t, float, uint64_t, char, const char*, bool> TestDataBase;
	typedef TemplatedDataBase<int16_t, float, uint64_t, char, const char*, bool, bool> TestDataBase2;
	TestDataBase tt;

	tt.appendRow(12, 22, 32, 'A', "juj", true);
	tt.appendRow(12, 22, 32, 'b', "asdf", false);

	//float a = tt.get<1>(0);
	FB_UNUSED_NAMED_VAR(int, b) = tt.get<0>(0);
	FB_UNUSED_NAMED_VAR(int, c) = tt.get<0>(0);

	tt.set<1>(0, 1);

	//FB_PRINTF("\n\nFLOAT: %f\n\n", tt.get<float, 1>(0));
	//FB_PRINTF("\n\nFLOAT: %f\n\n", tt.get<float, 2>(0));

	tt.set<1>(0, 1.3f);
	tt.set<1>(0, 1.4);

	//tt.copy(e); // Runtime error
	//tt.copy(TestDataBase2()); // Compile time error

	LOLMBDA::print({ "int16_t", "float", "uint64_t", "char", "const char*", "bool" }, tt);
}
#endif

FB_END_PACKAGE0();
