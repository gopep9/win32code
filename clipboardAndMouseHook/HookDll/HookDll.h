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
	DLL_SAMPLE_API int setHook(HWND hwnd);
	DLL_SAMPLE_API int removeHook();
}
#undef DLL_SAMPLE_API


#endif