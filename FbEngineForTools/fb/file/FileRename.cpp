#include "Precompiled.h"

#include "FileRename.h"

#include "fb/algorithm/Sort.h"
#include "fb/container/LinearMap.h"
#include "fb/container/PodVector.h"
#include "fb/file/CopyFile.h"
#include "fb/file/CreateDirectory.h"
#include "fb/file/DeleteDirectory.h"
#include "fb/file/DeleteFile.h"
#include "fb/file/DoesFileExist.h"
#include "fb/file/FileList.h"
#include "fb/file/MoveFile.h"
#include "fb/file/TemporaryFile.h"
#include "fb/lang/MinMax.h"
#include "fb/string/HeapString.h"
#include "fb/string/util/SplitString.h"
#include "fb/sys/versioncontrol/DefaultVersionControl.h"

FB_PACKAGE1(file)

#if FB_EDITOR_ENABLED == FB_TRUE

bool FileRename::renameDirectory(const StringRef &oldFullFilename, const StringRef &newFullFilename, HeapString &errorString)
{
	string::SplitString filenamePartsOld(oldFullFilename, "/");
	string::SplitString filenamePartsNew(newFullFilename, "/");
	HeapString newPathSoFar;
	HeapString oldPathSoFar;
	bool pathChanged = false;
	for (SizeType i = 0; i + 1 < filenamePartsOld.getNumPieces() && i + 1 < filenamePartsNew.getNumPieces(); i++)
	{
		newPathSoFar += filenamePartsNew[i];
		oldPathSoFar += filenamePartsOld[i];
		if (filenamePartsNew[i] != filenamePartsOld[i])
		{
			directoryMapping[oldPathSoFar] = newPathSoFar;
		}
		newPathSoFar += "/";
		oldPathSoFar += "/";
	}
	return true;
}

bool FileRename::renameFile(const StringRef &oldFullFilename, const StringRef &newFullFilename, HeapString &errorString)
{
	if (!file::doesFileExist(oldFullFilename))
	{
		errorString << "Source file " << oldFullFilename << " does not exist.";
		return false;
	}

	// check for directories for which only the case changed
	string::SplitString filenamePartsOld(oldFullFilename, "/");
	string::SplitString filenamePartsNew(newFullFilename, "/");
	HeapString newPathSoFar;
	HeapString oldPathSoFar;
	bool pathChanged = false;
	for (SizeType i = 0; i + 1 < filenamePartsOld.getNumPieces() && i + 1 < filenamePartsNew.getNumPieces(); i++)
	{
		newPathSoFar += filenamePartsNew[i];
		oldPathSoFar += filenamePartsOld[i];
		if (filenamePartsNew[i] != filenamePartsOld[i])
		{
			// only case changed
			if (filenamePartsNew[i].compareCaseInsensitive(filenamePartsOld[i]))
			{
				//fb_assert(directoryDepth(oldPathSoFar) == directoryDepth(newPathSoFar));
				directoryMapping[oldPathSoFar] = newPathSoFar;
			}
			else
			{
				pathChanged = true;
				break;
			}
		}
		newPathSoFar += "/";
		oldPathSoFar += "/";
	}

	HeapString fileMappingKey(oldFullFilename);
	fileMapping[fileMappingKey] = newFullFilename;
	return true;
}

void FileRename::moveFilesFromDir(const StringRef &oldDirectory, const StringRef &newDirectory)
{
	file::FileList list(oldDirectory);
	file::FileList::Entry *entry = nullptr;
	while ((entry = list.next()) != nullptr)
	{
		HeapString fromFile(oldDirectory);
		fromFile += "/";
		fromFile += entry->name;

		HeapString toFile(newDirectory);
		toFile += "/";
		toFile += entry->name;

		if (entry->directory)
		{
			moveFilesFromDir(fromFile, toFile);
		}
		else
		{
			HeapString fileMappingKey(fromFile);
			StringMap::Iterator it = fileMapping.find(fileMappingKey);
			if (it == fileMapping.getEnd())
			{
				// add new mapping
				fileMapping[fileMappingKey] = toFile;
			}
			else
			{
				// fix the path in existing mapping
				const StringRef name = it.getValue().getFileName();
				TempString toFile2(newDirectory);
				toFile2 += "/";
				toFile2 += name;
				it.getValue() = toFile2;
			}
		}
	}
}

struct DirPair
{
	SmallTempString oldDir;
	SmallTempString newDir;
};

struct DirDepthSort
{
	bool operator()(const DirPair &a, const DirPair &b)
	{
		int countA = 0;
		int countB = 0;
		for (SizeType i = 0, num = a.oldDir.getLength(); i < num; i++)
		{
			if (a.oldDir[i] == '/')
				countA++;
		}
		for (SizeType i = 0, num = b.oldDir.getLength(); i < num; i++)
		{
			if (b.oldDir[i] == '/')
				countB++;
		}
		return countA > countB;
	}
};

struct TemporaryFile
{
	HeapString filename;
	bool wasVersioned;
};

bool FileRename::finish(bool showCommitDialog)
{
	bool succeeded = true;

	typedef Vector<SmallTempString> StringList;
	StringList dirsToDelete;
	StringList filesToDelete;
	StringList filesToAdd;

	if (directoryMapping.getSize() > 0)
	{
		// create pairs of directories to rename
		Vector<DirPair> dirPairs;
		dirPairs.resize(directoryMapping.getSize());
		SizeType index = 0;
		for (StringMap::ConstIterator it = directoryMapping.getBegin(); it != directoryMapping.getEnd(); it++, index++)
		{
			dirPairs[index].oldDir = it.getKey();
			dirPairs[index].newDir = it.getValue();
		}

		// sort by depth
		algorithm::sort(dirPairs.getBegin(), dirPairs.getEnd(), DirDepthSort());

		// rename bottom up
		for (SizeType i = 0; i < dirPairs.getSize(); i++)
		{
			moveFilesFromDir(dirPairs[i].oldDir, dirPairs[i].newDir);
			filesToAdd.pushBack() = dirPairs[i].newDir;
			filesToDelete.pushBack() = dirPairs[i].oldDir;
			dirsToDelete.pushBack() = dirPairs[i].oldDir;
			dirsToDelete.getBack().toLower();
			dirsToDelete.getBack() += "/";
		}
	}

	sys::versioncontrol::DefaultVersionControl vc;

	#if defined(LOG_ENABLE)
		FILE *fout = fopen("log/rename.txt", "wb");
	#endif

	// Copy old files to temporary
	typedef LinearMap<HeapString, TemporaryFile> TemporaryMap;
	TemporaryMap temporaries;
	for (StringMap::ConstIterator it = fileMapping.getBegin(); it != fileMapping.getEnd(); ++it)
	{
		const int tempBufferSize = 270;
		char tempBuffer[tempBufferSize] = { 0 };
		TempString tempStr;
		if (!file::getTemporaryFilename(tempStr))
			return false;

		temporaries[tempStr].filename = it.getValue();
		temporaries[tempStr].wasVersioned = vc.isInVersionControl(it.getKey());

		if (!file::copyFile(it.getKey(), tempStr))
		{
			return false;
		}
		else
		{
			// delete only if the directory under it is not already deleted
			bool alreadyDeleting = false;
			for (SizeType i = 0; i < dirsToDelete.getSize(); i++)
			{
				if (strncmp(dirsToDelete[i].getPointer(), it.getKey().getPointer(), lang::min(it.getKey().getLength(), dirsToDelete[i].getLength())) == 0)
				{
					alreadyDeleting = true;
					break;
				}
			}
			if (!alreadyDeleting)
			{
				filesToDelete.pushBack(it.getKey());
			}
		}

		#if defined(LOG_ENABLE)
			if (i == 0)
				fprintf(fout, "Copy \"from\" to temporary\r\n");
			fprintf(fout, "CopyFile %s to %s\r\n", it.getKey().getPointer(), tempStr.getPointer());
		#endif
	}

	// delete unversioned old files
	for (SizeType i = 0; i < filesToDelete.getSize();)
	{
		if (vc.isInVersionControl(filesToDelete[i]))
		{
			file::deleteFile(filesToDelete[i]);
			i++;
		}
		else
		{
			file::deleteFile(filesToDelete[i]);
			filesToDelete[i] = filesToDelete.getBack();
			filesToDelete.popBack();
		}
	}
	for (SizeType i = 0; i < dirsToDelete.getSize(); i++)
	{
		if (vc.isInVersionControl(dirsToDelete[i]))
		{
			file::deleteEmptyDirectory(dirsToDelete[i]);
			filesToDelete.pushBack(dirsToDelete[i]);
		}
		else
		{
			file::deleteEmptyDirectory(dirsToDelete[i]);
		}
	}

	// delete versioned old files
	if (!filesToDelete.isEmpty())
	{
		vc.runCommand(sys::versioncontrol::DefaultVersionControl::Delete, &filesToDelete[0], (int)filesToDelete.getSize(), HeapString("Removing files with invalid filenames"));
		if (showCommitDialog)
			vc.runCommand(sys::versioncontrol::DefaultVersionControl::Commit, &filesToDelete[0], (int)filesToDelete.getSize(), HeapString("Removing files with invalid filenames"));
	}

	// Move old files from temporary to new locations
	for (TemporaryMap::ConstIterator it = temporaries.getBegin(); it != temporaries.getEnd(); ++it)
	{
		TempString newPath;
		newPath.appendPathFromString(it.getValue().filename);
		file::createPathIfMissing(newPath);
		if (!file::moveFile(it.getKey(), it.getValue().filename))
		{
			succeeded = false;
		}
		else
		{
			if (it.getValue().wasVersioned)
			{
				filesToAdd.pushBack(it.getValue().filename);
			}
		}

		#if defined(LOG_ENABLE)
			if (i == 0)
				fprintf(fout, "\r\n\r\nMove temporary to \"to\"\r\n");
			fprintf(fout, "MoveFile %s to %s\r\n", it.getKey().getPointer(), it.getValue().getPointer());
		#endif
	}

	// Commit changes
	if (!filesToAdd.isEmpty() && showCommitDialog)
		vc.runCommand(sys::versioncontrol::DefaultVersionControl::Commit, &filesToAdd[0], (int)filesToAdd.getSize(), HeapString("Adding files with correct filenames"));

	return succeeded;
}

#endif

FB_END_PACKAGE1()
