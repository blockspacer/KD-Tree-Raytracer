#include "Precompiled.h"
#include "MysteryFilePackage.h"

#include "fb/algorithm/Sort.h"
#include "fb/file/FileAllocator.h"
#include "fb/file/FileSortInfo.h"
#include "fb/file/InputFile.h"
#include "fb/file/MemoryMappedFile.h"
#include "fb/file/RootPath.h"
#include "fb/lang/AlignmentFunctions.h"
#include "fb/lang/ByteOrderSwap.h"
#include "fb/lang/CallStack.h"
#include "fb/lang/FastCopy.h"
#include "fb/lang/thread/MutexGuard.h"
#include "fb/lang/thread/SemaphoreGuard.h"
#include "fb/lang/time/ScopedTimer.h"
#include "fb/stream/InputStream.h"
#include "fb/string/HeapString.h"
#include "fb/string/StaticString.h"
#include "fb/string/StringHash.h"
#include "fb/util/BufferCompressor.h"
#include "fb/util/Checksum.h"

#if FB_TRACK_UNUSED_FILES == FB_TRUE
#include "fb/file/DebugBlockWriter.h"
#endif

static fb::SizeType maxNumConcurrentAccesses = 32;
#define FB_ENABLE_CRYPT_SUPPORT FB_TRUE
#define FB_USE_MEMORY_MAPPED_FILE_SUPPORT FB_FALSE
#include "fb/lang/IncludeWindows.h"

#define FB_HACK_FILE_STATS FB_FALSE

#if FB_HACK_FILE_STATS == FB_TRUE
	uint32_t fileReadAmount = 0;
	uint32_t fileReadBytes = 0;
#endif


FB_PACKAGE2(file, package)

Semaphore &MysteryFilePackage::getFileSemaphore()
{
	static Semaphore semaphore(maxNumConcurrentAccesses);
	return semaphore;
}


namespace {

	struct FileData
	{
		StaticString fileName;
		uint32_t offset = 0;
		uint32_t uncompressedSize = 0;
		uint32_t compressedSize = 0;
		uint32_t checksum = 0;
		uint8_t flags = 0;
		#if FB_TRACK_UNUSED_FILES == FB_TRUE
			mutable bool hasBeenLoaded = false;
		#endif

		// Raw file data
		FileBufferPointer buffer;

		FileData()
		{
		}
	};

	struct FileSorter
	{
		bool operator () (const FileSortInfo &a, const FileSortInfo &b) const
		{
			uint32_t aData = a.packagedata;
			uint32_t bData = b.packagedata;
			int aType = a.typePriority;
			int bType = b.typePriority;

			if (aType != bType)
				return aType > bType;

			return aData < bData;
		}
	};

	FB_STACK_SET_CLASS(MysterFilePackage);
}

class MysteryFilePackage::Impl
{
public:
	Impl(MysteryFilePackage *self, const StringRef &fileName, bool optional, ErrorFunc errorFunc, bool useCryptStuff, bool memoryResident)
		: self(self)
		, errorFunc(errorFunc)
		, useCryptStuff(useCryptStuff)
		, memoryResident(memoryResident)
	{
		FB_ZONE("MysteryFilePackage::Impl::Impl()");
		ScopedTimer timer;
		archiveFileName << file::RootPath::get() << fileName;

		memoryMappedFileReadPtr = nullptr;
		#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
			if (mFile.open(archiveFileName))
				memoryMappedFileReadPtr = mFile.getData();
		#endif

		if (!memoryMappedFileReadPtr)
		{
			file = new file::InputFile();
			if (!file->open(archiveFileName))
			{
				if (!optional)
					errorFunc(archiveFileName.getPointer(), "Failed to open package file.");

				delete file;
				file = nullptr;
				return;
			}
		}
		// Read TOC
		int32_t version = 0;
		SizeType fileAmount = 0;
		SizeType headerSize = 0;

		static const SizeType startSize = 3 * sizeof(uint32_t);
		const char *dataPointer = nullptr;
		char start[startSize] = { 0 };
		if (memoryMappedFileReadPtr)
		{
			dataPointer = memoryMappedFileReadPtr;
			memoryMappedFileReadPtr += startSize;
		}
		else
		{
			dataPointer = start;
			file->readData(start, startSize);
		}

		stream::InputStream<lang::LittleEndian> startStream(dataPointer, startSize);
		startStream.read(version);
		startStream.read(fileAmount);
		startStream.read(headerSize);

		if (version != 3)
		{
			errorFunc(archiveFileName.getPointer(), "Package version number is invalid.");

			cleanup();
			return;
		}
		if (fileAmount <= 0 || headerSize <= 0)
		{
			errorFunc(archiveFileName.getPointer(), "Package file amount is zero or header size is zero.");

			cleanup();
			return;
		}

		PodVector<char> headerBuffer;
		const char *headerPtr = nullptr;
		if (memoryMappedFileReadPtr)
		{
			headerPtr = memoryMappedFileReadPtr;
			memoryMappedFileReadPtr += headerSize;
		}
		else
		{
			headerBuffer.resize(headerSize);
			file->readData(&headerBuffer[0], headerSize);
			headerPtr = &headerBuffer.getFront();
		}
		stream::InputStream<lang::LittleEndian> headerStream(headerPtr, headerSize);

		#if FB_ENABLE_CRYPT_SUPPORT == FB_TRUE
			unsigned char *xorTable = 0;
			int tableLength = 0;
			xorTable = MysteryFilePackage::getXorTable(tableLength);
		#endif

		fileHeaders.resize(fileAmount);
		uint32_t fileDataSize = 0;
		for (SizeType i = 0; i < fileAmount; ++i)
		{
			FileData &d = fileHeaders[i];

			#if FB_ENABLE_CRYPT_SUPPORT == FB_TRUE
				if (useCryptStuff)
				{
					uint16_t fileLength = 0;
					headerStream.read(fileLength);
					fb_assert(fileLength < MysteryFilePackage::maxFileLength);

					CacheHeapString<MysteryFilePackage::maxFileLength> str;
					str.resizeAndFill(fileLength, 0);
					headerStream.read(&str[0], fileLength);

					for (SizeType j = 0; j < fileLength; ++j)
					{
						unsigned char v = (unsigned char)str[j];
						unsigned char key  = xorTable[j % tableLength];
						v ^= (unsigned char)((key + (j * 2)) & 0xff);
						str[j] = char(v);
					}

					d.fileName = StaticString(str);
				}
				else
				{
					const char *fn = 0;
					headerStream.readString(&fn);
					d.fileName = StaticString::createFromConstChar(fn);
				}
			#else
				const char *fn = 0;
				headerStream.readString(&fn);
				d.fileName = StaticString(fn);
			#endif

			headerStream.read(d.offset);
			headerStream.read(d.flags);
			headerStream.read(d.uncompressedSize);
			headerStream.read(d.compressedSize);
			headerStream.read(d.checksum);

			fileDataSize += d.compressedSize;

			// Convert to actual file offset
			d.offset += startSize + headerSize;
			// And store lookup
			fileMap[d.fileName] = &d;
		}

		if (memoryMappedFileReadPtr)
		{
			#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
				memoryMappedFileReadPtr = mFile.getData();
				memoryMappedFileReadSize = mFile.getSize();
			#endif
			/* Memory mapped files are basically memoryResident, so no need to do manual copying */
			return;
		}

		/* If we are not a memory package, that was that */
		if (!memoryResident)
			return;

		// Read in the data for every single file
		// Chunk based reading for optimal performance
		// However, to really be efficient we'd need to do async streaming in of the next chunk while
		// processing the current chunk.
		// I'd guess otherwise we don't get optimal reading performance from the optical disk ...
		/* However, since memory file packages are supposed to be small, it's unlikely we'll be reading that many
		 * chunks anyway */
		SizeType numReads = 1;
		char *readPtr = new char[fileReadChunkSize];

		// Init read buffer
		if (fileDataSize < fileReadChunkSize)
			file->readData(readPtr, fileDataSize);
		else
			file->readData(readPtr, fileReadChunkSize);

		const uint32_t maxFilePos = fileDataSize + startSize + headerSize;
		uint32_t readOffset = 0;
		for (SizeType i = 0; i < fileAmount; ++i)
		{
			FileData &d = fileHeaders[i];
			char *rawPtr = new char[d.compressedSize];
			d.buffer.reset(new FileBuffer(rawPtr));

			uint32_t readCurrentBytes = 0;
			while (readCurrentBytes < d.compressedSize)
			{
				uint32_t bytesBuffered = fileReadChunkSize - readOffset;
				uint32_t bytesNeeded = d.compressedSize - readCurrentBytes;

				uint32_t copySize = (bytesNeeded >= bytesBuffered) ? bytesBuffered : bytesNeeded;
				fb_assert(readCurrentBytes + copySize <= d.compressedSize);
				fb_assert(readOffset + copySize <= fileReadChunkSize);

				if (copySize)
				{
					lang::fastSmallMemoryCopy(rawPtr + readCurrentBytes, readPtr + readOffset, copySize);
					readCurrentBytes += copySize;
					readOffset += copySize;
				}

				if (readOffset >= fileReadChunkSize)
				{
					uint32_t readSize = fileReadChunkSize;
					uint32_t currentFilePos = d.offset + readCurrentBytes;
					if (currentFilePos + readSize >= maxFilePos)
						readSize = maxFilePos - currentFilePos;

					//FB_PRINTF("Reading %u bytes of more data\n", readSize);
					++numReads;
					file->readData(readPtr, readSize);
					readOffset = 0;
				}
				else
				{
					// If we don't read in another chunk, we actually have data left to process
					// Otherwise we are stuck in this loop ..
					fb_assert(copySize);
					fb_assert(bytesBuffered);
				}
			}
		}

		delete[] readPtr;

		/* Print some stats */
		Time duration = timer.getTime();
		TempString msg;
		uint64_t fileSize = file->getSize();
		uint64_t averageSpeed = fileSize * 1000 / lang::max(int64_t(1), duration.getMilliseconds());
		msg << "Read " << fileAmount << " files, " << (fileSize >> 10) << " kB in " << duration.getMilliseconds() <<
			" ms (" << (averageSpeed >> 10) << " kB/s) using " << numReads << " chunks, from " << archiveFileName << ".";
		FB_LOG_INFO(msg);

	}

	~Impl()
	{
		cleanup();
	}

	void cleanup()
	{
		FB_ZONE("MysteryFilePackage::Impl::cleanup");
		if (file)
		{
			delete file;
			file = nullptr;
		}
	}

	File openFile(const DynamicString &fileName)
	{
		FB_ZONE("MysteryFilePackage::Impl::openFile");
		FB_STACK_METHOD();

		FileMap::Iterator it = fileMap.find(fileName);
		if (it == fileMap.getEnd())
			return File(getEmptyBuffer(), 0);

		FileData *fileData = it.getValue();
		#if FB_TRACK_UNUSED_FILES == FB_TRUE
			fileData->hasBeenLoaded = true;
		#endif

		// ignore empty files
		if (fileData->uncompressedSize == 0)
			return File(getEmptyBuffer(), 0);

		/* Allocate memory for uncompressed buffer only if file is compressed in the first place */
		/* Allocate possibly needed uncompressed buffer first, as that way our heap gets less fragmented after we are 
		 * done with this file (buffer that's not freed will be at the bottom of the heap) */
		bool fileIsCompressed = (fileData->flags & 1) == 0;
		uint32_t compressedSize = fileData->compressedSize;
		uint32_t uncompressedSize = fileData->uncompressedSize;
		FileBufferPointer uncompressedPointer = getEmptyBuffer();
		char *uncompressedBuffer = nullptr;
		if (fileIsCompressed)
		{
			uncompressedPointer = getFileBuffer(uncompressedSize, fileName.getPointer());
			uncompressedBuffer = uncompressedPointer.getMutable();
		}
		/* Check if we have the file loaded already (memory resident package). This could be based on memoryResident 
		 * bool too, but in case we want to do e.g. some per extension memory residence later, why not just check the 
		 * buffer */

		#ifdef FB_ENABLE_REGRESSION_VERBOSE_MODE
			TempString msg;
			msg << "Loading " << fileName << "\n";
			msg << "  Compressed size: " << compressedSize << ", uncompressed: " << uncompressedSize << "\n";
			msg << "  Offset: " << fileData->offset << ", memory resident: " << (fileData->buffer.isValid() ? "yes" : "no");
			FB_FINAL_LOG_INFO(msg);
		#endif

		FileBufferPointer compressedPointer = fileData->buffer;
		char *compressedBuffer = fileData->buffer.getMutable();
		if (!compressedBuffer)
		{
			/* Read the file */
			compressedPointer = getFileBuffer(compressedSize, fileName.getPointer());
			compressedBuffer = compressedPointer.getMutable();
			FB_STACK_CUSTOM("Filesystem calls");

			if (memoryMappedFileReadPtr)
			{
				/* FIXME: If we had non-destructive FileBufferPointer (and FileBuffer), this could be done without copying */
				fb_assert(fileData->offset + compressedSize <= memoryMappedFileReadSize && "Out of bounds. Backing file is not that large");
				lang::MemCopy::copy(compressedBuffer, memoryMappedFileReadPtr + fileData->offset, compressedSize);
			}
			else
			{
				file->readDataWithOffset(fileData->offset, compressedBuffer, compressedSize);
			}
			#if FB_HACK_FILE_STATS == FB_TRUE
				++fileReadAmount;
				fileReadBytes += compressedSize;
			#endif

			#ifdef FB_ENABLE_REGRESSION_MODE
				// Very expensive, doesn't even try to be fast
				char *tmpBuffer = new char[compressedSize];
				if (memoryMappedFileReadPtr)
					lang::MemCopy::copy(tmpBuffer, memoryMappedFileReadPtr + fileData->offset, compressedSize);
				else
					file->readDataWithOffset(fileData->offset, tmpBuffer, compressedSize);

				uint32_t firstErrorIndex = 0xFFFFFFFF;
				uint32_t lastErrorIndex = 0;
				uint32_t errorAmount = 0;
				for (uint32_t i = 0; i < compressedSize; ++i)
				{
					if (compressedBuffer[i] != tmpBuffer[i])
					{
						firstErrorIndex = firstErrorIndex < i ? firstErrorIndex : i;
						lastErrorIndex = lastErrorIndex > i ? lastErrorIndex : i;
						++errorAmount;
					}
				}
				delete[] tmpBuffer;
				if (errorAmount)
					FB_FINAL_LOG_ERROR("  File validation errors %d (first %u, last %u)", errorAmount, firstErrorIndex, lastErrorIndex);
			#endif
		}

		#if FB_ENABLE_CRYPT_SUPPORT == FB_TRUE
			if (useCryptStuff)
			{
				FB_STACK_CUSTOM("Decrypt");

				unsigned char keyFudge = 0;
				if (MysteryFilePackage::shouldCrypt(fileName, keyFudge))
				{
					unsigned char *xorTable = 0;
					int tableLength = 0;
					xorTable = MysteryFilePackage::getXorTable(tableLength);

					for (uint32_t i = 0; i < compressedSize; ++i)
					{
						unsigned char v = (unsigned char)(compressedBuffer[i]);
						unsigned char key  = xorTable[i % tableLength];

						// Double the XOR! Double the safety!
						v ^= keyFudge;
						v ^= (unsigned char)((key + (i * 4)) & 0xff);

						compressedBuffer[i] = (char) v;
					}
				}
			}
		#endif

		// Checksum
		{
			FB_STACK_CUSTOM("Checksum");
			uint32_t checksum = util::calculateChecksumFast(compressedBuffer, compressedSize);
			if (checksum != fileData->checksum)
			{
				TempString msg;
				msg << "Checksum failed for " << fileName << "\n";
				msg << "  current checksum: ";
				msg.appendHexNumber(checksum, true) << "\n";
				msg << "  expected checksum: ";
				msg.appendHexNumber(fileData->checksum, true) << "\n";
				msg << "  offset: ";
				msg.appendHexNumber(fileData->offset, true) << "\n";
				msg << "  uncompressedSize: ";
				msg.appendHexNumber(uncompressedSize, true) << "\n";
				msg << "  compressedSize: ";
				msg.appendHexNumber(compressedSize, true);
				FB_FINAL_LOG_ERROR(msg);
				errorFunc(archiveFileName.getPointer(), FB_MSG("Checksum failed for file: ", fileName).getPointer());
				return File(getEmptyBuffer(), 0);
			}
		}

		#if FB_HACK_FILE_STATS == FB_TRUE
			FB_FINAL_LOG_INFO(FB_FMT("Files read %u, Megs read %u.", fileReadAmount, fileReadBytes/1024/1024));
		#endif

		if (!fileIsCompressed)
			return File(compressedPointer, compressedSize);

		// Uncompress the file
		{
			FB_STACK_CUSTOM("Decompression");

			util::CompressionType type = util::CompressionTypeLZ4;
			if(fileData->flags & 4)
				type = util::CompressionTypeZSTD;

			if (!util::decompressBuffer(type, compressedBuffer, compressedSize, uncompressedBuffer, uncompressedSize))
			{
				errorFunc(archiveFileName.getPointer(), FB_MSG("decompressBuffer failed for file: ", fileName).getPointer());
				return File(getEmptyBuffer(), 0);
			}
		}

		return File(uncompressedPointer, uncompressedSize);
	}

	bool doesFileExist(const DynamicString &fileName)
	{
		FB_ZONE("MysteryFilePackage::Impl::doesFileExist");
		FileMap::Iterator it = fileMap.find(fileName);
		return it != fileMap.getEnd();
	}

	static void popTrailingSlash(TempString &str)
	{
		if (str.getLength() > 1 && str[str.getLength() - 1] == '/')
			str.trimRight(1);
	}

	void listFiles(FileList &results, const StringRef &dir, FileFlagMask flags)
	{
		FB_ZONE("MysteryFilePackage::Impl::listFiles - 1");
		bool recurse = !(flags & FileFlagNoRecurse);
		bool excludeDirs = (flags & FileFlagExcludeDirs) != 0;
		bool excludeFiles = (flags & FileFlagExcludeFiles) != 0;

		TempString dirTmp(dir);
		if (!dirTmp.isEmpty())
			dirTmp += "/";

		DynamicString dirWithSlash(dirTmp);

		for (SizeType i = 0; i < fileHeaders.getSize(); ++i)
		{
			const DynamicString &fileName = fileHeaders[i].fileName;
			if (fileName.doesStartWith(dirWithSlash) && fileName != dirWithSlash)
			{
				if (!recurse)
				{
					const char *relativeFileName = fileName.getPointer() + dirWithSlash.getLength();
					const char *subDirStart = strchr(relativeFileName, '/');
					if (subDirStart && !excludeDirs)
					{
						// include the first subdirectory when listing directories
						const char *subDirEnd = strchr(subDirStart, '/');
						if (subDirEnd)
						{
							TempString fileNameDir(fileName.getPointer(), (SizeType)(1 + subDirEnd - fileName.getPointer()));
							results.insertOrAssign(DynamicString(fileNameDir), self);
						}
					}

					// don't recurse to subdirectories
					if (subDirStart)
						continue;
				}

				if (!excludeDirs)
				{
					TempString fileNameDir;
					fileNameDir.appendPathFromString(fileName);
					if (fileNameDir != dirWithSlash)
					{
						results.insertOrAssign(DynamicString(fileNameDir), self);
					}
				}
				if (!excludeFiles)
				{
					results.insertOrAssign(fileName, self);
				}
			}
		}
	}

	void listFiles(Vector<DynamicString> &files)
	{
		FB_ZONE("MysteryFilePackage::Impl::listFiles - 2");
		for (SizeType i = 0; i < fileHeaders.getSize(); ++i)
		{
			const DynamicString &fileName = fileHeaders[i].fileName;
			files.pushBack(fileName);
		}
	}


	static const uint32_t fileReadChunkSize = 1024 * 1024;

	MysteryFilePackage *self = nullptr;
	HeapString archiveFileName;
	ErrorFunc errorFunc = nullptr;

	typedef LinearHashMap<DynamicString, FileData*> FileMap;
	FileMap fileMap;
	Vector<FileData> fileHeaders;

	file::InputFile *file = nullptr;
	#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
		file::MemoryMappedFile mFile;
	#endif
	const char* memoryMappedFileReadPtr = nullptr;
	BigSizeType memoryMappedFileReadSize = 0;
	bool useCryptStuff = false;
	bool memoryResident = false;

	// This is a package specific mutex for our internal file pointer. We use no state for reading files, but file system file pointer is not guaranteed to be thread safe.
	Mutex fileMutex;

};

MysteryFilePackage::MysteryFilePackage(const StringRef &archive, bool optional, ErrorFunc errorFunc, bool useCryptStuff, bool memoryResident)
{
	bool useCryptStuffReally = useCryptStuff;

	// NOTE: If crypt stuff is used, archiver needs to use crypt stuff also when creating data packages
	if (FB_DEMO_ENABLED_DEFAULT_VALUE == true)
		useCryptStuffReally = true;
	else
		useCryptStuffReally = false;

	impl.reset(new Impl(this, archive, optional, errorFunc, useCryptStuffReally, memoryResident));
}

MysteryFilePackage::~MysteryFilePackage()
{
}

bool MysteryFilePackage::hasFiles() const
{
	return !impl->fileMap.isEmpty();
}

File MysteryFilePackage::openFile(const DynamicString &fileName, bool archiveOnly)
{
	return impl->openFile(fileName);
}

bool MysteryFilePackage::openFiles(const FileNameVector &names, FileVector &outFiles)
{
	/* First collect all consecutive files. This catches FileManager's async queueing */
	PodVector<FileData *> consecutiveFiles;
	uint64_t startOffset = 0xFFFFFFFF;
	uint64_t totalBufferSizeNeeded = 0;
	bool success = true;
	for (const DynamicString &fileName : names)
	{
		Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
		if (it == impl->fileMap.getEnd())
			break;

		FileData *fileData = it.getValue();
		if (consecutiveFiles.isEmpty())
		{
			consecutiveFiles.pushBack(fileData);
			startOffset = fileData->offset;
			totalBufferSizeNeeded = fileData->compressedSize;
		}
		else
		{
			/* Check if we have a file right after previous ones */
			/* FIXME: This should be single value. Now it's both here and FileManager. As the one in FileManager 
			 * dominates, I'll just leave this to largish, but not outrageous value */
			const uint32_t allowedHoleSize = 8 * 1024 * 1024;
			uint64_t holeSize = fileData->offset - (startOffset + totalBufferSizeNeeded);
			if (holeSize > allowedHoleSize)
				break;

			consecutiveFiles.pushBack(fileData);
			totalBufferSizeNeeded += fileData->compressedSize + holeSize;
		}
	}

	if (!consecutiveFiles.isEmpty())
	{
		/* We have some files to read in bulk. First allocate memory for uncompressed files, as that memory will live 
		 * longer than any temp buffers */
		Vector<FileBufferPointer> uncompressedPointers;
		for (FileData *fileData : consecutiveFiles)
			uncompressedPointers.pushBack(getFileBuffer(fileData->uncompressedSize, fileData->fileName.getPointer()));

		/* Allocate big buffer to read the data of all files in one go */
		PodVector<char> bufferVec;
		bufferVec.uninitialisedResize(SizeType(totalBufferSizeNeeded));
		char *buffer = &bufferVec.getFront();
		if (impl->memoryMappedFileReadPtr)
		{
			/* We don't use file semaphore, because we trust OS to do something sensible with memory mapping */
			lang::MemCopy::copy(buffer, impl->memoryMappedFileReadPtr + startOffset, totalBufferSizeNeeded);
		}
		else
		{
			SemaphoreGuard sg(getFileSemaphore());
			impl->file->readDataWithOffset(startOffset, buffer, totalBufferSizeNeeded);
		}

		/* Handle individual files */
		const uint64_t compressedBufferOffset = consecutiveFiles.getFront()->offset;
		for (SizeType fileIndex = 0, num = consecutiveFiles.getSize(); fileIndex < num; ++fileIndex)
		{
			FileData *fileData = consecutiveFiles[fileIndex];
			uint64_t compressedSize = fileData->compressedSize;
			uint64_t uncompressedSize = fileData->uncompressedSize;
			char *compressedBuffer = buffer + (fileData->offset - compressedBufferOffset);
			FileBufferPointer &uncompressedPointer = uncompressedPointers[fileIndex];
			char *uncompressedBuffer = uncompressedPointer.getMutable();
			/* Decrypt if necessary */
			#if FB_ENABLE_CRYPT_SUPPORT == FB_TRUE
				if (impl->useCryptStuff)
				{
					unsigned char keyFudge = 0;
					if (MysteryFilePackage::shouldCrypt(fileData->fileName, keyFudge))
					{
						unsigned char *xorTable = 0;
						int tableLength = 0;
						xorTable = MysteryFilePackage::getXorTable(tableLength);

						for (uint64_t decryptIndex = 0; decryptIndex < compressedSize; ++decryptIndex)
						{
							unsigned char v = (unsigned char)(compressedBuffer[decryptIndex]);
							unsigned char key  = xorTable[decryptIndex % tableLength];

							// Double the XOR! Double the safety!
							v ^= keyFudge;
							v ^= (unsigned char)((key + (decryptIndex * 4)) & 0xff);
							compressedBuffer[decryptIndex] = (char) v;
						}
					}
				}
			#endif
			/* Calculate data checksum */
			uint32_t checksum = util::calculateChecksumFast(compressedBuffer, compressedSize);
			if (checksum != fileData->checksum)
			{
				TempString msg;
				msg << "Checksum failed for " << fileData->fileName << "\n";
				msg << "  current checksum: ";
				msg.appendHexNumber(checksum, true) << "\n";
				msg << "  expected checksum: ";
				msg.appendHexNumber(fileData->checksum, true) << "\n";
				msg << "  offset: ";
				msg.appendHexNumber(fileData->offset, true) << "\n";
				msg << "  uncompressedSize: ";
				msg.appendHexNumber(uncompressedSize, true) << "\n";
				msg << "  compressedSize: ";
				msg.appendHexNumber(compressedSize, true);
				FB_FINAL_LOG_ERROR(msg);
				impl->errorFunc(impl->archiveFileName.getPointer(), FB_MSG("Checksum failed for file: ", fileData->fileName).getPointer());
				uncompressedPointer = getEmptyBuffer();
				uncompressedSize = 0;
				success = false;
			}

			if (fileData->flags & 1) 
			{
				/* No compression, just copy */
				lang::MemCopy::copy(uncompressedBuffer, compressedBuffer, compressedSize);
			}
			else if (uncompressedSize > 0)
			{
				/* Decompress */
				util::CompressionType type = util::CompressionTypeLZ4;
				if (fileData->flags & 4)
					type = util::CompressionTypeZSTD;

				fb_assert(uint32_t(compressedSize) == compressedSize && uint32_t(uncompressedSize) == uncompressedSize);
				if (util::decompressBuffer(type, compressedBuffer, uint32_t(compressedSize), uncompressedBuffer, uint32_t(uncompressedSize)) == 0)
				{
					impl->errorFunc(impl->archiveFileName.getPointer(), FB_MSG("decompressBuffer failed for file: ", fileData->fileName).getPointer());
					uncompressedPointer = getEmptyBuffer();
					uncompressedSize = 0;
					success = false;
				}
			}
			outFiles.pushBack(File(uncompressedPointer, uncompressedSize));
		}
	}

	/* Handle rest of the files. Commonly, we should never get here, unless someone is doing it wrong */
	for (SizeType i = consecutiveFiles.getSize(), num = names.getSize(); i < num; ++i)
	{
		outFiles.pushBack(openFile(names[i], false));
		success = success && outFiles.getBack();
	}
	return success;
}

bool MysteryFilePackage::doesFileExist(const DynamicString &fileName)
{
	return impl->doesFileExist(fileName);
}

bool MysteryFilePackage::doesFileExistInArchive(const DynamicString &fileName)
{
	return impl->doesFileExist(fileName);
}

BigSizeType MysteryFilePackage::getFileSize(const DynamicString &fileName)
{
	FB_ZONE("MysteryFilePackage::getFileSize");
	Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
	if (it != impl->fileMap.getEnd())
		return it.getValue()->uncompressedSize;

	return 0;
}

BigSizeType MysteryFilePackage::getFileSizeOnDisk(const DynamicString &fileName)
{
	FB_ZONE("MysteryFilePackage::getFileSizeOnDisk");
	Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
	if (it != impl->fileMap.getEnd())
		return it.getValue()->compressedSize;

	return 0;
}

bool MysteryFilePackage::writeFile(const DynamicString &fileName, const void *data, BigSizeType dataSize, bool createPath)
{
	// not supported
	return false;
}

bool MysteryFilePackage::deleteFile(const DynamicString &fileName, bool suppressErrorMessages)
{
	// not supported
	return false;
}

void MysteryFilePackage::listFiles(FileList &results, const StringRef &dir, FileFlagMask flags)
{
	impl->listFiles(results, dir, flags);
}

void MysteryFilePackage::sortFiles(FileSortInfo *pointer, SizeType files)
{
	for (SizeType i = 0; i < files; ++i)
	{
		FileSortInfo &f = pointer[i];
		f.packagedata = 0xFFFFFFFF;

		Impl::FileMap::Iterator it = impl->fileMap.find(f.fileName);
		if (it != impl->fileMap.getEnd())
		{
			FileData *fileData = it.getValue();
			f.packagedata = fileData->offset;
		}
	}

	algorithm::stableSort(pointer, pointer + files, FileSorter());
}

void MysteryFilePackage::listFiles(Vector<DynamicString> &files)
{
	impl->listFiles(files);
}

bool MysteryFilePackage::resolveFile(const DynamicString &fileName, FileResolveData &resolveDataOut) const
{
	Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
	if (it == impl->fileMap.getEnd())
		return false;

	const FileData *fileData = it.getValue();
	resolveDataOut.offset = fileData->offset;
	resolveDataOut.rawSize = fileData->compressedSize;
	resolveDataOut.size = fileData->uncompressedSize;
	return true;
}

bool MysteryFilePackage::resolveFile(const DynamicString &fileName, DynamicString &resultFileName, uint32_t &offset, uint32_t *size)
{
	Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
	if (it != impl->fileMap.getEnd())
	{
		FileData *fileData = it.getValue();

		// Uncompressed match?
		if (fileData->flags & 1)
		{
#if FB_TRACK_UNUSED_FILES == FB_TRUE
			fileData->hasBeenLoaded = true;
#endif
			resultFileName = DynamicString(impl->archiveFileName);
			offset = fileData->offset;
			if (size)
				*size = fileData->uncompressedSize;

			return true;
		}
	}

	return false;
}

bool MysteryFilePackage::resolvePackage(const DynamicString &fileName, ResolveData &resolveData)
{
	Impl::FileMap::Iterator it = impl->fileMap.find(fileName);
	if (it != impl->fileMap.getEnd())
	{
		FileData *FB_RESTRICT fileData = it.getValue();

		resolveData.handler = this;
		resolveData.internalData1 = fileData->offset;
		resolveData.internalData2 = fileData->compressedSize;
		return true;
	}

	return false;
}

bool MysteryFilePackage::isMemoryResident() const
{
	return impl->memoryResident;
}

uint32_t MysteryFilePackage::getPackageChecksum() const
{
	uint32_t checksum = 0;
	if (impl->memoryMappedFileReadPtr)
	{
		#if FB_USE_MEMORY_MAPPED_FILE_SUPPORT == FB_TRUE
			checksum = util::calculateChecksumFast(impl->mFile.getData(), impl->mFile.getSize());
		#endif
	}
	else if (impl->file)
	{
		uint32_t fileSize = (uint32_t)impl->file->getSize();
		char *fileData = new char[fileSize];
		impl->file->readDataWithOffset(0, fileData, fileSize);
		checksum = util::calculateChecksumFast(fileData, fileSize);
		delete[] fileData;
	}
	return checksum;
}

#if FB_TRACK_UNUSED_FILES == FB_TRUE
void MysteryFilePackage::dumpUnusedFiles(file::DebugBlockWriter &writer, file::DebugBlockWriter &writer2)
{
	int bytesUsed = 0;
	int totalBytes = 0;
	Impl::FileMap::Iterator it = impl->fileMap.getBegin();
	for (;it != impl->fileMap.getEnd(); it++)
	{
		totalBytes += it.getValue()->compressedSize;
		if (!it.getValue()->hasBeenLoaded)
		{
			char str[1024];
			sprintf(str, "%s\r\n", it.getKey().getPointer());
			writer.write(str, (int)strlen(str));
		}
		else
		{
			char str[1024];
			sprintf(str, "%s\r\n", it.getKey().getPointer());
			writer2.write(str, (int)strlen(str));

			bytesUsed += it.getValue()->compressedSize;
		}
	}
	FB_FINAL_LOG_INFO(FB_FMT("******** %s, bytes used %i out of %i bytes", impl->archiveFileName.getPointer(), bytesUsed, totalBytes));
}
#endif

#if FB_ENABLE_CRYPT_SUPPORT == FB_TRUE

	// Helpers for "crypt" stuff
	unsigned char *MysteryFilePackage::getXorTable(int &length)
	{
		static unsigned char table[] =
		{
			'F',
			'b',
			'R',
			'0',
			'c',
			's',
			'Y',
			'o',
			'u',
			'r',
			'S',
			'0',
			'c',
			'k',
			's',
		};

		length = sizeof(table);
		return table;
	}

	bool MysteryFilePackage::shouldCrypt(const DynamicString &fileName, unsigned char &keyFudge)
	{
		// List of safe, but important file types (which won't get aliased by other middleware readers)
		FB_STATIC_CONST_STRING(fbi, "fbi");
		FB_STATIC_CONST_STRING(fbm, "fbm");
		FB_STATIC_CONST_STRING(bin, "bin");
		FB_STATIC_CONST_STRING(lub, "lub");
		FB_STATIC_CONST_STRING(vs, "vs");
		FB_STATIC_CONST_STRING(ps, "ps");

		bool match = false;
		TempString extension;
		extension.appendFileExtensionFromString(fileName);
		if (extension.isEmpty())
			return false;

		if (extension == fbi)
			match = true;
		else if (extension == fbm)
			match = true;
		else if (extension == bin)
			match = true;
		else if (extension == lub)
			match = true;
		else if (extension == vs)
			match = true;
		else if (extension == ps)
			match = true;

		if (!match)
			return false;

		// Scientific stuff right here!
		int thingy = 0;
		for (SizeType i = 0; i < extension.getLength(); ++i)
			thingy += extension[i];
		thingy /= extension.getLength();
		keyFudge = (unsigned char)(thingy);

		return true;
	}

#endif

FB_END_PACKAGE2()
