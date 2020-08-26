#pragma once

#include "fb/lang/IntTypes.h"

FB_PACKAGE1(file)

class InputFileAsync
{
public:
	typedef size_t SizeType;

public:
	InputFileAsync();
	~InputFileAsync();

	bool open(const StringRef &filename);
	void close();

	SizeType getSize() const;

	// This interface is NOT properly designed. 
	// It should prolly support multiple reads and perhaps non-polling way of notifying completion.

	// Start a reading operation. Returns false in case no operation was started. 
	// For the time being, only 1 read allowed in queue
	bool startReadingWithOffset(SizeType offset, void *buffer, SizeType size);

	// Returns true if reads are still pending (and thus further read operations are not possible)
	bool anyReadsPending() const;
	// Check if the previous read was succesfull (at least some data was returned). 
	// Return values are defined IFF anyReadsPending() returns false AND there has been a read operation since opening the file.
	bool getReadStatus(SizeType &numberOfBytesRead) const;
	// Block until there are at least some results
	void blockUntilReady();

private:
	void updateResults(bool block);

	void *fileHandle;
	char *overlappedBuffer;
	bool readPending;
	int32_t readBytes;
};

FB_END_PACKAGE1()
