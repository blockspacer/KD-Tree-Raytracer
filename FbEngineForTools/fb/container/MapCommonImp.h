/* This file contains code for common functions in various Map classes */

bool tryFind(const KeyType &key, ValueType &valueOut) const
{
	ConstIterator iter = find(key);
	if (iter != getEnd())
	{
		valueOut = iter.getValue();
		return true;
	}
	return false;
}

ValueType *tryFind(const KeyType &key)
{
	Iterator iter = find(key);
	if (iter != getEnd())
	{
		return &iter.getValue();
	}
	return nullptr;
}

const ValueType *tryFind(const KeyType &key) const
{
	ConstIterator iter = find(key);
	if (iter != getEnd())
	{
		return &iter.getValue();
	}
	return nullptr;
}
