// clipboardAndMouseHook.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../CommonFiles/CmnHdr.h"
#include "../HookDll/HookDll.h"
#include "resource.h"
#include <windowsx.h>
HINSTANCE g_hinstance;

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

	return TRUE;
}

void Dlg_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT codeNotify) {
	switch (id)
	{
	case IDC_SET_HOOK_BUTTON:
		setHook(hWnd);
		break;
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

	InsertLineToListView(_T(""), (PCTSTR)pcds->lpData, _T(""), _T(""), hWnd);
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

