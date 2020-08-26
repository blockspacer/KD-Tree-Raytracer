#include "Precompiled.h"
#include "Delegate.h"

#include <cstring>

FB_PACKAGE0()


DelegateCallData::DelegateCallData()
{
	clear();
}


void DelegateCallData::clear()
{
	memset(storage, 0, SizeInBytes);
}


bool DelegateCallData::operator==(const DelegateCallData &other) const
{
	return memcmp(storage, other.storage, SizeInBytes) == 0;
}


FB_END_PACKAGE0()
