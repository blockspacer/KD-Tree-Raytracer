#include "Precompiled.h"

#include "fb/lang/MemoryFunctions.h"

#pragma warning(push)
/* FIXME: warning C4191: 'type cast' : unsafe conversion from 'FARPROC' to 'fb::sys::util::`anonymous-namespace'::LPFN_GLPI'
 * I don't know whether these are valid or not.
 */
#pragma warning(disable: 4191)

#include "fb/sys/util/SystemInfoUtil.h"

#include "fb/lang/IncludeWindows.h"

FB_PACKAGE2(sys, util)

namespace {

	typedef BOOL (WINAPI *LPFN_GLPI)(
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
		PDWORD);

	DWORD countBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    

		for (DWORD i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest) ? 1 : 0);
			bitTest /= 2;
		}

		return bitSetCount;
	}
}

static SizeType getConcurrentThreadCountImpl(bool physical)
{
	// Information that we are interested of
	static DWORD processorCoreCount = 0;
	static DWORD logicalProcessorCount = 0;
	static DWORD physicalPocessorCount = 0;

	// Ignore hyper threading .. not worth it in practice
	if (processorCoreCount != 0)
		return processorCoreCount;

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = 0;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = 0;
	DWORD returnLength = 0;
	GetLogicalProcessorInformation(buffer, &returnLength);
	buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) lang::allocateMemory(returnLength);
	if (buffer)
	{
		GetLogicalProcessorInformation(buffer, &returnLength);

		ptr = buffer;
		DWORD byteOffset = 0;
		while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 
		{
			switch(ptr->Relationship) 
			{
				case RelationProcessorCore:
					++processorCoreCount;
					logicalProcessorCount += countBits(ptr->ProcessorMask);
					break;

				case RelationProcessorPackage:
					++physicalPocessorCount;
					break;
				default :
					break;
			}

			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		lang::freeMemory(buffer);
	}

	// Ignore hyper threading .. not worth it in practice
	if (physical)
		return processorCoreCount;
	else
		return logicalProcessorCount;

}

SizeType SystemInfoUtil::getConcurrentThreadCountHint()
{
	return getPhysicalThreadCount();
}

SizeType SystemInfoUtil::getPhysicalThreadCount()
{
	static SizeType count = getConcurrentThreadCountImpl(true);
	return count;
}

SizeType SystemInfoUtil::getLogicalThreadCount()
{
	static SizeType count = getConcurrentThreadCountImpl(false);
	return count;
}

const char *SystemInfoUtil::getPlatformTypeString()
{
	return "pc";
}

const char *SystemInfoUtil::getPlatformTypeStringUpperCase()
{
	return "PC";
}

const char *SystemInfoUtil::getPlatformString()
{
	return "windows";
}

const char *SystemInfoUtil::getPlatformStringUpperCase()
{
	return "WINDOWS";
}

uint32_t SystemInfoUtil::getOsVersion()
{
	static uint32_t osVersion = 0;
	if (osVersion > 0)
		return osVersion;

	OSVERSIONINFOEX osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	DWORDLONG dwlConditionMask = 0;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	bool is0602OrAbove = VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;
	if (is0602OrAbove)
	{
		osVersion = (6 << 8) + 2;
		return osVersion;
	}

	dwlConditionMask = 0;
	osvi.dwMinorVersion = 1;
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	bool is0601OrAbove = VerifyVersionInfo(&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask) != 0;
	if (is0601OrAbove)
	{
		osVersion = (6 << 8) + 1;
		return osVersion;
	}
	osVersion = (1 << 8) + 0;
	return osVersion;
}

FB_END_PACKAGE2()

#pragma warning(pop)
