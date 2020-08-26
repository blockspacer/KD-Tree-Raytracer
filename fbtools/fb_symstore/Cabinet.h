#pragma once

#include <fci.h>
#include <fdi.h>
#include <io.h>
#include <fcntl.h>
#pragma comment(lib,"cabinet.lib")

FB_PACKAGE1(cabinet)

static int DIAMONDAPI fnFilePlaced(PCCAB pccab, LPSTR pszFile, long cbFile, BOOL fContinuation, void FAR *pv)
{
	return 0;
}

static void HUGE * FAR DIAMONDAPI fnMemAlloc(ULONG cb)
{
	return new uint8_t[cb];
}

static void FAR DIAMONDAPI fnMemFree(void HUGE *memory)
{
	delete[](uint8_t *)memory;
}

static INT_PTR FAR DIAMONDAPI fnOpen(IN LPSTR pszFile, int oflag, int pmode)
{
	HANDLE hFile;
	DWORD dwDesiredAccess;
	if (oflag & _O_RDWR)
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else if (oflag & _O_WRONLY)
	{
		dwDesiredAccess = GENERIC_WRITE;
	}
	else
	{
		dwDesiredAccess = GENERIC_READ;
	}

	hFile = CreateFileA(pszFile, dwDesiredAccess, (DWORD)FILE_SHARE_READ, NULL, (oflag & _O_CREAT) ? (DWORD)CREATE_ALWAYS : (DWORD)OPEN_EXISTING, (DWORD)FILE_ATTRIBUTE_NORMAL, NULL);
	return (INT_PTR)hFile;
}

static INT_PTR FAR DIAMONDAPI fnFileOpen(IN LPSTR pszFile, int oflag, int pmode, int FAR *err, void FAR *pv)
{
	HANDLE hFile;
	DWORD dwDesiredAccess;
	if (oflag & _O_RDWR)
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else if (oflag & _O_WRONLY)
	{
		dwDesiredAccess = GENERIC_WRITE;
	}
	else
	{
		dwDesiredAccess = GENERIC_READ;
	}

	hFile = CreateFileA(pszFile, dwDesiredAccess, (DWORD)FILE_SHARE_READ, NULL, (oflag & _O_CREAT) ? (DWORD)CREATE_ALWAYS : (DWORD)OPEN_EXISTING, (DWORD)FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		*err = (int)GetLastError();
	return (INT_PTR)hFile;
}

static UINT FAR DIAMONDAPI fnFileRead(INT_PTR hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)
{
	DWORD dwBytesRead;
	if (!ReadFile((HANDLE)hf, memory, cb, &dwBytesRead, NULL))
	{
		dwBytesRead = (DWORD)-1;
		*err = (int)GetLastError();
	}
	return dwBytesRead;
}

static UINT FAR DIAMONDAPI fnRead(INT_PTR hf, void FAR *memory, UINT cb)
{
	DWORD dwBytesRead;
	if (!ReadFile((HANDLE)hf, memory, cb, &dwBytesRead, NULL))
	{
		dwBytesRead = (DWORD)-1;
	}
	return dwBytesRead;
}

static UINT FAR DIAMONDAPI fnFileWrite(INT_PTR hf, void FAR *memory, UINT cb, int FAR *err, void FAR *pv)
{
	DWORD dwBytesWritten;
	if (!WriteFile((HANDLE)hf, memory, cb, &dwBytesWritten, NULL))
	{
		dwBytesWritten = (DWORD)-1;
		*err = (int)GetLastError();
	}
	return dwBytesWritten;
}

static UINT FAR DIAMONDAPI fnWrite(INT_PTR hf, void FAR *memory, UINT cb)
{
	DWORD dwBytesWritten;
	if (!WriteFile((HANDLE)hf, memory, cb, &dwBytesWritten, NULL))
	{
		dwBytesWritten = (DWORD)-1;
	}
	return dwBytesWritten;
}

static int FAR DIAMONDAPI fnFileClose(INT_PTR hf, int FAR *err, void FAR *pv)
{
	INT iResult = 0;
	if (!CloseHandle((HANDLE)hf))
	{
		*err = (int)GetLastError();
		iResult = -1;
	}
	return iResult;
}

static int FAR DIAMONDAPI fnClose(INT_PTR hf)
{
	INT iResult = 0;
	if (!CloseHandle((HANDLE)hf))
	{
		iResult = -1;
	}
	return iResult;
}

static long FAR DIAMONDAPI fnFileSeek(INT_PTR hf, long dist, int seektype, int FAR *err, void FAR *pv)
{
	INT iResult = 0;
	iResult = (INT)SetFilePointer((HANDLE)hf, dist, NULL, (DWORD)seektype);
	if (iResult == INVALID_SET_FILE_POINTER)
		*err = (int)GetLastError();
	return iResult;
}

static long FAR DIAMONDAPI fnSeek(INT_PTR hf, long dist, int seektype)
{
	INT iResult = 0;
	iResult = (INT)SetFilePointer((HANDLE)hf, dist, NULL, (DWORD)seektype);
	return iResult;
}

static int FAR DIAMONDAPI fnFileDelete(IN LPSTR pszFile, int FAR *err, void FAR *pv)
{
	INT iResult = 0;
	if (!DeleteFileA(pszFile))
	{
		*err = (int)GetLastError();
		iResult = -1;
	}
	return iResult;
}

static BOOL DIAMONDAPI fnGetTempFileName(OUT char *pszTempName, IN int cbTempName, void FAR *pv)
{
	CHAR pszTempPath[MAX_PATH], pszTempFile[MAX_PATH];
	if (GetTempPathA(MAX_PATH, pszTempPath))
	{
		if (GetTempFileNameA(pszTempPath, "CABINET", 0, pszTempFile) != 0)
		{
			DeleteFileA(pszTempFile);
			strcpy_s(pszTempName, (size_t)cbTempName, pszTempFile);
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL DIAMONDAPI fnGetNextCabinet(PCCAB pccab, ULONG cbPrevCab, void FAR *pv)
{
	return TRUE;
}

static long DIAMONDAPI fnStatus(UINT typeStatus, ULONG cb1, ULONG cb2, void FAR *pv)
{
	return 0;
}

static INT_PTR DIAMONDAPI fnGetOpenInfo(IN LPSTR pszName, USHORT *pdate, USHORT *ptime, USHORT *pattribs, int FAR *err, void FAR *pv)
{
	HANDLE hFile;
	FILETIME fileTime;
	BY_HANDLE_FILE_INFORMATION fileInfo;
	hFile = (HANDLE)fnFileOpen(pszName, _O_RDONLY, 0, err, pv);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (GetFileInformationByHandle(hFile, &fileInfo) && FileTimeToLocalFileTime(&fileInfo.ftCreationTime, &fileTime) && FileTimeToDosDateTime(&fileTime, pdate, ptime))
		{
			*pattribs = (USHORT)fileInfo.dwFileAttributes;
			*pattribs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
		}
		else
		{
			fnFileClose((INT_PTR)hFile, err, pv);
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	return (INT_PTR)hFile;
}

static INT_PTR DIAMONDAPI fnNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin)
{
	if (fdint == fdintCOPY_FILE)
	{
		char filename[MAX_PATH] = { 0 };
		strcpy_s(filename, (const char *)pfdin->pv);
		strcat_s(filename, "\\");
		strcat_s(filename, pfdin->psz1);
		return (INT_PTR)CreateFileA(filename, GENERIC_WRITE, (DWORD)FILE_SHARE_READ, NULL, CREATE_ALWAYS, (DWORD)FILE_ATTRIBUTE_NORMAL, NULL);
	}
	else if (fdint == fdintCLOSE_FILE_INFO)
	{
		CloseHandle((HANDLE)pfdin->hf);
		return TRUE;
	}

	return 0;
}

static bool createCabinet(const StringRef &filename, const StringRef &filenameInCab, const StringRef &cabinetFilename, Vector<HeapString> &log)
{
	CCAB ccab = { 0 };
	ccab.cb = 1024 * 1024 * 1024;
	ccab.cbFolderThresh = 1024 * 1024 * 1024;
	ccab.iCab = 1;
	memcpy(ccab.szCab, cabinetFilename.getPointer(), cabinetFilename.getLength());

	ERF erf = { 0 };
	HFCI hfci = FCICreate(&erf, fnFilePlaced, fnMemAlloc, fnMemFree, fnFileOpen, fnFileRead, fnFileWrite, fnFileClose, fnFileSeek, fnFileDelete, fnGetTempFileName, &ccab, NULL);
	if (hfci == NULL)
	{
		log.pushBack().doSprintf("FCICreate failed: %d", erf.erfOper);
		return false;
	}

	if (!FCIAddFile(hfci, (LPSTR)filename.getPointer(), (LPSTR)filenameInCab.getPointer(), FALSE, fnGetNextCabinet, fnStatus, fnGetOpenInfo, tcompTYPE_LZX | tcompLZX_WINDOW_HI))
	{
		log.pushBack().doSprintf("FCIAddFile failed: %d", erf.erfOper);
		FCIDestroy(hfci);
		return false;
	}

	if (!FCIFlushCabinet(hfci, FALSE, fnGetNextCabinet, fnStatus))
	{
		log.pushBack().doSprintf("FCIFlushCabinet failed: %d", erf.erfOper);
		FCIDestroy(hfci);
		return false;
	}

	FCIDestroy(hfci);
	return true;
}

static bool extractCabinet(const StringRef &cabinetFilenameParam, const StringRef &destinationPath, Vector<HeapString> &log)
{
	TempString cabinetFilename(cabinetFilenameParam);
	cabinetFilename.replace("/", "\\");
	SizeType pathSplit = cabinetFilename.findRight("\\");
	if (!pathSplit)
	{
		log.pushBack().doSprintf("Failed to parse path %s", cabinetFilename.getPointer());
		return false;
	}
	pathSplit += 1;
	TempString cabinetPath(cabinetFilename.getPointer(), pathSplit);
	TempString cabinetName(cabinetFilename.getPointer() + pathSplit);
	
	ERF erf = { 0 };
	HFDI hfdi = FDICreate(fnMemAlloc, fnMemFree, fnOpen, fnRead, fnWrite, fnClose, fnSeek, -1, &erf);
	if (hfdi == NULL)
	{
		log.pushBack().doSprintf("FDICreate failed: %d", erf.erfOper);
		return false;
	}

	if (!FDICopy(hfdi, (LPSTR)cabinetName.getPointer(), (LPSTR)cabinetPath.getPointer(), 0, fnNotify, NULL, (void *)destinationPath.getPointer()))
	{
		log.pushBack().doSprintf("FDICopy failed: %d", erf.erfOper);
		FDIDestroy(hfdi);
		return false;
	}

	FDIDestroy(hfdi);
	return true;
}

FB_END_PACKAGE1()
