#include <windows.h>
#include <iostream>

BOOL WINAPI console_signal_handler(DWORD event_type);
LONG done;

int main()
{
    std::cout << "main: running\n";
    SetConsoleCtrlHandler(console_signal_handler, TRUE);

    while (!done)
    {
        Sleep(5000);
        Beep(1000, 250);
    }

    std::cout << "main: done incremented\n";
    return 0;
}

BOOL WINAPI console_signal_handler(DWORD event_type)
{
    InterlockedIncrement(&done);

    switch (event_type)
    {
        case CTRL_C_EVENT:
            for (int i = 10; i > 0; --i)
            {
                std::cout << "console_signal_handler: closing in " << i << " sec\n";
                Sleep(1000);
            }
            return TRUE;

        default:
            return FALSE;
    }

    return FALSE;
}
