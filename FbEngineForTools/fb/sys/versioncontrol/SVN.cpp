#include "Precompiled.h"

#if (FB_EDITOR_ENABLED == FB_TRUE)
#include "SVN.h"

#include "fb/container/LinearHashMap.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/InputFile.h"
#include "fb/file/OutputFile.h"
#include "fb/file/TemporaryFile.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/UnicodeConverter.h"
#include "fb/sys/windows/RunProcess.h"

#include <time.h>
#include "external/sqlite/sqlite3.h"
#include <direct.h>

FB_PACKAGE2(sys, versioncontrol)

struct SVN::Data
{
	HeapString svn;
	
	class DatabaseCache
	{
	public:
		DatabaseCache(const HeapString &rootDir)
			: rootDir(rootDir)
		{
		}

		HeapString rootDir;
		LinearHashMap<HeapString, __time64_t> files;
	};
	ScopedPointer<DatabaseCache> databaseCache;

	static void convertToAbsoluteFilename(HeapString &filename)
	{
		if (filename.getLength() >= 3 && filename[1] == ':' && filename[2] == '/')
		{
			// already absolute
		}
		else
		{
			char wdir[1024] = {0};
			_getcwd(wdir, 1024);
			HeapString absoluteFilename(wdir);
			absoluteFilename.replace("\\", "/");
			if (!absoluteFilename.isEmpty() && absoluteFilename[absoluteFilename.getLength()-1] != '/')
				absoluteFilename += "/";

			absoluteFilename += filename;
			filename = absoluteFilename;
		}
	}

	Data()
	{
		// ToDo: Look for this from several folders or something?
		svn = "C:\\Program Files\\TortoiseSVN\\bin\\TortoiseProc.exe";
	}

	~Data()
	{
		databaseCache.reset();
	}

	bool findDatabaseRootDir(HeapString &path)
	{
		TempString dbFilename;
		dbFilename << path << ".svn/wc.db";
		if (file::doesFileExist(dbFilename))
		{
			return true;
		}
		else if (!path.isEmpty())
		{
			// go down one level
			for (SizeType len = path.getLength(), i = len - 2; i < len; i--)
			{
				if (path[i] == '/')
				{
					path.truncateToSize(i+1);
					return findDatabaseRootDir(path);
				}
			}
			path.truncateToSize(0);
			return findDatabaseRootDir(path);
		}
		return false;
	}
	
	static int sqlQueryCallback(void *resultPtr, int numCols, char **colText, char **colNames)
	{
		DatabaseCache *db = (DatabaseCache *)resultPtr;
		if (numCols >= 1 && colText[0])
		{
			__time64_t date = 0;
			if (colText[1])
			{
				sscanf(colText[1], "%llu", &date);
			}
			db->files[TempString(colText[0])] = date;
		}
		return 0;
	}

	bool findFileFromDatabase(const StringRef &filename_, __time64_t *outDate = nullptr)
	{
		HeapString absoluteFilename(filename_);
		convertToAbsoluteFilename(absoluteFilename);
		// dir shouldn't trail
		if (!absoluteFilename.isEmpty() && absoluteFilename[absoluteFilename.getLength()-1] == '/')
			absoluteFilename.trimRight(1);

		// if cached DB path doesn't match
		if (!databaseCache.get() || strstr(absoluteFilename.getPointer(), databaseCache->rootDir.getPointer()) != absoluteFilename.getPointer())
		{
			// find TortoiseSVN 1.7 database in .svn/wc.db
			HeapString dbRoot;
			dbRoot.appendPathFromString(absoluteFilename);
			if (!findDatabaseRootDir(dbRoot))
				return false;

			TempString dbFilename;
			dbFilename << dbRoot << ".svn/wc.db";

			sqlite3 *db = nullptr;
			sqlite3_open_v2(dbFilename.getPointer(), &db, SQLITE_OPEN_READONLY, nullptr);
			if (!db)
				return false;
			databaseCache.reset(new DatabaseCache(dbRoot));

			// dump all rows into a hashmap (yes this is a billion times faster than using SQL queries)
			TempString query("SELECT local_relpath,last_mod_time FROM NODES;");
			char *errorMessage = nullptr;
			sqlite3_exec(db, query.getPointer(), sqlQueryCallback, databaseCache.get(), &errorMessage);
			if (errorMessage)
				sqlite3_free(errorMessage);

			sqlite3_close(db);

			fb_assert(!databaseCache->files.isEmpty() && "TSVN database format not supported");
		}

		// find file from cache
		const char *pathRelativeToDBRoot = absoluteFilename.getPointer() + databaseCache->rootDir.getLength();
		auto it = databaseCache->files.find(TempString(pathRelativeToDBRoot));
		if (it == databaseCache->files.getEnd())
			return false;
		if (outDate)
			*outDate = it.getValue();
		return true;
	}

	bool isFilePossiblyChanged(const StringRef &filename)
	{
		// NOTE: This returns true quite often even if the file hasn't changed, but seems to work for now quite OK
		// TODO: tweak tweak tweak
		// - Jari K. 2010-07-08 

		// get file timestamp
		WIN32_FILE_ATTRIBUTE_DATA fad;
		memset(&fad, 0, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
		if (GetFileAttributesEx(filename.getPointer(), GetFileExInfoStandard, &fad) == 0)
		{
			/*
			// For debug
			void *buf;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,GetLastError(),0,
				(LPTSTR) &buf,0,nullptr);

			MessageBox(nullptr, (LPTSTR)buf, nullptr, MB_OK | MB_ICONINFORMATION);
			LocalFree(buf);

			// Seems to fail on these
			// "The system cannot find the path specified."
			// "The system cannot find the file specified."
			*/

			// NOTE: Don't allow crappy files or folders. It messes up the SVN .tmp commit list with crappy files and folders.
			// Due to that, SVN commit window doesn't select files (checkbox enabled) and because of that lots of files aren't committed to the SVN
			// So, return false
			// - Jari K. 2010-07-08 
			if (!isInVersionControl(filename))
			{
				return false;
			}

			// file doesn't exist: it was deleted
			return true;
		}

		SYSTEMTIME tm;
		FileTimeToSystemTime(&fad.ftLastWriteTime, &tm);

		// fetch timestamp info from svn entries
		TempString path;
		path.appendPathFromString(filename);
		TempString entriesFile;
		entriesFile << path << "/.svn/entries.";
		file::InputFile inputFile;
		if (inputFile.open(entriesFile))
		{
			BigSizeType size = inputFile.getSize();
			fb_assert(size < PodVector<char>::getMaxCapacity());
			PodVector<char> fileData;
			fileData.resize(SizeType(size));
			if (size == 0)
			{
				// (Almost) never returns here?
				return true;
			}
			
			TempString file;
			file.appendFileNameFromString(filename);
			bool isDir = file.getFileExtension()[0] == '\0';

			TempString identifierString(file);
			if (isDir)
				identifierString += "\ndir";
			else
				identifierString += "\nfile";

			inputFile.readData(&fileData[0], fileData.getSize());
			inputFile.close();
			char *ptr = &fileData[0];
			for (SizeType i = 0; i < size; i++)
			{
				SizeType len = SizeType(size - i);
				if (len > identifierString.getLength())
					len = identifierString.getLength();

				if (_strnicmp(ptr+i, identifierString.getPointer(), len) == 0)
				{
					// find next timestamp
					SizeType start = i + len;
					for (SizeType j = start; j < size; j++)
					{
						int lengthLeft = (int)(size - j);
						// \n2010-02-01T10:40:50.000000Z
						//  0123456789012345678901234567
						if (ptr[j] == '\n' && lengthLeft >= 28
							&& ptr[j+5] == '-'
							&& ptr[j+8] == '-'
							&& ptr[j+11] == 'T'
							&& ptr[j+14] == ':'
							&& ptr[j+17] == ':'
							&& ptr[j+20] == '.'
							&& ptr[j+27] == 'Z')
						{
							char timeStamp[28];
							memcpy(timeStamp, ptr+j+1, 27);
							timeStamp[27] = 0;
							int file_year, file_mon, file_mday, file_hour, file_min, file_sec, file_usec;
							if (sscanf(timeStamp, "%04d-%02d-%02dT%02d:%02d:%02d.%06dZ",
																&file_year,
																&file_mon,
																&file_mday,
																&file_hour,
																&file_min,
																&file_sec,
																&file_usec) == 7)
							{
								if (file_year == tm.wYear
									&& file_mon == tm.wMonth
									&& file_mday == tm.wDay
									&& file_hour == tm.wHour
									&& file_min == tm.wMinute
									&& file_sec == tm.wSecond
									&& (file_usec / 1000) == tm.wMilliseconds)
								{
									// timestamp matches: file has not been changed
									return false;
								}
								break;
							}
						}
					}
					break;
				}
			}
		}
		else
		{
			__time64_t date;
			if (findFileFromDatabase(filename, &date))
			{
				// convert date to tm
				date /= 1000000;
				struct tm dateTM = {0};
				_localtime64_s(&dateTM, &date);

				// convert sys time to local time
				static TIME_ZONE_INFORMATION timeZone = {-1};
				if (timeZone.Bias == -1)
					GetTimeZoneInformation(&timeZone);
				SYSTEMTIME tmO = tm;
				SystemTimeToTzSpecificLocalTime(&timeZone, &tmO, &tm);

				bool year = tm.wYear == (WORD)dateTM.tm_year + 1900;
				bool day = tm.wDay == (WORD)dateTM.tm_mday;
				bool hour = tm.wHour == (WORD)dateTM.tm_hour;
				bool min = tm.wMinute == (WORD)dateTM.tm_min;
				bool mon = tm.wMonth == (WORD)dateTM.tm_mon+1;
				bool sec = tm.wSecond == (WORD)dateTM.tm_sec;

				// timestamp matches: file has not been changed
				if (year && day && hour && min && mon && sec)
					return false;
			}
		}

		// otherwise we can't know for sure
		// NOTE: Returns true here quite often
		return true;
	}
	
	bool isInVersionControl(const StringRef &filename)
	{
		TempString path;
		path.appendPathFromString(filename);
		TempString entriesFile;
		entriesFile << path << "/.svn/entries.";
		file::InputFile inputFile;
		if (!inputFile.open(entriesFile))
		{
			if (findFileFromDatabase(filename))
				return true;
			return false;
		}
		BigSizeType size = inputFile.getSize();
		fb_assert(size < PodVector<char>::getMaxCapacity());
		PodVector<char> fileData;
		fileData.resize(SizeType(size));
		if (size == 0)
			return false;
		
		TempString file;
		file.appendFileNameFromString(filename);
		bool isDir = file.getFileExtension()[0] == '\0';

		TempString identifierString(file);
		if (isDir)
			identifierString += "\ndir";
		else
			identifierString += "\nfile";

		inputFile.readData(&fileData[0], fileData.getSize());
		inputFile.close();
		char *ptr = &fileData[0];
		for (SizeType i = 0; i < size; i++)
		{
			if (_strnicmp(ptr+i, identifierString.getPointer(), identifierString.getLength()) == 0)
				return true;
		}
		return false;
	}

	bool run(Command command, const HeapString *files, int fileAmount, const HeapString &logMessage)
	{
		// unlock database
		databaseCache.reset();

		// ToDo: We'd need to split the command if it gets too long ..
		HeapString cmd = svn;
		cmd += " ";

		// Add command
		cmd += "/command:";
		if (command == Update)
			cmd += "update";
		else if (command == Commit)
			cmd += "commit";
		else if (command == Add)
			cmd += "add";
		else if (command == Delete)
			cmd += "remove";
		else if (command == Lock)
			cmd += "lock";
		else if (command == Unlock)
			cmd += "unlock";
		
		const int tempBufferSize = 270;
		char tempBuffer[tempBufferSize] = { 0 };
		TempString tempFilename;
		if (!file::getTemporaryFilename(tempFilename))
			return false;

		if (tempFilename[0] == '.' && tempFilename[1] == '\\')
			tempFilename.trimLeft(2);

		file::OutputFile out;
		if (!out.open(tempFilename))
		{
			return false;
		}
		const int pathSize = 290;
		char currentPath[pathSize] = { 0 };
		GetCurrentDirectory(pathSize, currentPath);

		// UTF-16 identifier
		//const unsigned char utf16id[2] = {0xFF, 0xFE};
		//out.writeData(utf16id, 2);

		for (int i = 0; i < fileAmount; ++i)
		{
			// Sanity check! (check another one later)
			// NOTE: We have a bug where editor tries to delete whole binary folder!
			// Try to detect it and assert!
			// If "data/root" is missing from the path, we are definitely trying to delete something outside of the root folder!
			// All type files are in the root folder!
			if (command == Delete && strstr(files[i].getPointer(), "data/root") == nullptr)
			{
				fb_assert(0 && "Sanity check vol 1. We are trying to delete files outside of \"data/root\" folder! Fix me asap! Hansoft task: hansoft://hansoft;Frozenbyte;abcb8319/Task/567510");

				// Stop this madness!
				return false;
			}

			// Do we need to apply full path here?
			TempString f(currentPath);
			f += "\\";
			f += files[i];
			for (SizeType j = 0; j < f.getLength(); ++j)
			{
				if (f[j] == '/')
					f[j] = '\\';
			}
			// don't end directories in a slash
			if (f[f.getLength()-1] == '\\')
				f.trimRight(1);

			if (i+1 != fileAmount)
			{
				f += "\n";
			}
			
			// Sanity check! Vol 2
			if (command == Delete && strstr(f.getPointer(), "\\data\\root\\") == nullptr)
			{
				fb_assert(0 && "Sanity check vol 2. We are trying to delete files outside of \"data/root\" folder! Fix me asap! Hansoft task: hansoft://hansoft;Frozenbyte;abcb8319/Task/567510");

				// Stop this madness!
				return false;
			}

			// convert to UTF-16
			string::SimpleUTF16String dest;
			string::UnicodeConverter::addUTF8StrToUTF16String(f.getPointer(), dest);
			out.writeData(dest.getPointer(), dest.getSize() * sizeof(string::UTF16Unit));
		}
		out.close();


		// Add files
		cmd += " /deletepathfile /pathfile:\"";
		cmd += currentPath;
		cmd += "\\";
		cmd += tempFilename;
		
		// Add log message
		cmd += "\" /logmsg:\"";
		cmd += logMessage;
		cmd += "\"";

		// Don't auto close for now
		cmd += " /closeonend:0";

		bool status = sys::windows::runProcessWait(svn.getPointer(), cmd.getPointer());
		/*if (command == Delete)
		{
			for (int i = 0; i < fileAmount; ++i)
				file::deleteFile(files[i].getPointer());
		}*/

		return status;
	}
};

SVN::SVN()
:	data(new Data())
{
}

SVN::~SVN()
{
	delete data;
}

bool SVN::isFilePossiblyChanged(const StringRef &filename)
{
	return data->isFilePossiblyChanged(filename);
}

bool SVN::isInVersionControl(const StringRef &filename)
{
	return data->isInVersionControl(filename);
}

bool SVN::runCommand(Command command, const HeapString *files, int fileAmount, const HeapString &logMessage)
{
	return data->run(command, files, fileAmount, logMessage);
}

FB_END_PACKAGE2()
#endif
