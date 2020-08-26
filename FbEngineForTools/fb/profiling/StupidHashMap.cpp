#include "Precompiled.h"
#include "StupidHashMap.h"

FB_PACKAGE0()

void testStupidHashMap()
{
	struct Asdf
	{
		uint64_t asdf = 0;
	};

	typedef profiling::StupidHashMap<Asdf, 10> MapType;

	Asdf stackMapValueBuffer[MapType::BufferSize];
	MapType::MiniHashType stackMapOccupancyBuffer[MapType::HashBufferSize];
	MapType stackMap(stackMapValueBuffer, stackMapOccupancyBuffer);

	uint64_t hash = 0;
	if (stackMap.canSet(hash))
		stackMap.set(hash).asdf = 1;

	if (stackMap.canGet(hash))
		stackMap.get(hash);
}

FB_END_PACKAGE0()
