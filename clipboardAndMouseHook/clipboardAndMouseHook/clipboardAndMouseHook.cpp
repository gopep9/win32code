// clipboardAndMouseHook.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CommonFiles/CmnHdr.h"
#include "../HookDll/HookDll.h"
#include "resource.h"
#include <windowsx.h>
HINSTANCE g_hinstance;

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

void InsertLineToListView(const TCHAR *from, const TCHAR *message, const TCHAR *lparam, const TCHAR *wparam,HWND hWnd)
{
	LVITEM lvI = {};
	HWND listViewHwnd = GetDlgItem(hWnd, IDC_LIST1);
	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;
	lvI.iItem = ListView_GetItemCount(listViewHwnd);
	lvI.iImage = ListView_GetItemCount(listViewHwnd);
	int ret = ListView_InsertItem(listViewHwnd, &lvI);
	ListView_SetItemText(listViewHwnd, ret, 0, (LPTSTR)from);
	ListView_SetItemText(listViewHwnd, ret, 1, (LPTSTR)message);
	ListView_SetItemText(listViewHwnd, ret, 2, (LPTSTR)lparam);
	ListView_SetItemText(listViewHwnd, ret, 3, (LPTSTR)wparam);

	ListView_EnsureVisible(listViewHwnd, ListView_GetItemCount(listViewHwnd) - 1, FALSE);
}

BOOL Dlg_OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam) {
	HWND listViewHwnd = GetDlgItem(hWnd, IDC_LIST1);
	TCHAR szText[256] = _T("aaaaa");
	LVCOLUMN lvc = {};
	int iCol;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText =(LPWSTR)_T("来源");
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	int ret=ListView_InsertColumn(listViewHwnd, 0, &lvc);
	lvc.iSubItem = 1;
	lvc.pszText = (LPWSTR)_T("消息");
	ret = ListView_InsertColumn(listViewHwnd, 1, &lvc);
	lvc.iSubItem = 2;
	lvc.pszText = (LPWSTR)_T("lparam");
	ret = ListView_InsertColumn(listViewHwnd, 2, &lvc);
	lvc.iSubItem = 3;
	lvc.pszText = (LPWSTR)_T("wparam");
	ret = ListView_InsertColumn(listViewHwnd, 3, &lvc);
	//TCHAR from[256] = _T("aa");

	//InsertLineToListView(_T("aaaaa"), _T("aaaaa"), _T("aaaaa"), _T("aaaaa"), hWnd);
	//InsertLineToListView(from, from, from, from, hWnd);
	//InsertLineToListView(from, from, from, from, hWnd);


	//初始化的时候设置钩子
	//setHook(hWnd, GetCurrentThreadId());
	return TRUE;
}

void Dlg_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify) {
	switch (id)
	{
	case IDC_SET_HOOK_BUTTON:{
		DWORD pid = GetDlgItemInt(hWnd, IDC_EDIT_PID, NULL, FALSE);
		if (0 == pid)
		{
			printf("pid=0\n");
			// 假如是0的话设置钩子在当前的程序里面
			setHook(hWnd, GetCurrentThreadId());
			return;
		}
		//获取pid对应的主tid
		HWND targetHwnd = getHwndByPid(pid);
		if (targetHwnd == NULL)
		{
			printf("targetHwnd=0\n");
			return;
		}
		DWORD tid=GetWindowThreadProcessId(targetHwnd, NULL);
		if (tid == NULL)
		{
			printf("tid=0\n");
			return;
		}
		setHook(hWnd, tid);
		//setHook(hWnd);
		break;
	}
	case IDC_DEL_HOOK_BUTTON:
		removeHook();
		break;
	case IDCANCEL:
		removeHook();
		EndDialog(hWnd, id);
		break;
	default:
		break;
	}
}

BOOL Dlg_OnCopyData(HWND hWnd, HWND hWndFrom, PCOPYDATASTRUCT pcds) {

	CopyMessage *copyMessage = (CopyMessage*)pcds->lpData;
	
	InsertLineToListView(copyMessage->copySource, copyMessage->copyAction, copyMessage->copyText, _T(""), hWnd);
	return TRUE;
}

INT_PTR WINAPI Dlg_Proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
		chHANDLE_DLGMSG(hWnd, WM_INITDIALOG, Dlg_OnInitDialog);
		chHANDLE_DLGMSG(hWnd, WM_COMMAND, Dlg_OnCommand);
		chHANDLE_DLGMSG(hWnd, WM_COPYDATA, Dlg_OnCopyData);
		//HANDLE_WM_COPYDATA;
	default:
		break;
	}
	return FALSE;
}

int main()
{
	g_hinstance = GetModuleHandle(NULL);
	DialogBox(g_hinstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, Dlg_Proc);
    return 0;
}

