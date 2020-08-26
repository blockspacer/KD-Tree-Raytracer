#pragma once

FB_PACKAGE1(file)

struct FileSortInfo
{
	DynamicString fileName;
	void *userdata = nullptr;
	uint32_t packagedata = 0;
	int typePriority = 0;

	FileSortInfo()
	{
	}
};

FB_END_PACKAGE1()
