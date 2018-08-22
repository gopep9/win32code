// HookDll.cpp: 定义 DLL 应用程序的导出函数。
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "HookDll.h"
#include "APIHook.h"
#include <Windows.h>
#include <WinNT.h>
#include <ObjIdl.h>
#include <WindowsX.h>

#pragma data_seg("SHARED")
static HHOOK g_hhook = NULL;
//static HINSTANCE glhInstance = NULL;
static HWND g_hWnd = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:SHARED,rws")

typedef HANDLE(WINAPI *PGetClipboardData)(_In_ UINT uFormat);
typedef HANDLE (WINAPI *PSetClipboardData)(_In_ UINT uFormat, _In_opt_ HANDLE hMem);
typedef HRESULT (WINAPI *POleSetClipboard)(_In_ LPDATAOBJECT pDataObj);
typedef HRESULT (WINAPI *POleGetClipboard)(_Out_ LPDATAOBJECT *ppDataObje);


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

int setHook(HWND hwnd) {

	BOOL bOk;
	g_hWnd = hwnd;
	g_hhook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, ModuleFromAddress(setHook), 0);
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
CAPIHook g_GetClipboardData("User32.dll", "GetClipboardData", (PROC)MyGetClipboardData);
CAPIHook g_SetClipboardData("User32.dll", "SetClipboardData", (PROC)MySetClipboardData);
CAPIHook g_OleSetClipboard("Ole32.dll", "OleSetClipboard", (PROC)MyOleSetClipboard);
CAPIHook g_OleGetClipboard("Ole32.dll", "OleGetClipboard", (PROC)MyOleGetClipboard);

HANDLE WINAPI MyGetClipboardData(_In_ UINT uFormat)
{
	HANDLE nResult = ((PGetClipboardData)(PROC)g_GetClipboardData)(uFormat);
	//先判断uFormat是否是CF_TEXT
	if (CF_UNICODETEXT == uFormat){
		//发送消息给原来的窗口
		TCHAR sz[2048] = TEXT("GetClipboardData");
		LPWSTR pData = (LPWSTR)GlobalLock(nResult);
		int size = wcslen(pData)*sizeof(TCHAR);
		TCHAR szText[2000] = {};
		if (size < 2000)
		{
			CopyMemory(szText, pData, size);
			wcscat(sz, szText);
		}
		COPYDATASTRUCT cds = { 0, ((DWORD)wcslen(sz) + 1)*sizeof(TCHAR), sz };
		FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);
	}

	return nResult;
}

HANDLE WINAPI MySetClipboardData(_In_ UINT uFormat, _In_opt_ HANDLE hMem)
{
	HANDLE nResult = ((PSetClipboardData)(PROC)g_SetClipboardData)(uFormat,hMem);
	TCHAR sz[2048] = TEXT("SetClipboardData");
	COPYDATASTRUCT cds = { 0, ((DWORD)wcslen(sz) + 1)*sizeof(TCHAR), sz };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);

	return nResult;
}

HRESULT WINAPI MyOleSetClipboard(_In_ LPDATAOBJECT pDataObj)
{
	HRESULT nResult = ((POleSetClipboard)(PROC)g_OleSetClipboard)(pDataObj);
	TCHAR sz[2048] = TEXT("OleSetClipboard");
	COPYDATASTRUCT cds = { 0, ((DWORD)wcslen(sz) + 1)*sizeof(TCHAR), sz };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);

	return nResult;
}

HRESULT WINAPI MyOleGetClipboard(_Out_ LPDATAOBJECT *ppDataObje)
{
	HRESULT nResult = ((POleGetClipboard)(PROC)g_OleGetClipboard)(ppDataObje);
	TCHAR sz[2048] = TEXT("OleGetClipboard");
	COPYDATASTRUCT cds = { 0, ((DWORD)wcslen(sz) + 1)*sizeof(TCHAR), sz };
	FORWARD_WM_COPYDATA(g_hWnd, NULL, &cds, SendMessage);

	return nResult;
}
