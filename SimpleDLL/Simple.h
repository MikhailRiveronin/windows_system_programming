#pragma once

#include "pch.h"

#define ASSERT _ASSERTE

#ifdef _DEBUG
#define VERIFY ASSERT
#else
#define VERIFY __noop
#endif

struct Last_Exception
{
    DWORD result;

    Last_Exception() : result(GetLastError())
    {
    }
};

#ifdef SIMPLEDLL_EXPORTS
#define SIMPLEDLL_API __declspec(dllexport)
#else
#define SIMPLEDLL_API __declspec(dllimport)
#endif

struct SIMPLEDLL_API Manual_Reset_Event
{
    HANDLE handle;

    Manual_Reset_Event();
    ~Manual_Reset_Event();
};

#ifdef _DEBUG
SIMPLEDLL_API void trace(wchar_t const* format, ...);

struct Tracer
{
    char const* m_filename;
    unsigned m_line;

    Tracer(char const* filename, unsigned const line) :
        m_filename{ filename },
        m_line{ line }
    {

    }

    template <typename... Args>
    auto operator()(wchar_t const* format, Args... args) const -> void
    {
        wchar_t buffer[256];

        auto count = swprintf_s(buffer,
            L"%S(%d): ",
            m_filename,
            m_line);

        ASSERT(-1 != count);

        ASSERT(-1 != _snwprintf_s(buffer + count,
            _countof(buffer) - count,
            _countof(buffer) - count - 1,
            format,
            args...));

        OutputDebugString(buffer);
    }
};
#endif

#ifdef _DEBUG
#define TRACE Tracer(__FILE__, __LINE__)
#else
#define TRACE __noop
#endif
