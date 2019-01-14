//Copyright (C) 2019 Programmer-Yang_Xun@outlook.com

#define MICROUTILITYLIB_API extern __declspec(dllexport)
#include "MicroUtilityLib.h"
#include <Windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <TlHelp32.h>
#include <WinInet.h>
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Wininet.lib")

BOOL DownloadFileFromInternetW(LPCWSTR lpcwUrl, LPCWSTR lpcwNewFileName, BOOL bFailIfFileExists)
{
	DWORD dwFlags;
	if (InternetGetConnectedState(&dwFlags, 0))
	{
		HINTERNET hInternetOpen = InternetOpenW(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (hInternetOpen)
		{
			BOOL bIsSuccess = FALSE;
			HINTERNET hInternetOpenUrl = InternetOpenUrlW(hInternetOpen, lpcwUrl, NULL, 0, INTERNET_FLAG_RELOAD, 0);
			if (hInternetOpenUrl)
			{
				HANDLE hFile = CreateFileW(lpcwNewFileName, GENERIC_WRITE, 0, 0, (bFailIfFileExists) ? (CREATE_NEW) : (CREATE_ALWAYS), FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD dwNumberOfBytesRead, dwNumberOfBytesWritten;
					BYTE byteBuffer[1024];
					while (InternetReadFile(hInternetOpenUrl, byteBuffer, sizeof(byteBuffer), &dwNumberOfBytesRead))
					{
						WriteFile(hFile, byteBuffer, dwNumberOfBytesRead, &dwNumberOfBytesWritten, NULL);
						if (dwNumberOfBytesRead != sizeof(byteBuffer))
						{
							break;
						}
					}
					CloseHandle(hFile);
					bIsSuccess = TRUE;
				}
				InternetCloseHandle(hInternetOpenUrl);
			}
			InternetCloseHandle(hInternetOpen);
			return bIsSuccess;
		}
	}
	return FALSE;
}

BOOL FindDiskDataW(WCHAR wchDriveLetter, LPVOID lpData, LPFIND_DISK_DATA_ROUTINEW lpFindDiskData_Routine)
{
	HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
	if (hHeap)
	{
		LPWSTR lpwNewFileName = (LPWSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, MAX_UNICODE_PATH * sizeof(WCHAR)),
			lpwPathName = (LPWSTR)HeapAlloc(hHeap, HEAP_NO_SERIALIZE, MAX_UNICODE_PATH * sizeof(WCHAR));
		if (lpwNewFileName == NULL || lpwPathName == NULL)
		{
			HeapDestroy(hHeap);
			return FALSE;
		}
		wnsprintfW(lpwPathName, MAX_UNICODE_PATH, L"\\\\?\\%lc:", wchDriveLetter);
		struct _DATA {
			INT iLengthOfPathName;
			HANDLE hFindFile;
			struct _DATA *Previous, *Next;
		} FindData, *pFindData = &FindData;
		FindData.Previous = NULL;
		WIN32_FIND_DATAW Win32_FindData;
		*Win32_FindData.cFileName = 0;
		Win32_FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		while (TRUE)
		{
			if (*Win32_FindData.cFileName != L'.' || (Win32_FindData.cFileName[1] && (Win32_FindData.cFileName[1] != L'.' || Win32_FindData.cFileName[2])))
			{
				if (Win32_FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if ((pFindData->Next = (struct _DATA*)HeapAlloc(hHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(FindData))) == NULL)
					{
						DWORD dwLastErrorCode = GetLastError();
						while (TRUE)
						{
							FindClose(pFindData->hFindFile);
							if ((pFindData = pFindData->Previous) == &FindData)
							{
								HeapDestroy(hHeap);
								SetLastError(dwLastErrorCode);
								return TRUE;
							}
						}
					}
					pFindData->Next->Previous = pFindData;
					pFindData->Next->iLengthOfPathName = wnsprintfW(lpwPathName, MAX_UNICODE_PATH, L"%ls%ls\\*.*", lpwPathName, Win32_FindData.cFileName) - 3;
					pFindData->Next->hFindFile = FindFirstFileW(lpwPathName, &Win32_FindData);
					lpwPathName[pFindData->Next->iLengthOfPathName] = 0;
					pFindData = pFindData->Next;
					if (pFindData->Previous == &FindData)
					{
						continue;
					}
				}
				else
				{
					wnsprintfW(lpwNewFileName, MAX_UNICODE_PATH, L"%ls%ls", lpwPathName, Win32_FindData.cFileName);
				}
				if (lpFindDiskData_Routine && lpFindDiskData_Routine(&Win32_FindData, lpwPathName, lpData) == PROGRESS_STOP)
				{
					while (TRUE)
					{
						FindClose(pFindData->hFindFile);
						if ((pFindData = pFindData->Previous) == &FindData)
						{
							HeapDestroy(hHeap);
							return TRUE;
						}
					}
				}
			}
			while (!FindNextFileW(pFindData->hFindFile, &Win32_FindData))
			{
				FindClose(pFindData->hFindFile);
				if ((pFindData = pFindData->Previous) == &FindData)
				{
					HeapDestroy(hHeap);
					return TRUE;
				}
				HeapFree(hHeap, HEAP_NO_SERIALIZE, pFindData->Next);
				lpwPathName[pFindData->iLengthOfPathName] = 0;
			}
		}
	}
	else
	{
		return FALSE;
	}
}

BOOL GetFileProductVersionW(LPCWSTR lpcwFileName, LPWSTR lpwFileProductVersionBuffer, DWORD cchFileProductVersionBuffer)
{
	BOOL bSuccess = FALSE;
	DWORD dwFileVersionInfoSize = GetFileVersionInfoSizeW(lpcwFileName, NULL);
	if (lpwFileProductVersionBuffer && cchFileProductVersionBuffer && dwFileVersionInfoSize)
	{
		HANDLE hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
		if (hHeap)
		{
			PVOID pvFileVersionInfo = HeapAlloc(hHeap, HEAP_NO_SERIALIZE, dwFileVersionInfoSize);
			if (pvFileVersionInfo && GetFileVersionInfoW(lpcwFileName, 0, dwFileVersionInfoSize, pvFileVersionInfo))
			{
				struct {
					WORD wLanguage, wCodePage;
				} *lpTranslate;
				UINT cbTranslationSize;
				if (VerQueryValueW(pvFileVersionInfo, L"\\VarFileInfo\\Translation", &lpTranslate, &cbTranslationSize) && cbTranslationSize / sizeof(*lpTranslate) != 0)
				{
					UINT cbFileProductVersionSize;
					LPWSTR lpwFileProductVersion;
					WCHAR szSubBlock[50];
					wnsprintfW(szSubBlock, _countof(szSubBlock), L"\\StringFileInfo\\%04X%04X\\ProductVersion", lpTranslate->wLanguage, lpTranslate->wCodePage);
					if (VerQueryValueW(pvFileVersionInfo, szSubBlock, (PVOID*)&lpwFileProductVersion, &cbFileProductVersionSize))
					{
						wnsprintfW(lpwFileProductVersionBuffer, cchFileProductVersionBuffer, lpwFileProductVersion);
						bSuccess = TRUE;
					}
				}
			}
			HeapDestroy(hHeap);
		}
	}
	return bSuccess;
}

BOOL SortStringsLogicalW(LPWSTR *lpwStrings, DWORD dwNumberOfStrings)
{
	if (lpwStrings == NULL)
	{
		return FALSE;
	}
	BOOL bIsSwapped;
	do {
		bIsSwapped = FALSE;
		for (DWORD dwIndex = 1; dwIndex < dwNumberOfStrings; dwIndex++)
		{
			INT iComparsionResult = StrCmpLogicalW(lpwStrings[dwIndex - 1], lpwStrings[dwIndex]);
			if (iComparsionResult == 1)
			{
				bIsSwapped = TRUE;
				LPWSTR lpwString = lpwStrings[dwIndex - 1];
				lpwStrings[dwIndex - 1] = lpwStrings[dwIndex];
				lpwStrings[dwIndex] = lpwString;
			}
		}
	} while (bIsSwapped);
	return TRUE;
}

DWORD FindStringInSortedStringsLogicalW(LPCWSTR lpcwStringToFind, LPWSTR *lpwStrings, DWORD dwLowerBound, DWORD dwUpperBound)
{
	if (lpwStrings == NULL)
	{
		return (DWORD)-1;
	}
	while (dwLowerBound <= dwUpperBound)
	{
		DWORD dwMiddle = (dwLowerBound + dwUpperBound) / 2;
		INT iComparsionResult = StrCmpLogicalW(lpcwStringToFind, lpwStrings[dwMiddle]);
		if (iComparsionResult == 1)
		{
			dwLowerBound = dwMiddle + 1;
		}
		else if (iComparsionResult == -1)
		{
			dwUpperBound = dwMiddle - 1;
		}
		else
		{
			return dwMiddle;
		}
	}
	return (DWORD)-1;
}

DWORD GetFileCRC32W(LPCWSTR lpcwFileName)
{
	DWORD dwCRC32 = 0;
	HANDLE hFile = CreateFileW(lpcwFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwCRC32_Table[256];
		for (DWORD dwIndex = 0; dwIndex < 256; dwIndex++)
		{
			dwCRC32_Table[dwIndex] = dwIndex;
			for (BYTE byteIndex = 0; byteIndex < 8; byteIndex++)
			{
				if (dwCRC32_Table[dwIndex] & 0x00000001)
				{
					dwCRC32_Table[dwIndex] = (dwCRC32_Table[dwIndex] >> 1) ^ 0xEDB88320;
				}
				else
				{
					dwCRC32_Table[dwIndex] >>= 1;
				}
			}
		}
		DWORD dwNumberOfBytesRead;
		BYTE byteCRC32Buffer[8 * 1024];
		while (ReadFile(hFile, byteCRC32Buffer, sizeof(byteCRC32Buffer), &dwNumberOfBytesRead, NULL))
		{
			dwCRC32 ^= 0xffffffff;
			DWORD dwBytes = dwNumberOfBytesRead;
			PBYTE pCRC32Buffer = byteCRC32Buffer;
			while (dwBytes--)
			{
				dwCRC32 = (dwCRC32 >> 8) ^ dwCRC32_Table[(dwCRC32 & 0xff) ^ *(pCRC32Buffer++)];
			}
			dwCRC32 ^= 0xffffffff;
			if (dwNumberOfBytesRead != sizeof(byteCRC32Buffer))
			{
				break;
			}
		}
	}
	return dwCRC32;
}

INT GetCurrentDpiX(VOID)
{
	INT iCurrentDpiX = USER_DEFAULT_SCREEN_DPI;
	HDC hDC = GetDC(NULL);
	if (hDC)
	{
		SetProcessDPIAware();
		GetDeviceCaps(hDC, LOGPIXELSX);
		iCurrentDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(NULL, hDC);
	}
	return iCurrentDpiX;
}

INT GetCurrentDpiY(VOID)
{
	INT iCurrentDpiY = USER_DEFAULT_SCREEN_DPI;
	HDC hDC = GetDC(NULL);
	if (hDC)
	{
		SetProcessDPIAware();
		GetDeviceCaps(hDC, LOGPIXELSY);
		iCurrentDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
		ReleaseDC(NULL, hDC);
	}
	return iCurrentDpiY;
}

VOID DeleteSelf(VOID)
{
	WCHAR szCommandLine[MAX_PATH + 20], szProgramFileName[MAX_PATH];
	GetModuleFileNameW(NULL, szProgramFileName, _countof(szProgramFileName));
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, szProgramFileName, NULL);
	wnsprintfW(szCommandLine, _countof(szCommandLine), L"/c del /q \"%ls\"", szProgramFileName);
	ShellExecuteW(NULL, L"open", L"cmd.exe", szCommandLine, NULL, SW_HIDE);
	ExitProcess(0);
}