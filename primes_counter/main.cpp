#include <Windows.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

int calculate_all_primes(int up_to, int thread_count, DWORD& elapsed);
DWORD WINAPI calculate_primes(PVOID data);
bool is_prime(int i);

struct Thread_Data
{
    int from;
    int to;
    int primes_count;
    DWORD tid;
};

int main()
{
    constexpr const int up_to = 1000000;
    constexpr const int thread_count = 4;
    DWORD elapsed;
    int result = calculate_all_primes(up_to, thread_count, elapsed);
    std::cout << "Result " << result << "\n";

    return 0;
}

int calculate_all_primes(int up_to, int thread_count, DWORD& elapsed)
{
    auto thread_data = std::make_unique<Thread_Data[]>(thread_count);
    auto thread_handles = std::make_unique<HANDLE[]>(thread_count);
    int chunk_size = up_to / thread_count;

    auto start = GetTickCount64();

    for (int i = 0; i < thread_count; ++i)
    {
        auto& data = thread_data[i];
        data.from = i * chunk_size;
        data.to = i != (thread_count - 1) ? (i + 1) * chunk_size - 1 : up_to;
        DWORD tid;
        thread_handles[i] = CreateThread(nullptr, 0, calculate_primes, &data, 0, &tid);
        assert(thread_handles[i]);
        std::cout << "Thread " << tid << " created\n";
    }

    WaitForMultipleObjects(thread_count, thread_handles.get(), TRUE, INFINITE);

    FILETIME kernel;
    FILETIME user;
    FILETIME dummy;
    int result = 0;
    for (int i = 0; i < thread_count; ++i)
    {
        GetThreadTimes(thread_handles[i], &dummy, &dummy, &kernel, &user);
        CloseHandle(thread_handles[i]);
        std::cout << "Thread " << thread_data[i].tid << ". Count " << thread_data[i].primes_count << ". Time " << (kernel.dwLowDateTime + user.dwLowDateTime) / 10000 << "ms\n";
        result += thread_data[i].primes_count;
    }

    elapsed = static_cast<DWORD>(GetTickCount64() - start);

    return result;
}

DWORD WINAPI calculate_primes(PVOID data)
{
    auto info = (Thread_Data*)data;
    int count = 0;
    for (int i = info->from; i <= info->to; ++i)
    {
        if (is_prime(i))
            count++;
    }
    info->primes_count = count;
    info->tid = GetCurrentThreadId();

    return count;
}

bool is_prime(int i)
{
    if (i < 2)
        return false;

    if (i == 2)
        return true;

    int root = static_cast<int>(sqrt(i));
    for (int j = 2; j <= root; ++j)
    {
        if ((i % j) == 0)
            return false;
    }

    return true;
}
