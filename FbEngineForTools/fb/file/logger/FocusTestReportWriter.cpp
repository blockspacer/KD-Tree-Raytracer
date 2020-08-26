#include "Precompiled.h"
#include "FocusTestReportWriter.h"

#include "fb/file/LineReader.h"
#include "fb/file/OutputFile.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"
#include "fb/string/Substring.h"

FB_PACKAGE1(file)

#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE

template <class StrClass>
static time_t stringToTime(StrClass &str)
{
	struct tm timeStruct;

	SizeType numberStartIndex = 0;
	timeStruct.tm_year = atoi(string::getSubstring(str, numberStartIndex, 4).getPointer()) - 1900;
	numberStartIndex += 4 + 1;
	timeStruct.tm_mon = atoi(string::getSubstring(str, numberStartIndex, 2).getPointer()) - 1;
	numberStartIndex += 2 + 1;
	timeStruct.tm_mday = atoi(string::getSubstring(str, numberStartIndex, 2).getPointer());

	numberStartIndex += 2 + 1;
	timeStruct.tm_hour = atoi(string::getSubstring(str, numberStartIndex, 2).getPointer());
	numberStartIndex += 2 + 1;
	timeStruct.tm_min = atoi(string::getSubstring(str, 11, 2).getPointer());
	numberStartIndex += 2 + 1;
	timeStruct.tm_sec = atoi(string::getSubstring(str, 13, 2).getPointer());

	time_t t = mktime(&timeStruct);
	return t;
}
#endif

FocusTestReportWriter::ParseInfo FocusTestReportWriter::readAndParseLine(file::LineReader *lineReader)
{
	ParseInfo info;
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	Vector<HeapString> line = lineReader->readSplitLine();
	if (line.getSize() == 4)
	{
		info.timestampString = line[0];
		info.eventName = line[1];
		info.positionString = line[2];
		HeapString moreInfo = line[3];
		info.message.clear();
		info.entityName.clear();

		info.timestamp = stringToTime(info.timestampString);

		FB_STATIC_CONST_STRING(entityString, "Entity: ");
		static const SizeType entityTextLength = entityString.getLength();

		if (moreInfo.doesContain(entityString))
		{
			SizeType commaPos = moreInfo.find(StringRef(", "), entityTextLength - 1);
			if (commaPos >= entityTextLength)
			{
				info.entityName = TempString().append(moreInfo.getPointer() + entityTextLength, commaPos - entityTextLength);
				info.message = TempString().append(moreInfo.getPointer() + commaPos + 2, moreInfo.getLength());
			}
		}
		else
		{
			info.message = moreInfo;
		}
		info.ok = true;
		return info;
	}
#endif
	info.ok = false;
	return info;
}

void FocusTestReportWriter::init(file::LineReader *lineReader, file::LineReader *highBandwidthLineReader)
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	this->reader = lineReader;
	this->highBandwidthReader = highBandwidthLineReader;
#endif
}

void FocusTestReportWriter::unInit()
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	reader = nullptr;
	highBandwidthReader = nullptr;
#endif
}

HeapString FocusTestReportWriter::getReport()
{
#if FB_FOCUS_TEST_LOGGING_ENABLED == FB_TRUE
	HeapString report;
	report += "Game report not implemented.\n\n";
	while (!this->reader->isEOF())
	{
		report += this->reader->readLine();
		report += "\n";
	}
	return report;
#else
	return HeapString::empty;
#endif
}

FB_END_PACKAGE1()
