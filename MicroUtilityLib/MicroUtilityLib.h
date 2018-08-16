#pragma once
#ifndef MICROUTILITYLIB_API
#define MICROUTILITYLIB_API __declspec(dllimport)
#endif

//Enable Win32 Visual Styles
#ifdef _M_IX86
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include <Windows.h>

#define MAX_DRIVE_NUMBER 26
#define MAX_FILENAME UNICODE_STRING_MAX_CHARS

#define ToLowercaseA(ch) (char)ToLowercaseW(ch)
#define ToLowercaseW(ch) (((wchar_t)(ch) >= L'A' && (wchar_t)(ch) <= L'Z') ? ((wchar_t)(ch) | 0x20) : (wchar_t)(ch))
#define ToUpperCaseA(ch) (char)ToUppercaseW(ch)
#define ToUpperCaseW(ch) (((wchar_t)(ch) >= L'a' && (wchar_t)(ch) <= L'z') ? ((wchar_t)(ch) & 0xdf) : (wchar_t)(ch))
#define MAKEDWORD(HIGH_WORD, LOW_WORD) ((DWORD)((((DWORD)(HIGH_WORD)) << 16) | (DWORD)(LOW_WORD)))
#define MAKEDWORD64(HIGH_DWORD, LOW_DWORD) ((DWORD64)((((DWORD64)(HIGH_DWORD)) << 32) | (DWORD64)(LOW_DWORD)))

#define ScaleX(iX, iCurrentDpi_X) MulDiv(iX, iCurrentDpi_X, USER_DEFAULT_SCREEN_DPI)
#define ScaleY(iY, iCurrentDpi_Y) MulDiv(iY, iCurrentDpi_Y, USER_DEFAULT_SCREEN_DPI)
#define DPIAware_CreateWindowExW(iCurrentDpi_X, iCurrentDpi_Y, dwExStyle, lpClassName, lpWindowName, dwStyle, iX, iY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam) CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, ScaleX(iX, iCurrentDpi_X), ScaleY(iY, iCurrentDpi_Y), ScaleX(nWidth, iCurrentDpi_X), ScaleY(nHeight, iCurrentDpi_Y), hWndParent, hMenu, hInstance, lpParam)

//Return PROGRESS_STOP to stop function FindDiskDataW
typedef DWORD(CALLBACK* LPFIND_DISK_DATA_ROUTINEW)(LPWIN32_FIND_DATAW lpWin32_FindData, LPWSTR lpwPathName, LPVOID lpData);
MICROUTILITYLIB_API BOOL FindDiskDataW(WCHAR wchDriveLetter, LPVOID lpData, LPFIND_DISK_DATA_ROUTINEW lpFindDiskData_Routine);

MICROUTILITYLIB_API BOOL DownloadFileFromInternet(LPCWSTR lpcwUrl, LPCWSTR lpcwNewFileName);
MICROUTILITYLIB_API BOOL GetFileProductVersion(LPCWSTR lpcwFileName, LPWSTR lpwFileProductVersionBuffer, DWORD cbFileProductVersionBufferSize);
MICROUTILITYLIB_API DWORD GetFileCRC32W(LPCWSTR lpcwFileName);
MICROUTILITYLIB_API INT GetCurrentDpiX(VOID);
MICROUTILITYLIB_API INT GetCurrentDpiY(VOID);
MICROUTILITYLIB_API VOID DeleteSelf(VOID);