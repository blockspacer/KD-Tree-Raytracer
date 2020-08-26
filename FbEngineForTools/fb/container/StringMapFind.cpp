#include "Precompiled.h"
#include "StringMapFind.h"

#include "fb/string/DynamicString.h"
#include "fb/string/HeapString.h"

FB_PACKAGE0()

void test()
{
	{
		typedef LinearMap<DynamicString, int> MapType;
		MapType map;

		FB_UNUSED_NAMED_VAR(SizeType, index) = 0U;
		index = StringMapFind::findIndex(map, "asdf");
		index = StringMapFind::findIndex(map, StringRef("asdf"));
		index = StringMapFind::findIndex(map, HeapString("asdf"));
		index = StringMapFind::findIndex(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(MapType::Iterator, it);
		it = StringMapFind::findIterator(map, "asdf");
		it = StringMapFind::findIterator(map, StringRef("asdf"));
		it = StringMapFind::findIterator(map, HeapString("asdf"));
		it = StringMapFind::findIterator(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(int *, insertVal);
		insertVal = &StringMapFind::findOrInsert(map, "asdf");
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"));

		insertVal = &StringMapFind::findOrInsert(map, "asdf", 1);
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"), 1);

		FB_UNUSED_NAMED_VAR(StringMapFind::InsertPair<MapType>, insertPair);
		insertPair = StringMapFind::findOrInsertIterator(map, "asdf");
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"));

		insertPair = StringMapFind::findOrInsertIterator(map, "asdf", 1);
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"), 1);
	}

	{
		typedef LinearMap<HeapString, int> MapType;
		MapType map;

		FB_UNUSED_NAMED_VAR(SizeType, index) = 0U;
		index = StringMapFind::findIndex(map, "asdf");
		index = StringMapFind::findIndex(map, StringRef("asdf"));
		index = StringMapFind::findIndex(map, HeapString("asdf"));
		index = StringMapFind::findIndex(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(MapType::Iterator, it);
		it = StringMapFind::findIterator(map, "asdf");
		it = StringMapFind::findIterator(map, StringRef("asdf"));
		it = StringMapFind::findIterator(map, HeapString("asdf"));
		it = StringMapFind::findIterator(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(int *, insertVal);
		insertVal = &StringMapFind::findOrInsert(map, "asdf");
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"));

		insertVal = &StringMapFind::findOrInsert(map, "asdf", 1);
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"), 1);

		FB_UNUSED_NAMED_VAR(StringMapFind::InsertPair<MapType>, insertPair);
		insertPair = StringMapFind::findOrInsertIterator(map, "asdf");
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"));

		insertPair = StringMapFind::findOrInsertIterator(map, "asdf", 1);
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"), 1);
	}

	{
		typedef LinearHashMap<DynamicString, int> MapType;
		MapType map;

		FB_UNUSED_NAMED_VAR(MapType::Iterator, it) = map.getBegin();
		it = StringMapFind::findIterator(map, "asdf");
		it = StringMapFind::findIterator(map, StringRef("asdf"));
		it = StringMapFind::findIterator(map, HeapString("asdf"));
		it = StringMapFind::findIterator(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(int *, insertVal);
		insertVal = &StringMapFind::findOrInsert(map, "asdf");
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"));
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"));

		insertVal = &StringMapFind::findOrInsert(map, "asdf", 1);
		insertVal = &StringMapFind::findOrInsert(map, StringRef("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, HeapString("asdf"), 1);
		insertVal = &StringMapFind::findOrInsert(map, DynamicString("asdf"), 1);

		FB_UNUSED_NAMED_VAR(StringMapFind::InsertPair<MapType>, insertPair) = StringMapFind::InsertPair<MapType>(map.getBegin(), false);
		insertPair = StringMapFind::findOrInsertIterator(map, "asdf");
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"));
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"));

		insertPair = StringMapFind::findOrInsertIterator(map, "asdf", 1);
		insertPair = StringMapFind::findOrInsertIterator(map, StringRef("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, HeapString("asdf"), 1);
		insertPair = StringMapFind::findOrInsertIterator(map, DynamicString("asdf"), 1);
	}

	{
		typedef LinearMap<DynamicString, int> MapType;
		const MapType map;

		FB_UNUSED_NAMED_VAR(SizeType, index) = 0U;
		index = StringMapFind::findIndex(map, "asdf");
		index = StringMapFind::findIndex(map, StringRef("asdf"));
		index = StringMapFind::findIndex(map, HeapString("asdf"));
		index = StringMapFind::findIndex(map, DynamicString("asdf"));

		FB_UNUSED_NAMED_VAR(MapType::ConstIterator, it);
		it = StringMapFind::findIterator(map, "asdf");
		it = StringMapFind::findIterator(map, StringRef("asdf"));
		it = StringMapFind::findIterator(map, HeapString("asdf"));
		it = StringMapFind::findIterator(map, DynamicString("asdf"));
	}

	{
		typedef LinearHashMap<DynamicString, int> MapType;
		const MapType map;

		FB_UNUSED_NAMED_VAR(MapType::ConstIterator, it) = map.getBegin();
		it = StringMapFind::findIterator(map, "asdf");
		it = StringMapFind::findIterator(map, StringRef("asdf"));
		it = StringMapFind::findIterator(map, HeapString("asdf"));
		it = StringMapFind::findIterator(map, DynamicString("asdf"));
	}
}

FB_END_PACKAGE0()
