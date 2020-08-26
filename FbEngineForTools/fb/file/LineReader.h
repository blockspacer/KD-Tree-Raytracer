#pragma once

#include "fb/container/Vector.h"
#include "fb/string/HeapString.h"

#include <stdio.h>

FB_DECLARE0(HeapString)

FB_PACKAGE1(file)

/* LineReader reads a UTF-8 (or similar enough 8 bit character set) file one line at a time */

class LineReader
{
	FILE *file = nullptr;
public:
	LineReader();
	LineReader(const StringRef &fileName);
	~LineReader();

	bool open(const StringRef &fileName);
	void close();

	/* Reads next line from file and returns it. Returns empty string if at end of file */
	HeapString readLine();
	/* Reads next line from file and stores it to given string. Returns true if successful, false if at end of file */
	bool readLine(HeapString &resultOut);
	/* Reads next line from file and appends it to given string. Returns true if successful, false if at end of file */
	bool readAndAppendLine(HeapString &resultOut);

	/* Returns line in parts splitted by separator character (note that using SplitString is probably more efficient.
	 * FIXME: refactor this to use SplitString */
	Vector<HeapString> readSplitLine(char separator = '\t');

	/* Returns true if reading is at the end of file */
	bool isEOF();
};

FB_END_PACKAGE1()
