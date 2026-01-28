#pragma once

#include <Windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <memory>

class Common_Dialog_Boxes
{
public:
    Common_Dialog_Boxes(HWND hwnd);

    bool open();
    bool copy();
    void cleanup();

private:
    struct File_Copy_Data
    {
        std::wstring src_filename;
        std::wstring dst_filename;
        LARGE_INTEGER size;
        HANDLE src_handle;
        HANDLE dst_handle;
    };

    struct IOData : OVERLAPPED
    {
        HANDLE hSrc, hDst;
        std::unique_ptr<BYTE[]> Buffer;
        ULONGLONG Size;
    };

    std::vector<File_Copy_Data> copy_entries;
 
    OPENFILENAME filename;

    wchar_t m_file[4096];
    wchar_t m_filetitle[1024];

    DWORD worker_thread();
};

