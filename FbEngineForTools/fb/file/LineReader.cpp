#include "Precompiled.h"
#include "LineReader.h"

#include "fb/string/HeapString.h"

FB_PACKAGE1(file)

LineReader::LineReader()
{
}


LineReader::LineReader(const StringRef &fileName)
{
	open(fileName);
}


LineReader::~LineReader()
{
	close();
}


bool LineReader::open(const StringRef &fileName)
{
	close();
	file = fopen(fileName.getPointer(), "rb");
	return file != nullptr;
}


void LineReader::close()
{
	if (file != nullptr)
		fclose(file);

	file = nullptr;
}


HeapString LineReader::readLine()
{
	HeapString line;
	readAndAppendLine(line);
	return line;
}


bool LineReader::readLine(HeapString &resultOut)
{
	resultOut.clear();
	return readAndAppendLine(resultOut);
}


bool LineReader::readAndAppendLine(HeapString &resultOut)
{
	char c = char(fgetc(file));
	if (c == EOF)
		return false;

	while ((c != '\n' && c != EOF))
	{
		if (c != '\r' && c != '\0')
			resultOut += c;

		c = char(fgetc(file));
	}
	return true;
}


Vector<HeapString> LineReader::readSplitLine(char separator)
{
	Vector<HeapString> line;
	HeapString part;
	char c;
	while (true)
	{
		c = char(fgetc(file));
		if (c == '\r')
		{
			continue;
		}
		else if (c == '\n' || c == EOF)
		{
			break;
		}
		else if (c == separator)
		{
			line.pushBack(part);
			part = "";
		}
		else
		{
			part += c;
		}
	}
	line.pushBack(part);
	return line;
}

bool LineReader::isEOF()
{
	return file == nullptr || feof(file) != 0;
}

FB_END_PACKAGE1()
