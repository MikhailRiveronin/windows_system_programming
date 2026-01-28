#include "common_dialog_boxes.h"
#include "resource.h"
#include <commctrl.h>
#include <cassert>

#define READ_COMPLETION_KEY 0
#define WRITE_COMPLETION_KEY 1

Common_Dialog_Boxes::Common_Dialog_Boxes(HWND hwnd) : filename{}
{
    filename.lStructSize = sizeof(filename);
    filename.hwndOwner = hwnd;
    filename.lpstrFilter = L"All Files (*.*)\0*.*\0\0";
    filename.lpstrFile = m_file;
    filename.nMaxFile = 4096;
    filename.lpstrFileTitle = m_filetitle;
    filename.nMaxFileTitle = 1024;
}

bool Common_Dialog_Boxes::open()
{
    filename.Flags = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
    if (GetOpenFileName(&filename) == TRUE)
    {
        wchar_t* file_title;
        std::wstring directory;
        bool single_file = filename.nFileExtension != 0;
        if (single_file)
        {
            file_title = filename.lpstrFile;
        }
        else
        {
            directory = filename.lpstrFile;
            file_title = filename.lpstrFile + directory.length() + 1;
        }

        int i = 0;
        HWND lv_wnd = GetDlgItem(filename.hwndOwner, IDC_LIST_VIEW);
        while (*file_title)
        {
            std::wstring full_path;
            if (single_file)
                full_path = file_title;
            else
                full_path = directory + L"\\" + file_title;

            LVITEM item = {};
            item.mask = LVIF_TEXT;
            item.pszText = (wchar_t*)full_path.c_str();
            item.iItem = i;
            item.iSubItem = 0;
            if (ListView_InsertItem(lv_wnd, &item) == -1)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to open files\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return false;
            }

            size_t extension_pos = full_path.find_first_of('.');
            std::wstring dst_filename = full_path.substr(0, extension_pos) + L"_copy" + full_path.substr(extension_pos);

            HANDLE file = CreateFile(full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
            if (!file)
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to create a handle to file\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return false;
            }

            LARGE_INTEGER size;
            if (!GetFileSizeEx(file, &size))
            {
                wchar_t buffer[128];
                swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to get file size\n", GetLastError());
                MessageBox(nullptr, buffer, L"Error", MB_OK);
                return false;
            }
            wchar_t size_str[256] = { 0 };
            _itow_s(size.QuadPart, size_str, 10);
            ListView_SetItemText(lv_wnd, i, 1, size_str);
            wchar_t* dst_buffer = const_cast<wchar_t*>(dst_filename.c_str());
            ListView_SetItemText(lv_wnd, i, 2, dst_buffer);

            File_Copy_Data copy_data;
            copy_data.src_filename = file_title;
            copy_data.src_handle = file;
            copy_data.size = size;
            copy_data.dst_filename = dst_filename;
            copy_entries.push_back(copy_data);

            file_title += lstrlen(file_title) + 1;
            i++;
        }

        return true;
    }

    wchar_t buffer[128];
    swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to open files\n", CommDlgExtendedError());
    MessageBox(nullptr, buffer, L"Error", MB_OK);

    return false;
}

bool Common_Dialog_Boxes::copy()
{
    HANDLE thread = CreateThread(nullptr, 0, [](auto param)
        {
            return ((Common_Dialog_Boxes*)param)->worker_thread();
        }, this, 0, nullptr);
    if (!thread)
    {
        wchar_t buffer[128];
        swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to create thread\n", GetLastError());
        MessageBox(nullptr, buffer, L"Error", MB_OK);
        return false;
    }
    CloseHandle(thread);

    return true;
}

void Common_Dialog_Boxes::cleanup()
{

}

DWORD Common_Dialog_Boxes::worker_thread()
{
    HANDLE cp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!cp)
    {
        wchar_t buffer[128];
        swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to create io completion port\n", GetLastError());
        MessageBox(nullptr, buffer, L"Error", MB_OK);
        return -1;
    }

    for (auto data : copy_entries)
    {
        data.dst_handle = CreateFile(data.dst_filename.c_str(), GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr);
        if (!data.dst_handle)
        {
            wchar_t buffer[128];
            swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to create a handle to file\n", GetLastError());
            MessageBox(nullptr, buffer, L"Error", MB_OK);
            return false;
        }

        SetFilePointerEx(data.dst_handle, data.size, nullptr, FILE_BEGIN);
        SetEndOfFile(data.dst_handle);

        HANDLE temp = CreateIoCompletionPort(data.src_handle, cp, (ULONG_PTR)READ_COMPLETION_KEY, 0);
        assert(cp == temp);
        temp = CreateIoCompletionPort(data.dst_handle, cp, (ULONG_PTR)WRITE_COMPLETION_KEY, 0);
        assert(cp == temp);

        auto io = new IOData;
        io->Size = data.size.QuadPart;
        io->Buffer = std::make_unique<BYTE[]>(data.size.QuadPart);
        io->hSrc = data.src_handle;
        io->hDst = data.dst_handle;
        ::ZeroMemory(io, sizeof(OVERLAPPED));
        auto ok = ReadFile(io->hSrc, io->Buffer.get(), data.size.QuadPart, nullptr, io);
        if (!ok)
            assert(GetLastError() == ERROR_IO_PENDING);
    }

    size_t count = copy_entries.size();

    while (count > 0)
    {
        DWORD transferred;
        ULONG_PTR key;
        OVERLAPPED* ov;
        BOOL ok = GetQueuedCompletionStatus(cp, &transferred, &key, &ov, INFINITE);
        if (!ok)
        {
            wchar_t buffer[128];
            swprintf_s(buffer, _countof(buffer), L"Error %d: Failed to get queued completion status\n", GetLastError());
            MessageBox(nullptr, buffer, L"Error", MB_OK);
            return false;
        }

        auto io = static_cast<IOData*>(ov);

        if (key == (DWORD_PTR)READ_COMPLETION_KEY)
        {
            io->Internal = io->InternalHigh = 0;
            ok = WriteFile(io->hDst, io->Buffer.get(), transferred, nullptr, ov);
            if (!ok)
                assert(GetLastError() == ERROR_IO_PENDING);

        }
        else
        {
            count--;
            delete io;
        }
    }

    CloseHandle(cp);

    wchar_t buffer[128];
    swprintf_s(buffer, _countof(buffer), L"Copy complete\n");
    MessageBox(nullptr, buffer, L"Ok", MB_OK);

    return 0;
}
