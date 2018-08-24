#ifndef HOOKDLL_H
#define HOOKDLL_H

#include <Windows.h>
#ifdef _DLL_SAMPLE
#define DLL_SAMPLE_API __declspec(dllexport)
#else
#define DLL_SAMPLE_API __declspec(dllimport)
#endif
//#define WM_HOOKMSG WM_USER+162
extern "C" {
	DLL_SAMPLE_API int setHook(HWND hwnd, DWORD dwThreadId);
	DLL_SAMPLE_API int removeHook();
	struct CopyMessage
	{
		TCHAR copySource[512];
		TCHAR copyAction[64];
		TCHAR copyText[2048];
	};
}
#undef DLL_SAMPLE_API


#endif