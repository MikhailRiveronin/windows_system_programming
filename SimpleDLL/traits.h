#pragma once

#include "pch.h"
#include "Simple.h"
#include <algorithm>

namespace WSP
{
    template <typename Traits>
    class Unique_Handle
    {
    public:
        using Pointer = typename Traits::Pointer;

        explicit Unique_Handle(Pointer value = Traits::invalid()) noexcept : m_value(value)
        {
        }

        Unique_Handle(Unique_Handle const&) = delete;
        Unique_Handle& operator=(Unique_Handle const&) = delete;

        Unique_Handle(Unique_Handle&& other) noexcept : m_value{ other.release() }
        {
        }

        Unique_Handle& operator=(Unique_Handle&& other) noexcept
        {
            if (this != &other)
            {
                reset(other.release());
            }

            return *this;
        }

        ~Unique_Handle() noexcept
        {
            close();
        }

        explicit operator bool() const noexcept
        {
            return m_value != Traits::invalid();
        }

        Pointer get() const noexcept
        {
            return m_value;
        }

        Pointer release() noexcept
        {
            auto value = m_value;
            m_value = Traits::invalid();
            return value;
        }

        bool reset(Pointer value = Traits::invalid()) noexcept
        {
            if (m_value != value)
            {
                close();
                m_value = value;
            }

            return static_cast<bool>(*this);
        }

        void swap(Unique_Handle<Traits>& other) noexcept
        {
            std::swap(m_value, other.m_value);
        }

    private:
        Pointer m_value;

        void close() noexcept
        {
            if (*this)
            {
                Traits::close(m_value);
            }
        }
    };

    template <typename Traits>
    void swap(Unique_Handle<Traits>& left, Unique_Handle<Traits>& right) noexcept
    {
        TRACE(L"swap\n");

        left.swap(right);
    }

    template <typename Traits>
    bool operator==(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() == right.get();
    }

    template <typename Traits>
    bool operator!=(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() != right.get();
    }

    template <typename Traits>
    bool operator<(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() < right.get();
    }

    template <typename Traits>
    bool operator>=(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() >= right.get();
    }

    template <typename Traits>
    bool operator>(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() > right.get();
    }

    template <typename Traits>
    bool operator<=(Unique_Handle<Traits> const& left, Unique_Handle<Traits> const& right) noexcept
    {
        return left.get() <= right.get();
    }

    struct Invalid_Handle_Traits
    {
        using Pointer = HANDLE;

        static Pointer invalid() noexcept
        {
            return INVALID_HANDLE_VALUE;
        }

        static void close(Pointer value) noexcept
        {
            VERIFY(CloseHandle(value));
        }
    };

    struct Null_Handle_Traits
    {
        using Pointer = HANDLE;

        static Pointer invalid() noexcept
        {
            return nullptr;
        }

        static void close(Pointer value) noexcept
        {
            VERIFY(CloseHandle(value));
        }
    };

    using Invalid_Handle = Unique_Handle<Invalid_Handle_Traits>;
    using Null_Handle = Unique_Handle<Null_Handle_Traits>;
}
