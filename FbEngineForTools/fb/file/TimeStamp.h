#pragma once

FB_PACKAGE1(file)


// Low-level stuff, only defined on windows/linux.
// We want to make sure we don't compile code using these

typedef int TimeStamp;
typedef int64_t TimeStamp64;

bool fileIsZeroSize(const char *file);

TimeStamp getFileTimestamp(const StringRef &file);
TimeStamp getNewerTimeStamp(TimeStamp a, TimeStamp b);
bool isFileNewerThanFile(TimeStamp stamp, TimeStamp thanStamp);

TimeStamp64 getFileTimestamp64(const StringRef &file);
TimeStamp64 getNewerTimeStamp(TimeStamp64 a, TimeStamp64 b);
bool isFileNewerThanFile(TimeStamp64 stamp, TimeStamp64 thanStamp);

bool isFileNewerThanFile(const StringRef &file, const StringRef &thanFile);

void setFileTimestampToMatch(const StringRef &file, const StringRef &stampFile);
void setFileTimestamp(const StringRef &file, TimeStamp64 stamp);

TimeStamp64 getCurrentTimeStamp();

// returns unix epoch timestamp in milliseconds
uint64_t getTimestampFromFileTime(TimeStamp64 fileTime);
// takes unix epoch timestamp in milliseconds
TimeStamp64 getFileTimeFromTimestamp(uint64_t timestamp);

FB_END_PACKAGE1()
