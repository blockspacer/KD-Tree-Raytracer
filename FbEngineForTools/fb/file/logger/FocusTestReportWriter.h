#pragma once

#include "fb/file/LineReader.h"
#include "fb/string/DynamicString.h"
#include <time.h>

FB_DECLARE0(HeapString)

FB_PACKAGE1(file)

class FocusTestReportWriter
{
public:
	class ParseInfo
	{
	public:
		ParseInfo()
			: ok(false)
		{
		}

		// Parse succesfull
		bool ok;

		DynamicString timestampString; // 1
		time_t timestamp; // 1
		DynamicString eventName; // 2
		DynamicString positionString; // 3

		// 4
		DynamicString entityName;
		DynamicString message;
	};

protected:
	file::LineReader *reader;
	file::LineReader *highBandwidthReader;
	Vector<Vector<DynamicString>> data;

public:
	FocusTestReportWriter() {}
	virtual ~FocusTestReportWriter() {}

	// Always call init() before getReport()
	ParseInfo readAndParseLine(file::LineReader *reader);
	void init(file::LineReader *reader, file::LineReader *highBandwidthReader);
	virtual HeapString getReport();
	// Called after getReport()
	void unInit();
};

FB_END_PACKAGE1()
