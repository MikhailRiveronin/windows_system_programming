#include "pch.h"
#include "Simple.h"

Manual_Reset_Event::Manual_Reset_Event()
{
    handle = CreateEvent(nullptr, true, false, nullptr);
    if (!handle)
    {
        throw Last_Exception();
    }
}

Manual_Reset_Event::~Manual_Reset_Event()
{
    VERIFY(CloseHandle(handle));
}

#ifdef _DEBUG
void trace(wchar_t const* format, ...)
{
    va_list args;
    va_start(args, format);

    wchar_t buffer[256];

    ASSERT(-1 != _vsnwprintf_s(buffer, _countof(buffer) - 1, format, args));

    va_end(args);

    OutputDebugString(buffer);
}
#endif
