#ifndef HOOKDLL_H
#define HOOKDLL_H

#include <Windows.h>
#ifdef _DLL_SAMPLE
#define DLL_SAMPLE_API __declspec(dllexport)
#else
#define DLL_SAMPLE_API __declspec(dllimport)
#endif
#define WM_MOUSE_HOOK WM_USER+160

extern "C"{
	DLL_SAMPLE_API int setMouseHook(HWND hwnd);
	DLL_SAMPLE_API int removeMouseHook();
}

#undef DLL_SAMPLE_API
#endif