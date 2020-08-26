#pragma once

#include "fb/container/LinearMap.h"
#include "fb/container/LinearHashMap.h"
#include "fb/container/Pair.h"
#include "fb/string/StringRef.h"

FB_PACKAGE0()

class StringMapFind
{
public:
#define FB_TEMPL template<typename KeyType, typename ValueType>

	template<typename MapType>
	using InsertPair = Pair<typename MapType::Iterator, bool>;

	FB_TEMPL static SizeType findIndex(const LinearMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static SizeType findIndex(const LinearMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static typename LinearMap<KeyType, ValueType>::Iterator findIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static typename LinearMap<KeyType, ValueType>::Iterator findIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static typename LinearMap<KeyType, ValueType>::ConstIterator findIterator(const LinearMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static typename LinearMap<KeyType, ValueType>::ConstIterator findIterator(const LinearMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static typename LinearHashMap<KeyType, ValueType>::Iterator findIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static typename LinearHashMap<KeyType, ValueType>::Iterator findIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static typename LinearHashMap<KeyType, ValueType>::ConstIterator findIterator(const LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static typename LinearHashMap<KeyType, ValueType>::ConstIterator findIterator(const LinearHashMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static ValueType &findOrInsert(LinearMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static ValueType &findOrInsert(LinearMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static ValueType &findOrInsert(LinearMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue);
	FB_TEMPL static ValueType &findOrInsert(LinearMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue);
	FB_TEMPL static ValueType &findOrInsert(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static ValueType &findOrInsert(LinearHashMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static ValueType &findOrInsert(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue);
	FB_TEMPL static ValueType &findOrInsert(LinearHashMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue);
	FB_TEMPL static InsertPair<LinearMap<KeyType, ValueType> > findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static InsertPair<LinearMap<KeyType, ValueType> > findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static InsertPair<LinearMap<KeyType, ValueType> > findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue);
	FB_TEMPL static InsertPair<LinearMap<KeyType, ValueType> > findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue);
	FB_TEMPL static InsertPair<LinearHashMap<KeyType, ValueType> > findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString);
	FB_TEMPL static InsertPair<LinearHashMap<KeyType, ValueType> > findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key);
	FB_TEMPL static InsertPair<LinearHashMap<KeyType, ValueType> > findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue);
	FB_TEMPL static InsertPair<LinearHashMap<KeyType, ValueType> > findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue);

#undef FB_TEMPL
};

FB_END_PACKAGE0()

#include "fb/container/StringMapFindInline.h"
