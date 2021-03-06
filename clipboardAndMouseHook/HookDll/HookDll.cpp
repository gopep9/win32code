// HookDll.cpp: 定义 DLL 应用程序的导出函数。
//
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "stdafx.h"
#include "HookDll.h"
#include "APIHook.h"
#include <Windows.h>
#include <WinNT.h>
#include <ObjIdl.h>
#include <WindowsX.h>
#include <stdio.h>
#include <winnt.h>
#include <tchar.h>
#include "../CommonFiles/CmnHdr.h"

#pragma data_seg("SHARED")
static HHOOK g_hhook = NULL;
//static HINSTANCE glhInstance = NULL;
static HWND g_hWnd = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:SHARED,rws")

typedef HANDLE(WINAPI *PGetClipboardData)(_In_ UINT uFormat);
typedef HANDLE(WINAPI *PSetClipboardData)(_In_ UINT uFormat, _In_opt_ HANDLE hMem);
typedef HRESULT(WINAPI *POleSetClipboard)(_In_ LPDATAOBJECT pDataObj);
typedef HRESULT(WINAPI *POleGetClipboard)(_Out_ LPDATAOBJECT *ppDataObje);
static HWND getHwndByPid(DWORD dwProcessID);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		break;
	case DLL_THREAD_ATTACH:

		break;
	case DLL_THREAD_DETACH:

		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

static HMODULE ModuleFromAddress(PVOID pv){
	MEMORY_BASIC_INFORMATION mbi;
	return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
}

static LRESULT WINAPI GetMsgProc(int code, WPARAM wParam, LPARAM lParam){

	return(CallNextHookEx(g_hhook, code, wParam, lParam));
}

//给单个的程序无法设置低级鼠标事件
static LRESULT WINAPI MyLowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
	//低级鼠标事件
	if (nCode == HC_ACTION)
	{
		if (wParam == WM_LBUTTONUP)
		{

			//HWND currentHWND= getHwndByPid(GetCurrentProcessId());
			//DWORD startpos = 0;
			//DWORD endpos = 0;

			//发送测试信息，是否进入到了左键弹起时间
			CopyMessage testMessage = {};
			wcscat(testMessage.copyAction, TEXT("test"));
			COPYDATASTRUCT testcds = { 0, sizeof(testMessage), &testMessage };
			FORWARD_WM_COPYDATA(g_hWnd, NULL, &testcds, SendMessage);

			//先获取鼠标的坐标
			PMSLLHOOKSTRUCT msllhookstruct = (PMSLLHOOKSTRUCT)lParam;
			HWND targetHwnd = WindowFromPoint(msllhookstruct->pt);
			//判断下是否是edit，暂时不判断了
			TCHAR szClassName[MAX_PATH] = {};
			GetClassName(targetHwnd, szClassName, MAX_PATH);
			/*if (wcscmp(szClassName, TEXT("Edit"))!=0)
			return(CallNextHookEx(g_hhook, nCode, wParam, lParam));*/
			const int bufferSize = 1024 * 1 * 1;
			TCHAR *buffer = new TCHAR[bufferSize]{};

			SendMessage(targetHwnd, WM_GETTEXT, 1024, (LPARAM)buffer);

			//在这里打印获取到的信息（字符串）
			memset(&testMessage, 0, sizeof(testMessage));
			wcscat(testMessage.copyAction, TEXT("WM_GETTEXT"));
			wsprintf(testMessage.copyText, _T("%s"), buffer);
			testcds = { 0, sizeof(testMessage), &testMessage };
			FORWARD_WM_COPYDATA(g_hWnd, NULL, &testcds, SendMessage);

			if (wcscmp(buffer, TEXT("")) == 0){
				memset(&testMessage, 0, sizeof(testMessage));
				wcscat(testMessage.copyAction, TEXT("test1"));
				testcds = { 0, sizeof(testMessage), &testMessage };
				FORWARD_WM_COPYDATA(g_hWnd, NULL, &testcds, SendMessage);
				return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
			}
			DWORD startpos = 0;
			DWORD endpos = 0;
			DWORD position = SendMessage(targetHwnd, EM_GETSEL, (WPARAM)&startpos, (LPARAM)&endpos);
			if ((startpos == endpos) || (startpos > bufferSize) || (endpos > bufferSize)){

				memset(&testMessage, 0, sizeof(testMessage));
				wcscat(testMessage.copyAction, TEXT("test2"));
				wsprintf(testMessage.copyText, _T("startpos %d endpos %d"), startpos, endpos);
				testcds = { 0, sizeof(testMessage), &testMessage };
				FORWARD_WM_COPYDATA(g_hWnd, NULL, &testcds, SendMessage);
				return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
			}
			CopyMessage copyMessage = {};
			wcscat(copyMessage.copyAction, TEXT("WM_LBUTTONUP"));
			TCHAR *bufferSelect = new TCHAR[bufferSize]{};
			CopyMemory(bufferSelect, buffer + startpos, (endpos - startpos)*sizeof(TCHAR));
			wcscat(copyMessage.copyText, bufferSelect);
			COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
			FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
			delete[] buffer;
			delete[] bufferSelect;
		}
	}
	return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
}

static LRESULT WINAPI MyMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	/*CopyMessage copyMessage = {};
	wcscat(copyMessage.copyAction, TEXT("WM_LBUTTONUP"));
	COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);*/

	if (nCode == HC_ACTION)
	{
		if (wParam == WM_LBUTTONUP)
		{
			PMOUSEHOOKSTRUCT msllhookstruct = (PMOUSEHOOKSTRUCT)lParam;
			int cursorPosX = msllhookstruct->pt.x;
			int cursorPosY = msllhookstruct->pt.y;
			HWND targetHwnd = WindowFromPoint(msllhookstruct->pt);
			TCHAR buffer[1024] = {};
			SendMessage(targetHwnd, WM_GETTEXT, 1024, (LPARAM)buffer);
			if (wcscmp(buffer, TEXT("")) == 0)
				return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
			DWORD startpos = 0;
			DWORD endpos = 0;
			DWORD position = SendMessage(targetHwnd, EM_GETSEL, (WPARAM)&startpos, (LPARAM)&endpos);
			if (startpos == endpos)
				return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
			CopyMessage copyMessage = {};
			wcscat(copyMessage.copyAction, TEXT("WM_LBUTTONUP"));
			TCHAR bufferSelect[1024] = {};
			CopyMemory(bufferSelect, buffer + startpos, (endpos - startpos)*sizeof(TCHAR));
			wcscat(copyMessage.copyText, bufferSelect);
			COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
			FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
		}
	}
	return(CallNextHookEx(g_hhook, nCode, wParam, lParam));
}

int setHook(HWND hwnd, DWORD dwThreadId) {

	BOOL bOk;
	g_hWnd = hwnd;
	//改为设置拦截鼠标事件
	//WH_MOUSE_LL加载的没办法获取复制的数据
	//g_hhook = SetWindowsHookEx(WH_MOUSE_LL, MyLowLevelKeyboardProc, ModuleFromAddress(setHook), 0);
	g_hhook = SetWindowsHookEx(WH_MOUSE, MyMouseProc, ModuleFromAddress(setHook), dwThreadId);
	bOk = (g_hhook != NULL);
	return bOk;
}

int removeHook() {
	BOOL bOk = FALSE;
	if (g_hhook != 0)
	{
		bOk = UnhookWindowsHookEx(g_hhook);
		g_hhook = NULL;
	}
	return bOk;
}

HANDLE WINAPI MyGetClipboardData(_In_ UINT uFormat);

HANDLE WINAPI MySetClipboardData(_In_ UINT uFormat, _In_opt_ HANDLE hMem);

HRESULT WINAPI MyOleSetClipboard(_In_ LPDATAOBJECT pDataObj);

HRESULT WINAPI MyOleGetClipboard(_Out_ LPDATAOBJECT *ppDataObje);


//设置替换几个函数的关键段
//效果不好，在Chrome，sublime等程序里面并没有捕捉到事件，尝试直接在被hook的程序里面弹窗
CAPIHook g_GetClipboardData("User32.dll", "GetClipboardData", (PROC)MyGetClipboardData);
CAPIHook g_SetClipboardData("User32.dll", "SetClipboardData", (PROC)MySetClipboardData);
CAPIHook g_OleSetClipboard("Ole32.dll", "OleSetClipboard", (PROC)MyOleSetClipboard);
CAPIHook g_OleGetClipboard("Ole32.dll", "OleGetClipboard", (PROC)MyOleGetClipboard);

HANDLE WINAPI MyGetClipboardData(_In_ UINT uFormat)
{
	HANDLE nResult = ((PGetClipboardData)(PROC)g_GetClipboardData)(uFormat);
	CopyMessage copyMessage = {};
	wcscat(copyMessage.copyAction, TEXT("GetClipboardData"));
	LPWSTR pData = (LPWSTR)GlobalLock(nResult);
	if ((CF_UNICODETEXT == uFormat) && (pData != NULL)){
		int size = wcslen(pData)*sizeof(TCHAR);
		if (size < (2048)*sizeof(TCHAR))
		{
			CopyMemory(copyMessage.copyText, pData, size);
		}
	}
	//TCHAR moduleFileName[MAX_PATH] = {};
	GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
	COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
	return nResult;
}

HANDLE WINAPI MySetClipboardData(_In_ UINT uFormat, _In_opt_ HANDLE hMem)
{
	HANDLE nResult = ((PSetClipboardData)(PROC)g_SetClipboardData)(uFormat, hMem);
	CopyMessage copyMessage = {};
	wcscat(copyMessage.copyAction, TEXT("SetClipboardData"));
	LPWSTR pData = (LPWSTR)GlobalLock(nResult);
	if ((CF_UNICODETEXT == uFormat) && (pData != NULL)){
		int size = wcslen(pData)*sizeof(TCHAR);
		if (size < (2048)*sizeof(TCHAR))
		{
			CopyMemory(copyMessage.copyText, pData, size);
		}
	}
	GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
	COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
	return nResult;
}

HRESULT WINAPI MyOleSetClipboard(_In_ LPDATAOBJECT pDataObj)
{
	HRESULT nResult = ((POleSetClipboard)(PROC)g_OleSetClipboard)(pDataObj);
	CopyMessage copyMessage = {};
	wcscat(copyMessage.copyAction, TEXT("OleSetClipboard"));
	GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
	COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);

	return nResult;
}

HRESULT WINAPI MyOleGetClipboard(_Out_ LPDATAOBJECT *ppDataObje)
{
	HRESULT nResult = ((POleGetClipboard)(PROC)g_OleGetClipboard)(ppDataObje);
	CopyMessage copyMessage = {};
	wcscat(copyMessage.copyAction, TEXT("OleGetClipboard"));
	GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
	COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);

	return nResult;
}

//进程id转窗口句柄
static HWND getHwndByPid(DWORD dwProcessID)
{
	{
		DWORD dwPID = 0;
		HWND hwndRet = NULL;
		// 取得第一个窗口句柄
		HWND hwndWindow = ::GetTopWindow(0);
		while (hwndWindow)
		{
			dwPID = 0;
			// 通过窗口句柄取得进程ID
			DWORD dwTheardID = ::GetWindowThreadProcessId(hwndWindow, &dwPID);
			if (dwTheardID != 0)
			{
				// 判断和参数传入的进程ID是否相等
				if (dwPID == dwProcessID)
				{
					// 进程ID相等，则记录窗口句柄
					hwndRet = hwndWindow;
					break;
				}
			}
			// 取得下一个窗口句柄
			hwndWindow = ::GetNextWindow(hwndWindow, GW_HWNDNEXT);
		}
		// 上面取得的窗口，不一定是最上层的窗口，需要通过GetParent获取最顶层窗口
		HWND hwndWindowParent = NULL;
		// 循环查找父窗口，以便保证返回的句柄是最顶层的窗口句柄
		while (hwndRet != NULL)
		{
			hwndWindowParent = ::GetParent(hwndRet);
			if (hwndWindowParent == NULL)
			{
				break;
			}
			hwndRet = hwndWindowParent;
		}
		// 返回窗口句柄
		return hwndRet;
	}
}

/*

//新添加捕捉textout
typedef BOOL(WINAPI *PTextOutW)(
HDC     hdc,
int     x,
int     y,
LPCWSTR lpString,
int     c
);
typedef BOOL(WINAPI *PTextOutA)(
HDC    hdc,
int    x,
int    y,
LPCSTR lpString,
int    c
);

typedef BOOL(WINAPI*PExtTextOutW)(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCWSTR    lpString,
UINT       c,
CONST INT  *lpDx
);

typedef BOOL(WINAPI*PExtTextOutA)(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCSTR     lpString,
UINT       c,
CONST INT  *lpDx
);


BOOL MyTextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c);

BOOL MyTextOutW(HDC hdc, int x, int y, LPCWSTR lpString, int c);

BOOL MyExtTextOutW(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCWSTR    lpString,
UINT       c,
CONST INT  *lpDx
);

BOOL MyExtTextOutA(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCSTR     lpString,
UINT       c,
CONST INT  *lpDx
);

//设置一下钩子钩TextOut
CAPIHook g_TextOutA("gdi32.dll", "TextOutA", (PROC)MyTextOutA);

CAPIHook g_TextOutW("gdi32.dll", "TextOutW", (PROC)MyTextOutW);

CAPIHook g_ExtTextOutA("gdi32.dll", "ExtTextOutA", (PROC)MyExtTextOutA);

CAPIHook g_ExtTextOutW("gdi32.dll", "ExtTextOutW", (PROC)MyExtTextOutW);

BOOL MyTextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c)
{
BOOL nResult = ((PTextOutA)(PROC)g_TextOutA)(hdc, x, y, lpString, c);
CopyMessage copyMessage = {};
wcscat(copyMessage.copyAction, TEXT("MyTextOutA"));
GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
return nResult;
}

BOOL MyTextOutW(HDC hdc, int x, int y, LPCWSTR lpString, int c)
{
BOOL nResult = ((PTextOutW)(PROC)g_TextOutW)(hdc, x, y, lpString, c);
CopyMessage copyMessage = {};
wcscat(copyMessage.copyAction, TEXT("MyTextOutW"));
GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
return nResult;
}

BOOL MyExtTextOutW(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCWSTR    lpString,
UINT       c,
CONST INT  *lpDx
)
{
BOOL nResult = ((PTextOutW)(PROC)g_ExtTextOutW)(hdc, x, y, lpString, c);
CopyMessage copyMessage = {};
wcscat(copyMessage.copyAction, TEXT("MyExtTextOutW"));
GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
return nResult;
return TRUE;
}
BOOL MyExtTextOutA(
HDC        hdc,
int        x,
int        y,
UINT       options,
CONST RECT *lprect,
LPCSTR     lpString,
UINT       c,
CONST INT  *lpDx
)
{
BOOL nResult = ((PTextOutA)(PROC)g_ExtTextOutA)(hdc, x, y, lpString, c);
CopyMessage copyMessage = {};
wcscat(copyMessage.copyAction, TEXT("MyExtTextOutA"));
GetModuleFileName(NULL, copyMessage.copySource, MAX_PATH);
COPYDATASTRUCT cds = { 0, sizeof(copyMessage), &copyMessage };
FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
return nResult;
return TRUE;
}

*/
