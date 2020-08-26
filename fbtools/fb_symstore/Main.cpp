#include "Precompiled.h"
#include "fb/container/ArraySlice.h"
#include "fb/container/LinearHashMap.h"
#include "fb/file/InputFile.h"
#include "fb/file/MoveFile.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/DeleteDirectory.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/FileList.h"
#include "fb/file/OutputFile.h"
#include "fb/lang/IncludeWindows.h"
#include "fb/lang/time/ScopedTimer.h"
#include "Cabinet.h"
#include <cstdlib>

FB_PACKAGE0()

bool compressAndMovePDB(HeapString tempFilename, HeapString pdbName, HeapString destinationFilename, Vector<HeapString> &log)
{
	TempString cabFilename;
	cabFilename << tempFilename << ".cab";

	printf("Compressing file\n");
	{
		ScopedTimer t;
		if (!cabinet::createCabinet(tempFilename, pdbName, cabFilename, log))
			return false;

		printf("Compression took %g seconds\n", t.getTime().getSeconds());
	}

	printf("Copying file\n");
	{
		ScopedTimer t;
		destinationFilename.replace("\\", "/");
		file::createPathIfMissing(destinationFilename);
		if (!file::moveFile(cabFilename, destinationFilename))
		{
			log.pushBack().doSprintf("Moving from %s to %s failed", cabFilename.getPointer(), destinationFilename.getPointer());
			return false;
		}
		file::deleteFile(tempFilename);
		printf("Copying took %g seconds\n", t.getTime().getSeconds());
	}

	printf("Copy done\n");
	return true;
}

bool readFile(StringRef filename, PodVector<uint8_t> &buffer)
{
	file::InputFile file;
	if (!file.open(filename))
	{
		return false;
	}
	buffer.uninitialisedResize((SizeType)file.getSize());
	return file.readData(buffer.getPointer(), buffer.getSize()) == buffer.getSize();
}

bool addPDB(HeapString path, HeapString sharePath, StringRef branchName, Vector<HeapString> &log)
{
	path.replace("\\", "/");
	sharePath.replace("\\", "/");

	char exeFilename[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(nullptr, exeFilename, MAX_PATH);

	WIN32_FIND_DATAA findData = { 0 };
	HANDLE handle = FindFirstFileExA(path.getPointer(), FindExInfoBasic, &findData, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
	if (handle == INVALID_HANDLE_VALUE)
	{
		log.pushBack().doSprintf("No files found with %s", path.getPointer());
		return false;
	}
	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		TempString filename;
		filename.appendPathFromString(path);
		filename << findData.cFileName;

		// ignore xxx_compile.pdb
		if (filename.doesEndWith("_compile.pdb"))
			continue;
		// ignore xxx_compile.dbs.xx.pdb
		if (filename.doesContain("_compile.dbs."))
			continue;

		// PDB
		TempString destinationFilename;
		{
			// load file
			PodVector<uint8_t> data;
			if (!readFile(filename, data))
			{
				log.pushBack().doSprintf("Failed to read %s", filename.getPointer());
				return false;
			}

			// find PDB header
			const uint32_t PdbStreamVersion_VC70 = 20000404;
			struct PdbStreamHeader
			{
				uint32_t Version;
				uint32_t Signature;
				uint32_t PdbAge;
				GUID PdbSig70;
			};
			PdbStreamHeader header = { 0 };
			bool headerFound = false;
			for (SizeType i = 0; i + 3 < data.getSize(); i++)
			{
				if (*(const uint32_t *)(data.getPointer() + i) == PdbStreamVersion_VC70)
				{
					memcpy(&header, data.getPointer() + i, sizeof(header));
					headerFound = true;
					break;
				}
			}
			if (!headerFound)
			{
				log.pushBack().doSprintf("Did not find PDB header from %s", filename.getPointer());
				return false;
			}

			TempString pdbName;
			pdbName.appendFileNameFromString(filename);

			TempString pdbVersion;
			pdbVersion.doSprintf("%08lX", header.PdbSig70.Data1);
			pdbVersion.doSprintf("%04X", header.PdbSig70.Data2);
			pdbVersion.doSprintf("%04X", header.PdbSig70.Data3);
			for (int j = 0; j < 8; j++)
				pdbVersion.doSprintf("%02X", header.PdbSig70.Data4[j]);
			pdbVersion.doSprintf("%x", header.PdbAge);

			printf("%s %s\n", pdbName.getPointer(), pdbVersion.getPointer());

			// make a copy of the PDB (so build can proceed)
			char tempPathBuffer[MAX_PATH] = { 0 };
			GetTempPathA(MAX_PATH, tempPathBuffer);
			char tempFilename[MAX_PATH] = { 0 };
			GetTempFileNameA(tempPathBuffer, pdbName.getPointer(), 0, tempFilename);
			if (!CopyFileA(filename.getPointer(), tempFilename, FALSE))
			{
				DWORD error = GetLastError();
				log.pushBack().doSprintf("Copying %s to %s failed (%u)", filename.getPointer(), tempFilename, error);
				return false;
			}

			// start a new process for compressing file to workspace (so build can proceed)
			{
				destinationFilename << sharePath;
				destinationFilename << "/";
				destinationFilename.appendFileNameFromString(filename);
				destinationFilename << "/" << pdbVersion << "/";
				destinationFilename.appendFileNameFromString(filename);
				// .pdb -> .pd_
				destinationFilename[destinationFilename.getLength() - 1] = '_';

				destinationFilename.replace("/", "\\");
				for (char &c : tempFilename)
				{
					if (c == '/')
						c = '\\';
				}
				TempString args;
				args << exeFilename;
				args << " compressAndMove \"" << tempFilename << "\" " << pdbName << " " << destinationFilename;

				STARTUPINFO si = { 0 };
				si.cb = sizeof(STARTUPINFO);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_SHOW | SW_MINIMIZE;

				PROCESS_INFORMATION pi = { 0 };
				if (!CreateProcessA(exeFilename, (LPSTR)args.getPointer(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
				{
					log.pushBack().doSprintf("Failed to start compression for PDB %s", filename.getPointer());
				}
			}

			// write additional info to workspace
			if (!branchName.isEmpty())
			{
				TempString destinationPath(destinationFilename);
				destinationPath.replace("\\", "/");

				TempString branchFilename;
				branchFilename.appendPathFromString(destinationPath);
				branchFilename << "branch_" << branchName << ".txt";
				file::createPathIfMissing(branchFilename);
				file::OutputFile file;
				if (file.open(branchFilename))
				{
					file.writeData(branchName.getPointer(), branchName.getLength());
				}
			}
		}

		// copy executable to workspace
		filename.trimRight(4);
		filename += ".exe";
		if (file::doesFileExist(filename, false))
		{
			TempString exeName;
			exeName.appendFileNameFromString(filename);

			// make a copy of the exe (so build can proceed)
			char tempPathBuffer[MAX_PATH] = { 0 };
			GetTempPathA(MAX_PATH, tempPathBuffer);
			char tempFilename[MAX_PATH] = { 0 };
			GetTempFileNameA(tempPathBuffer, exeName.getPointer(), 0, tempFilename);
			if (!CopyFileA(filename.getPointer(), tempFilename, FALSE))
			{
				DWORD error = GetLastError();
				log.pushBack().doSprintf("Copying %s to %s failed (%u)", filename.getPointer(), tempFilename, error);
				return false;
			}

			// start a new process for compressing file to workspace (so build can proceed)
			{
				destinationFilename.trimRight(4);
				destinationFilename += ".ex_";
				for (char &c : tempFilename)
				{
					if (c == '/')
						c = '\\';
				}
				TempString args;
				args << exeFilename;
				args << " compressAndMove \"" << tempFilename << "\" " << exeName << " " << destinationFilename;

				STARTUPINFO si = { 0 };
				si.cb = sizeof(STARTUPINFO);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_SHOW | SW_MINIMIZE;

				PROCESS_INFORMATION pi = { 0 };
				if (!CreateProcessA(exeFilename, (LPSTR)args.getPointer(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
				{
					log.pushBack().doSprintf("Failed to start compression for PDB %s", filename.getPointer());
				}
			}
		}
	}
	while (FindNextFileA(handle, &findData));

	// also launch a cleanup process
	{
		TempString args;
		args << exeFilename;
		args << " cleanup \"" << sharePath << "\"";

		STARTUPINFO si = { 0 };
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW | SW_MINIMIZE;

		PROCESS_INFORMATION pi = { 0 };
		if (!CreateProcessA(exeFilename, (LPSTR)args.getPointer(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			log.pushBack().doSprintf("Failed to start cleanup for PDBs %s", sharePath.getPointer());
		}
	}

	return true;
}

struct PDBFile
{
	HeapString branchName;
	file::TimeStamp64 timestamp;
};

static void listFilesRecursively(StringRef path, LinearHashMap<HeapString, PDBFile> &files, uint64_t &totalBytes)
{
	Vector<HeapString> recursePaths;
	{
		file::FileList fileList(path);
		file::FileList::Entry *entry = nullptr;
		while ((entry = fileList.next()) != nullptr)
		{
			if (entry->directory)
			{
				HeapString fullPath;
				fullPath << path << "\\" << entry->name;
				recursePaths.pushBack(fullPath);
			}
			else
			{
				if (entry->name.doesStartWith("branch_") && entry->name.doesEndWith(".txt"))
				{
					TempString branchName(entry->name);
					branchName.trimLeft(StringRef("branch_").getLength());
					branchName.trimRight(StringRef(".txt").getLength());
					files[HeapString(path)].branchName = branchName;
				}
				else if (entry->name.doesEndWith(".pd_"))
				{
					PDBFile &file = files[HeapString(path)];
					file.timestamp = entry->lastModified;
					totalBytes += entry->size;
				}
			}
		}
	}
	for (HeapString &fullPath : recursePaths)
		listFilesRecursively(fullPath, files, totalBytes);
}

static uint64_t millisecondsToMinutes(uint64_t ms)
{
	return ms / (60 * 1000);
}

void cleanupPDBs(const StringRef &sharePath)
{
	printf("Listing files\n");

	// check if running cleanup is necessary
	{
		TempString filename;
		filename << sharePath << "\\last_cleanup.txt";
		uint64_t timeNow = file::getTimestampFromFileTime(file::getCurrentTimeStamp());
		file::TimeStamp64 lastCleanup = file::getFileTimestamp64(filename);

		// clean up once per hour
		if (lastCleanup == -1 || millisecondsToMinutes(timeNow - file::getTimestampFromFileTime(lastCleanup)) > 60)
		{
			TempString timeNowString;
			timeNowString << timeNow;
			file::OutputFile file;
			if (file.open(filename))
			{
				file.writeData(timeNowString.getPointer(), timeNowString.getLength());
			}
		}
		else
		{
			printf("Last cleanup was %u minutes ago, skipping\n", (uint32_t)millisecondsToMinutes(timeNow - file::getTimestampFromFileTime(lastCleanup)));
			return;
		}
	}

	// find files
	uint64_t totalBytes = 0;
	LinearHashMap<HeapString, PDBFile> pdbFiles;
	{
		ScopedTimer t;
		listFilesRecursively(sharePath, pdbFiles, totalBytes);
		printf("Listing files took %g seconds\n", t.getTime().getSeconds());
	}

	printf("Found %u PDB files, total %" FB_FSU64 " MB\n", pdbFiles.getSize(), (totalBytes + (1024*1024 - 1)) / (1024*1024));

	// delete files
	uint64_t timeNow = file::getTimestampFromFileTime(file::getCurrentTimeStamp());
	for (const LinearHashMap<HeapString, PDBFile>::Iterator &it : pdbFiles)
	{
		const PDBFile &pdbFile = it.getValue();

		uint64_t deleteAfterDays = 100;
		if (pdbFile.branchName.doesEndWith("_exe"))
		{
			// editor PDBs are only stored for 20 days
			deleteAfterDays = 20;
		}

		uint64_t ageInDays = millisecondsToMinutes(timeNow - file::getTimestampFromFileTime(pdbFile.timestamp)) / (60 * 24);
		if (ageInDays > deleteAfterDays)
		{
			HeapString path = it.getKey();
			printf("Deleting %s (%" FB_FSU64 " days old)\n", path.getPointer(), ageInDays);
			path.replace("/", "\\");
			TempString error;
			file::deleteDirectoryAndFiles(path, error);
		}
	}

	printf("Cleanup done\n");
}

int main(int argc, const char *argv[])
{
	printf("Running:");
	for (int i = 1; i < argc; i++)
		printf(" %s", argv[i]);
	printf("\n");

	Vector<HeapString> log;
	bool success = false;
	if (StringRef(argv[1]) == "add" && argc >= 4)
	{
		success = addPDB(HeapString(argv[2]), HeapString(argv[3]), argc >= 5 ? StringRef(argv[4]) : StringRef(""), log);
	}
	else if (StringRef(argv[1]) == "compressAndMove" && argc == 5)
	{
		success = compressAndMovePDB(HeapString(argv[2]), HeapString(argv[3]), HeapString(argv[4]), log);
	}
	else if (StringRef(argv[1]) == "cleanup" && argc == 3)
	{
		cleanupPDBs(StringRef(argv[2]));
		success = true;
	}
	else
	{
		printf("Invalid args. Usage:\n");
		printf("add path sharepath branchname\n");
		printf("compressAndMove tempfilename pdbname sharefilename\n");
		printf("cleanup sharepath\n");
		return 1;
	}

	for (HeapString &line : log)
	{
		printf("%s\n", line.getPointer());
	}
	return success ? 0 : 1;
}

FB_END_PACKAGE0()

int main(int argc, const char *argv[])
{
	return fb::main(argc, argv);
}