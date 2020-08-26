#include "Precompiled.h"
#include "StupidRingBuffer.h"

FB_PACKAGE0()

void testStupidRingBuffer()
{
	struct Asdf
	{
		uint64_t asdf = 0;
	};


	Asdf buffer[5];
	profiling::StupidRingBuffer<Asdf, 5> asdf(buffer);
	asdf.pushBack(Asdf());
	for (Asdf a : asdf)
	{
		a.asdf = 0;
	}
	asdf.popFront();
}


FB_END_PACKAGE0()

