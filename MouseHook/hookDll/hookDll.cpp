#define _DLL_SAMPLE
#include <Windows.h>
#include "hookDll.h"

#pragma data_seg("SHARED")
static HHOOK _glh_hook_ = NULL;
static HINSTANCE glhInstance = NULL;
static HWND g_hWnd = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:SHARED,rws")

LRESULT CALLBACK MouseProc(int nCode, WPARAM wparam, LPARAM lparam)
{
	/*if (nCode >= 0)
	{
	FILE *fp = fopen("D:\\log.txt","a");
	if (wparam == WM_RBUTTONDOWN){
	fputs("WM_RBUTTONDOWN", fp);
	}
	else if (wparam == WM_LBUTTONDOWN)
	{
	fputs("WM_LBUTTONDOWN",fp);
	}
	else if (wparam == WM_MBUTTONDOWN)
	{
	fputs("WM_MBUTTONDOWN", fp);
	}
	fclose(fp);
	}*/
	if (g_hWnd != NULL&&nCode == HC_ACTION)
	{
		::SendMessage(g_hWnd, WM_MOUSE_HOOK, wparam, lparam);
	}

	return CallNextHookEx(_glh_hook_, nCode, wparam, lparam);
}

int setMouseHook(HWND hwnd)
{
	g_hWnd = hwnd;
	glhInstance = GetModuleHandle(__TEXT("dlltest.dll"));
	HWND hWndLv = FindWindow(TEXT("testWindows"), NULL);
	DWORD ThreadId = GetWindowThreadProcessId(hWndLv, NULL);
	_glh_hook_ = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, glhInstance, NULL);

	return 0;
}

int removeMouseHook()
{
	if (_glh_hook_ != NULL){
		UnhookWindowsHookEx(_glh_hook_);
		_glh_hook_ = NULL;
	}
	return 0;
}