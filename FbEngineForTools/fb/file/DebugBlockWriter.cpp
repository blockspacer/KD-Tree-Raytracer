#include "Precompiled.h"
#include "DebugBlockWriter.h"

#define FB_ENABLE_DEBUG_BLOCK_WRITER FB_TRUE

#if FB_ENABLE_DEBUG_BLOCK_WRITER == FB_TRUE

#include "fb/container/PodVector.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/RootPath.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/string/HeapString.h"
#include "fb/lang/time/SystemTime.h"
#include "fb/task/Scheduler.h"

#define FB_BLOCKWRITE_USE_C_API FB_TRUE

#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
	#include <stdio.h>
#endif

FB_PACKAGE1(file)

struct DebugBlockWriterData
{
	DebugBlockWriterData(unsigned int flags_, task::Scheduler* scheduler)
		: flags(flags_)
		, scheduler(scheduler)
	{
	}


	~DebugBlockWriterData()
	{
		{
			MutexGuard guard(mutex);
			fb_assert(queuedTasks <= 1);
		}
		close();
	}


	bool inImmediateMode() const
	{
		return (flags & DebugBlockWriter::UseImmediateMode) != 0;
	}

	bool dontModifyFileName() const
	{
		return (flags & DebugBlockWriter::DontModifyFilename) != 0;
	}

	bool hasFile() const
	{
		#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
			return f != 0;
		#endif
	}

	bool open(const StringRef &id, bool isFullPath, bool append)
	{
		MutexGuard guard(mutex);
		fb_assert(!hasFile());

		#if FB_BLOCKWRITE_USE_C_API == FB_TRUE

			const char *path = "";
			TempString filePath;

			if (isFullPath)
			{
				filePath << id;
			}
			else if (dontModifyFileName())
			{
				filePath << path << id;
			}
			else
			{
				filePath << path << id << "_";
				SystemTime::now().addFileTimeStampToString(filePath);
				filePath << ".txt";
			}

			/* Not all platforms have createPath() */
			TempString newPath;
			newPath.appendPathFromString(filePath);
			file::createPathIfMissing(newPath);
			f  = fopen(filePath.getPointer(), (append ? "ab" : "wb"));
		#endif

		return hasFile();
	}

	void write(const char *data, SizeType dataSize)
	{
		if (!hasFile())
			return;

		#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
			fwrite(data, dataSize, 1, f);
		#endif
	}

	void flush()
	{
		if (!hasFile())
			return;

		#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
			fflush(f);
		#endif
	}

	void close()
	{
		if (!hasFile())
			return;

		flush();

		#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
			fclose(f);
			f = 0;
		#endif
	}


	void queueWrite(const char *data, SizeType dataSize)
	{
		MutexGuard guard(mutex);
		// Nothing to be done
		if (!hasFile())
			return;

		fb_assert(!closeFlag && "We are closed already");
		if (inImmediateMode())
		{
			write(data, dataSize);
			return;
		}

		SizeType newDataIndex = queuedData.getSize();
		if (queuedData.getSize() < (1 << 24))
		{
			justWarnedAboutDataOverflow = false;
			queuedData.resize(queuedData.getSize() + dataSize);
			lang::MemCopy::copy(&queuedData[newDataIndex], data, dataSize);
		}
		else if (!justWarnedAboutDataOverflow)
		{
			/* Warn about lost messages (like anyone's going to read the huge log being dumped). */
			StringRef msg("\n\n*********\n\nDebugBlockWriter queue full. Losing messages.\n*********\n");
			dataSize = msg.getLength();
			queuedData.resize(queuedData.getSize() + dataSize);
			lang::MemCopy::copy(&queuedData[newDataIndex], msg.getPointer(), dataSize);
			justWarnedAboutDataOverflow = true;
		}

		/* Spawn task to handle actual writing */
		if (queuedTasks == 0)
			spawnTask();
	}

	void queueFlush()
	{
		MutexGuard guard(mutex);
		// Nothing to be done
		if (!hasFile())
			return;

		fb_assert(!closeFlag && "We are closed already");
		if (inImmediateMode())
		{
			flush();
			return;
		}

		// Already queued
		if (flushFlag)
			return;

		flushFlag = true;
		/* Spawn task to handle actual flush */
		if (queuedTasks == 0)
			spawnTask();
	}

	void queueClose()
	{
		// We can't have any tasks if we have no file
		if (!hasFile())
		{
			delete this;
			return;
		}

		if (inImmediateMode())
		{
			close();
			return;
		}

		MutexGuard guard(mutex);
		fb_assert(!closeFlag && "We are closed already");
		closeFlag = true;
		/* Spawn task to handle actual closing */
		if (queuedTasks == 0)
			spawnTask();
	}

	/* Defined later */
	void spawnTask();

	unsigned int flags;

#if FB_BLOCKWRITE_USE_C_API == FB_TRUE
	FILE *f = nullptr;
#endif

	// Threading interface
	Mutex writeMutex;
	Mutex mutex;
	task::Scheduler* scheduler;
	typedef PodVector<char> DataQueue;
	DataQueue queuedData;
	/* NX seems to spent lot of time doing memory allocations here. Let's help by reusing queue vectors */
	DataQueue tmpQueue;
	DynamicString filename;
	uint32_t queuedTasks = 0;
	bool openedWithFullPath = false;
	bool flushFlag = false;
	bool closeFlag = false;
	bool justWarnedAboutDataOverflow = false;
};


struct DebugBlockWriterTask : public fb::task::ISchedulerTask
{
	struct Params
	{
		DebugBlockWriterData *writer;

		FB_SCHEDULER_TASK_PARAM_TYPE();
	};


	const char *getStaticTaskNameString() const
	{
		return "DebugBlockWriterTask";
	}


	void run(const fb::task::SchedulerTaskData &data, fb::task::Scheduler &scheduler)
	{
		Params p;
		data.getParam(p);
		DebugBlockWriterData &writer = *p.writer;
		bool thingsToDo = false;
		do
		{
			/* Check if there's more to do after everything for this round has been done */
			thingsToDo = false;
			// First, make local copy of everything
			bool shouldFlush = false;
			bool shouldClose = false;
			bool shouldWrite = false;
			{
				MutexGuard guard(p.writer->mutex);
				/* DebugBlockWriterData.queuedTasks is basically a synchronization flag that makes sure we don't 
				 * run into race conditions when there are lot's of writes and a close call */
				fb_assert(writer.queuedTasks == 1);
				fb_assert(writer.tmpQueue.isEmpty());
				shouldClose = writer.closeFlag;
				shouldFlush = writer.flushFlag;
				/* Reset flushFlag, so it can be raised again before next batch of data  */
				writer.flushFlag = false;
				shouldWrite = !writer.queuedData.isEmpty();
				/* Pick up any data there is to write */
				if (shouldWrite)
				{
					writer.tmpQueue.swap(writer.queuedData);
					if (writer.flags & DebugBlockWriter::UseAutomagicalFlush)
						shouldFlush = true;
				}

				if (!shouldClose && !shouldFlush && !shouldWrite)
				{
					/* Nothing to do, just decrease task count and return */
					--writer.queuedTasks;
					break;
				}
			}

			thingsToDo = true;
			/* Do write related stuff */
			if (shouldFlush || shouldWrite)
			{
				MutexGuard guard(p.writer->writeMutex);
				if (shouldWrite)
				{
					// Do the work
					writer.write(&writer.tmpQueue.getFront(), writer.tmpQueue.getSize());
					writer.tmpQueue.clear();
				}
				if (shouldFlush)
					p.writer->flush();
			}
			else if (shouldClose)
			{
				/* Only do this when sure that no more stuff is coming in */
				delete p.writer;
				return;
			}
			else
			{
				fb_assert(0 && "Nothing to do? We should have quit earlier");
			}
		} while (thingsToDo);
	}
};

namespace
{
	static DebugBlockWriterTask writerTask;
}


void DebugBlockWriterData::spawnTask()
{
	fb_assert(scheduler != nullptr);

	DebugBlockWriterTask::Params p;
	p.writer = this;

	/* We don't actually have synchronization in place to handle more than one concurrent tasks. Instead we loop in 
	 * running task until there's nothing to do, then exit and allow new task to be inserted if necessary */
	fb_assert(queuedTasks == 0);
	++queuedTasks;
	scheduler->addTask(fb::task::Scheduler::TaskGroupBackground, fb::task::Scheduler::TaskPriorityNormal, &writerTask, fb::task::SchedulerTaskData(p));
}


DebugBlockWriter::DebugBlockWriter(task::Scheduler* scheduler, unsigned int flags)
	: scheduler(scheduler)
	, flags(flags)
{
	fb_assert(((flags & DebugBlockWriter::UseImmediateMode) != 0 || scheduler != nullptr) && "DebugBlockWriter needs to have a scheduler or work in immediate mode");
}


DebugBlockWriter::~DebugBlockWriter()
{
	if (data != nullptr)
	{
		data->queueClose();
		data = nullptr;
	}
}


bool DebugBlockWriter::open(const StringRef &id)
{
	if (data == nullptr)
		data = new DebugBlockWriterData(flags, scheduler);

	data->filename = id;
	data->openedWithFullPath = false;
	return data->open(id, false, false);
}


bool DebugBlockWriter::openFile(const StringRef &fullPathToFile)
{
	if (data == nullptr)
		data = new DebugBlockWriterData(flags, scheduler);

	data->filename = fullPathToFile;
	data->openedWithFullPath = true;
	return data->open(fullPathToFile, true, false);
}


bool DebugBlockWriter::appendFile(const StringRef &fullPathToFile)
{
	if (data == nullptr)
		data = new DebugBlockWriterData(flags, scheduler);

	data->filename = fullPathToFile;
	data->openedWithFullPath = true;
	return data->open(fullPathToFile, true, true);
}


void DebugBlockWriter::close()
{
	/* Allow calling close multiple times */
	if (data != nullptr)
	{
		data->queueClose();
		data = nullptr;
	}
}


void DebugBlockWriter::write(const char *dataPtr, SizeType dataSize)
{
	fb_assert(dataPtr && dataSize);
	if (data != nullptr && dataPtr != nullptr && dataSize > 0)
		data->queueWrite(dataPtr, dataSize);
}


void DebugBlockWriter::write(const StringRef &str)
{
	if (data != nullptr && !str.isEmpty())
		data->queueWrite(str.getPointer(), str.getLength());
}

void DebugBlockWriter::flush()
{
	if (data == nullptr || data->flags & UseAutomagicalFlush)
		return;

	data->queueFlush();
}


const DynamicString &DebugBlockWriter::getOpenFilename() const
{
	return data != nullptr ? data->filename : DynamicString::empty;
}


bool DebugBlockWriter::isOpenedWithFullPath() const
{
	return data != nullptr ? data->openedWithFullPath : false;
}


#else

FB_PACKAGE1(file)

// dummy writer
DebugBlockWriter::DebugBlockWriter(task::Scheduler* scheduler, unsigned int flags) { }
DebugBlockWriter::~DebugBlockWriter() { }
bool DebugBlockWriter::open(const StringRef &id) { return false; }
bool DebugBlockWriter::openFile(const StringRef &fullPathToFile) { return false; }
bool DebugBlockWriter::appendFile(const StringRef &fullPathToFile) { return false; }
void DebugBlockWriter::close() { }
const DynamicString &DebugBlockWriter::getOpenFilename() const { return DynamicString::empty; }
bool DebugBlockWriter::isOpenedWithFullPath() const { return false; }
void DebugBlockWriter::write(const char *, SizeType) { }
void DebugBlockWriter::write(const StringRef &) { }
void DebugBlockWriter::flush() { }

#endif

FB_END_PACKAGE1()
