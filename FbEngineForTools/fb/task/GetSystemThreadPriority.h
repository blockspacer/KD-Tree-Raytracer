#pragma once

FB_PACKAGE1(task)

struct GetSystemThreadPriority
{
	// Convert priority offset to system priority level. 0 = normal, -1 below normal, +1 above normal
	static uint32_t getSystemThreadPriority(int priorityOffset);
};

FB_END_PACKAGE1()
