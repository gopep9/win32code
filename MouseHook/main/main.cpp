#include "../CommonFiles/CmnHdr.h"
#include "../hookDll/hookDll.h"
#include <Windowsx.h>
#include "resource.h"
#include <tchar.h>
#include <winnt.h>
#include <stdio.h>
static HMODULE g_hInst;
static BOOL CHECK_MOVE;
static BOOL CHECK_WHEEL;
BOOL insertListViewColumn(HWND hWndListView, TCHAR *str,int iCol,int width)
{
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = str;
	lvc.cx = width;
	lvc.fmt = LVCFMT_CENTER;
	if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
		return FALSE;
	return TRUE;
}

BOOL insertListViewLine(HWND hWndListView, TCHAR *col1, TCHAR *col2, TCHAR *col3, TCHAR *col4, TCHAR *col5)
{
	LVITEM lvI = {};
	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;
	lvI.iItem = ListView_GetItemCount(hWndListView);
	lvI.iImage = 0;
	int now = 0;
	if ((now=ListView_InsertItem(hWndListView, &lvI)) == -1)
		return FALSE;
	ListView_SetItemText(hWndListView, now, 0, col1);
	ListView_SetItemText(hWndListView, now, 1, col2);
	ListView_SetItemText(hWndListView, now, 2, col3);
	ListView_SetItemText(hWndListView, now, 3, col4);
	ListView_SetItemText(hWndListView, now, 4, col5);
	return TRUE;
}

//按钮
void Dlg_OnCommand(HWND hWnd, int id, HWND hWndCtl, UINT hwparam){
	switch (id){
	case IDCANCEL:
		EndDialog(hWnd, id);
		break;
	case IDC_BUTTON_START:
		setMouseHook(hWnd);
		break;
	case IDC_BUTTON_STOP:
		removeMouseHook();
		break;
	case IDC_CHECK_MOVE:{
		LRESULT r = SendMessage(hWndCtl, BM_GETCHECK, 0, 0);
		//被点击
		if (r == BST_CHECKED)
		{
			CHECK_MOVE = TRUE;
		}
		else if (r == BST_UNCHECKED)//没有被点击
		{
			CHECK_MOVE = FALSE;
		}
		break;
	}
	case IDC_CHECK_WHEEL:{
		LRESULT r = SendMessage(hWndCtl, BM_GETCHECK, 0, 0);
		//被点击
		if (r == BST_CHECKED)
		{
			CHECK_WHEEL = TRUE;
		}
		else if (r == BST_UNCHECKED)//没有被点击
		{
			CHECK_WHEEL = FALSE;
		}
		break;
	}
	}
}

BOOL Dlg_OnInitDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam){
	//在初始化的时候设置listview的列数
	HWND listViewHwnd = GetDlgItem(hWnd, IDC_LIST1);
	//LVCOLUMN lvc;
	//lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	insertListViewColumn(listViewHwnd, _T("wParam"),0,100);
	insertListViewColumn(listViewHwnd, _T("lParam"),1,100);
	insertListViewColumn(listViewHwnd, _T("鼠标消息"), 2,150);
	insertListViewColumn(listViewHwnd, _T("鼠标坐标"), 3,100);
	insertListViewColumn(listViewHwnd, _T("所在窗口"), 4,250);
	ListView_SetExtendedListViewStyle(listViewHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	Button_SetCheck(GetDlgItem(hWnd, IDC_CHECK_MOVE), BST_CHECKED
		);
	Button_SetCheck(GetDlgItem(hWnd, IDC_CHECK_WHEEL), BST_CHECKED);

	CHECK_MOVE=Button_GetCheck(GetDlgItem(hWnd, IDC_CHECK_MOVE));
	CHECK_WHEEL = Button_GetCheck(GetDlgItem(hWnd, IDC_CHECK_WHEEL));
	return TRUE;
}

INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg)
	{
	case(WM_INITDIALOG) : return(SetDlgMsgResult(hwnd,uMsg,HANDLE_WM_INITDIALOG(hwnd,wParam,lParam,Dlg_OnInitDialog)));
	case(WM_COMMAND) : return(SetDlgMsgResult(hwnd, uMsg, HANDLE_WM_COMMAND(hwnd, wParam, lParam, Dlg_OnCommand)));
	case(WM_MOUSE_HOOK) : {
		TCHAR col1[100] = {};
		TCHAR col2[100] = {};
		TCHAR col3[100] = {};
		TCHAR col4[100] = {};
		TCHAR col5[100] = {};
		wsprintf(col1, _T("0x%X"), wParam);
		wsprintf(col2, _T("0x%X"), lParam);

		if (wParam == WM_LBUTTONDBLCLK)	// 鼠标左键按下
		{
			wsprintf(col3, _T("WM_LBUTTONDBLCLK"));
		}
		else if (wParam == WM_LBUTTONUP) // 鼠标左键弹起
		{
			wsprintf(col3, _T("WM_LOBUTTONUP"));
		}
		else if (wParam == WM_RBUTTONDBLCLK) // 鼠标右键按下
		{
			wsprintf(col3, _T("WM_RBUTTONDELCLK"));
		}
		else if (wParam == WM_RBUTTONUP) // 鼠标右键弹起
		{
			wsprintf(col3, _T("WM_RBUTTONUP"));
		}
		else if (wParam == WM_MOUSEMOVE) // 鼠标移动
		{
			//if (!m_bIsShowMouseMoveMsg && m_List.GetItemCount()>0)
			//{
			//	m_List.DeleteItem(m_List.GetItemCount() - 1);
			//	return 1;
			//}
			if (!CHECK_MOVE)
				return FALSE;

			wsprintf(col3, _T("WM_MOUSEMOVE"));
		}
		else if (wParam == WM_MOUSEWHEEL) // 鼠标滚动
		{
			/*if (!m_bIsSHowMouseWheelMsg && m_List.GetItemCount()>0)
			{
				m_List.DeleteItem(m_List.GetItemCount() - 1);
				return 1;
			}*/
			if (!CHECK_WHEEL)
				return FALSE;

			wsprintf(col3, _T("WM_MOUSEWHEEL"));
		}
		static PMSLLHOOKSTRUCT mouseHookStruct;
		mouseHookStruct = (PMSLLHOOKSTRUCT)lParam;
		wsprintf(col4, _T("%d,%d"), mouseHookStruct->pt.x, mouseHookStruct->pt.y);
		
		HWND hwndIn = WindowFromPoint(mouseHookStruct->pt);
		TCHAR title[MAX_PATH];
		GetWindowText(hwndIn, title, MAX_PATH);
		insertListViewLine(GetDlgItem(hwnd, IDC_LIST1), col1, col2, col3, col4, title);
		ListView_EnsureVisible(GetDlgItem(hwnd, IDC_LIST1), ListView_GetItemCount(GetDlgItem(hwnd, IDC_LIST1)) - 1, FALSE);
		break;
	}
	default:
		break;
	}
	return FALSE;
}


int main(int argc, char *argv[])
{
	g_hInst = GetModuleHandle(NULL);
	int ret=DialogBox(g_hInst, MAKEINTRESOURCE(IDD_HOOKWINDOW_DIALOG), NULL, Dlg_Proc);
	return 0;
}