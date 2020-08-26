#pragma once

#include "fb/lang/ICharacterOutputReceiver.h"
#include "fb/string/DynamicString.h"

FB_DECLARE(task, Scheduler)

FB_PACKAGE1(file)

/**
 * Class which wraps the task of writing debug output to a given file. This doesn't work with standard file API on 
 * console platforms.
 *
 * Note: 
 * Use identifiers, don't give complete filenames to this class!
 * For example writer->open("blockmemstats");
 * This class will add required paths and time stamps to make sure you don't overwrite files.
 * New: writer->openFile and writer->appendFile take full paths.
 *
 * As the name implies, nothing is ever written in final release builds.
 *
 * Writes, flushes and deletion are done as background tasks.
 * Flag UseImmediateMode of course causes everything to happen right on call.
 * Open is done in current thread and may take quite a bit of time on some platforms.

 * DebugBlockWriter is thread safe for writing things, but do note that if one thread is calling write while another is 
 * calling open or close, things will go bad.
 *
 */

struct DebugBlockWriterData;
class DebugBlockWriter : public fb::lang::ICharacterOutputReceiver
{
	DebugBlockWriterData *data = nullptr;
	task::Scheduler* scheduler = nullptr;
	unsigned int flags = 0;

	// Not implemented
	DebugBlockWriter(const DebugBlockWriter &) = delete;
	void operator = (const DebugBlockWriter &) = delete;

public:
	enum Flags
	{
		/* Flush after each write */
		UseAutomagicalFlush = 1,
		/* Write everything immediately (won't need scheduler) */
		UseImmediateMode = 2,
		/* Prevent appending "_<timestamp>.txt" to filename */
		DontModifyFilename = 4
	};

	explicit DebugBlockWriter(task::Scheduler* scheduler, unsigned int flags);
	~DebugBlockWriter();

	/** open works at it used to, never give it a full path, only id. While openFile requires a full path */
	bool open(const StringRef &id);
	bool openFile(const StringRef &fullPathToFile);
	bool appendFile(const StringRef &fullPathToFile);
	void close();

	// Returns full path if isOpenedWithFullPath() == true, else path id as in ->open(id)
	const DynamicString &getOpenFilename() const;
	bool isOpenedWithFullPath() const;

	/* write */
	virtual void write(const char *data, SizeType dataSize);
	virtual void write(const StringRef &str);
	void flush();
};

FB_END_PACKAGE1()
