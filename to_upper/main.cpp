#include <windows.h>
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: to_upper <filename>, ...\n";
        return 0;
    }

    wchar_t out_filename[256];

    for (int i = 1; i < argc; ++i)
    {
        __try
        {
            wsprintf(out_filename, L"uc_%s", argv[i]);
            HANDLE in_handle = INVALID_HANDLE_VALUE;
            HANDLE out_handle = INVALID_HANDLE_VALUE;
            char* buffer = nullptr;

            __try
            {
                in_handle = CreateFile(argv[i], GENERIC_READ, 0, 0, OPEN_EXISTING, 0, NULL);
                if (in_handle == INVALID_HANDLE_VALUE)
                {
                    std::cout << "Error occured processing " << argv[i] << std::endl;
                    DWORD exception_code = 1;
                    RaiseException((0x0FFFFFFF | exception_code) & 0xE0000000, 0, 0, 0);
                }

                out_handle = CreateFile(out_filename, GENERIC_WRITE, 0, 0, CREATE_NEW, 0, NULL);
                if (out_handle == INVALID_HANDLE_VALUE)
                {
                    std::cout << "Error occured creating " << out_filename << std::endl;
                    DWORD exception_code = 1;
                    RaiseException((0x0FFFFFFF | exception_code) & 0xE0000000, 0, 0, 0);
                }

                LARGE_INTEGER file_size;
                GetFileSizeEx(in_handle, &file_size);
                buffer = new char[file_size.LowPart + 1];

                DWORD bytes_read;
                if (!ReadFile(in_handle, buffer, (DWORD)file_size.LowPart, &bytes_read, 0))
                {
                    std::cout << "Error occured while reading " << argv[i] << std::endl;
                    DWORD exception_code = 1;
                    RaiseException((0x0FFFFFFF | exception_code) & 0xE0000000, 0, 0, 0);
                }

                buffer[bytes_read] = '\0';

                int wide_char_size = MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, 0, 0);
                if (wide_char_size == 0)
                {
                    std::cout << "Error occured converting chars" << std::endl;
                    DWORD exception_code = 1;
                    RaiseException((0x0FFFFFFF | exception_code) & 0xE0000000, 0, 0, 0);
                }

                wchar_t* wide_buffer = new wchar_t[wide_char_size];
                MultiByteToWideChar(CP_UTF8, 0, buffer, bytes_read, wide_buffer, wide_char_size);

                for (int i = 0; i < wide_char_size; ++i)
                {
                    if (iswalpha(wide_buffer[i]))
                    {
                        wide_buffer[i] = towupper(wide_buffer[i]);
                    }
                }

                if (!WriteFile(out_handle, wide_buffer, wide_char_size * sizeof(wchar_t), 0, 0))
                {
                    std::cout << "Error occured while writing " << out_filename << std::endl;
                    DWORD exception_code = 1;
                    RaiseException((0x0FFFFFFF | exception_code) & 0xE0000000, 0, 0, 0);
                }
            }
            __finally
            {
                if (buffer)
                {
                    delete buffer;
                }

                if (in_handle != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(in_handle);
                    in_handle = INVALID_HANDLE_VALUE;
                }
                if (out_handle != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(out_handle);
                    out_handle = INVALID_HANDLE_VALUE;
                }
                memset(out_filename, 0, sizeof(out_filename));
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            std::cout << "Error occured processing " << argv[i] << std::endl;
        }
    }

    std::cout << "Processing complete\n";
    return 0;
}
