#pragma once

FB_PACKAGE0()

namespace
{
template<typename PairType, typename StringType>
struct StringSortFunctor
{
	bool operator()(const PairType &a, const StringType &b)
	{
		return a.key < b.getPointer();
	}
	bool operator()(const StringType &a, const PairType &b)
	{
		 return a < b.key;
	}
	bool operator()(const StringType &a, const StringType &b)
	{
		return a < b;
	}
	bool operator()(const PairType &a, const PairType &b)
	{
		return a.key < b.key;
	}
};
}

// Find index, linear map (only)

template<typename KeyType, typename ValueType>
SizeType StringMapFind::findIndex(const LinearMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	typedef container::MapPair<KeyType, ValueType> PairType;
	typedef fb::Vector<PairType> ImpVecType;

	typename ImpVecType::ConstIterator begin = map.getBegin().it;
	typename ImpVecType::ConstIterator end = map.getEnd().it;
	typename ImpVecType::ConstIterator result = algorithm::binaryFind(begin, end, keyString, StringSortFunctor<PairType, StringRef>());
	return (SizeType)(result - begin);
}
template<typename KeyType, typename ValueType>
SizeType StringMapFind::findIndex(const LinearMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.findIndex(key);
}

// Find iterator

// LinearMap
template<typename KeyType, typename ValueType>
typename LinearMap<KeyType, ValueType>::Iterator StringMapFind::findIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map.getBegin().it + findIndex(map, keyString);
}
template<typename KeyType, typename ValueType>
typename LinearMap<KeyType, ValueType>::Iterator StringMapFind::findIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.find(key);
}
template<typename KeyType, typename ValueType>
typename LinearMap<KeyType, ValueType>::ConstIterator StringMapFind::findIterator(const LinearMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map.getBegin().it + findIndex(map, keyString);
}
template<typename KeyType, typename ValueType>
typename LinearMap<KeyType, ValueType>::ConstIterator StringMapFind::findIterator(const LinearMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.find(key);
}

// LinearHashMap
template<typename KeyType, typename ValueType>
typename LinearHashMap<KeyType, ValueType>::Iterator StringMapFind::findIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map.find(KeyType(keyString));
}
template<typename KeyType, typename ValueType>
typename LinearHashMap<KeyType, ValueType>::Iterator StringMapFind::findIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.find(key);
}
template<typename KeyType, typename ValueType>
typename LinearHashMap<KeyType, ValueType>::ConstIterator StringMapFind::findIterator(const LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map.find(KeyType(keyString));
}
template<typename KeyType, typename ValueType>
typename LinearHashMap<KeyType, ValueType>::ConstIterator StringMapFind::findIterator(const LinearHashMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.find(key);
}

// Find or insert (with and without default value, functions similarly to operator[])

// LinearMap
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	typename LinearMap<KeyType, ValueType>::Iterator it = findIterator(map, keyString);
	if (it != map.getEnd())
		return it.getValue();

	return map.insert(KeyType(keyString), ValueType()).first.getValue();
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map[key];
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue)
{
	typename LinearMap<KeyType, ValueType>::Iterator it = findIterator(map, keyString);
	if (it != map.getEnd())
		return it.getValue();

	return map.insert(KeyType(keyString), defaultValue).first.getValue();
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue)
{
	return map.insert(key, defaultValue).first.getValue();
}

// LinearHashMap
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map[KeyType(keyString)];
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearHashMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map[key];
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue)
{
	typename LinearHashMap<KeyType, ValueType>::Iterator it = findIterator(map, keyString);
	if (it != map.getEnd())
		return it.getValue();

	return map.insert(KeyType(keyString), defaultValue).first.getValue();
}
template<typename KeyType, typename ValueType>
ValueType &StringMapFind::findOrInsert(LinearHashMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue)
{
	return map.insert(key, defaultValue).first.getValue();
}

// Find or insert (with and without default value, functions similarly to insert)

// LinearMap
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	typedef StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > Pair;
	typename LinearMap<KeyType, ValueType>::Iterator it = findIterator(map, keyString);
	if (it != map.getEnd())
		return Pair(it, false);

	return map.insert(KeyType(keyString), ValueType());
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key)
{
	typedef StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > Pair;
	typename LinearMap<KeyType, ValueType>::Iterator it = findIterator(map, key);
	if (it != map.getEnd())
		return Pair(it, false);

	return map.insert(key, ValueType());
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue)
{
	typedef StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > Pair;
	typename LinearMap<KeyType, ValueType>::Iterator it = findIterator(map, keyString);
	if (it != map.getEnd())
		return Pair(it, false);

	return map.insert(KeyType(keyString), defaultValue);
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue)
{
	return map.insert(key, defaultValue);
}

// LinearHashMap
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearHashMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString)
{
	return map.insert(KeyType(keyString), ValueType());
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearHashMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key)
{
	return map.insert(key, ValueType());
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearHashMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const StringRef &keyString, const ValueType &defaultValue)
{
	return map.insert(KeyType(keyString), defaultValue);
}
template<typename KeyType, typename ValueType>
StringMapFind::InsertPair<LinearHashMap<KeyType, ValueType> > StringMapFind::findOrInsertIterator(LinearHashMap<KeyType, ValueType> &map, const KeyType &key, const ValueType &defaultValue)
{
	return map.insert(key, defaultValue);
}

FB_END_PACKAGE0()
