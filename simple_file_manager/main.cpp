#include "common_dialog_boxes.h"
#include "resource.h"
#include <commctrl.h>
#include <stdio.h>

INT_PTR CALLBACK dialog_proc(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    INITCOMMONCONTROLSEX init_common_control = {};
    init_common_control.dwSize = sizeof(init_common_control);
    init_common_control.dwICC = ICC_LISTVIEW_CLASSES;
    if (!InitCommonControlsEx(&init_common_control))
    {
        wchar_t buffer[128];
        swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to initialize common controls\n", GetLastError());
        MessageBox(nullptr, buffer, L"Error", MB_OK);
        return -1;
    }

    DialogBox(nullptr, MAKEINTRESOURCE(IDD_MAIN_DIALOG), nullptr, dialog_proc);

    return 0;
}

INT_PTR CALLBACK dialog_proc(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    static Common_Dialog_Boxes cdb(wnd);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            HWND lv_wnd = GetDlgItem(wnd, IDC_LIST_VIEW);
            ListView_SetExtendedListViewStyle(lv_wnd, LVS_EX_FULLROWSELECT);
            ListView_SetExtendedListViewStyle(lv_wnd, LVS_EX_DOUBLEBUFFER);
            ListView_SetExtendedListViewStyle(lv_wnd, LVS_EX_INFOTIP);

            LVCOLUMN column = {};
            column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            column.fmt = LVCFMT_LEFT;
            column.cx = 350;
            column.pszText = const_cast<wchar_t*>(L"Source");
            column.iSubItem = 0;
            if (ListView_InsertColumn(lv_wnd, 0, &column) == -1)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to insert column\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return static_cast<INT_PTR>(FALSE);
            }

            column.cx = 100;
            column.pszText = const_cast<wchar_t*>(L"Size");
            column.iSubItem = 1;
            if (ListView_InsertColumn(lv_wnd, 1, &column) == -1)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to insert column\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return static_cast<INT_PTR>(FALSE);
            }

            column.cx = 250;
            column.pszText = const_cast<wchar_t*>(L"Destination");
            column.iSubItem = 2;
            if (ListView_InsertColumn(lv_wnd, 2, &column) == -1)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to insert column\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return static_cast<INT_PTR>(FALSE);
            }



            return static_cast<INT_PTR>(TRUE);
        }

        case WM_COMMAND:
            switch (LOWORD(w_param))
            {
                case IDOK:
                    cdb.cleanup();
                    EndDialog(wnd, IDOK);
                    return static_cast<INT_PTR>(TRUE);

                case IDCANCEL:
                    cdb.cleanup();
                    EndDialog(wnd, IDCANCEL);
                    return static_cast<INT_PTR>(TRUE);

                case IDC_ADD_FILES_BUTTON:
                    if (cdb.open())
                        return static_cast<INT_PTR>(TRUE);
                    else
                        return static_cast<INT_PTR>(FALSE);

                case IDC_COPY_BUTTON:   
                    if (cdb.copy())
                        return static_cast<INT_PTR>(TRUE);
                    else
                        return static_cast<INT_PTR>(FALSE);
            }

            return static_cast<INT_PTR>(TRUE);
    }

    return static_cast<INT_PTR>(FALSE);
}
