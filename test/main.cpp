#include "../framework/threadsafe_queue.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

void thread_func(int, int)
{
    std::this_thread::sleep_for(1s);
}

void thread_func(double)
{
    std::this_thread::sleep_for(1s);
}

int main()
{
    std::thread t1(thread_func, 4, 3);
    std::thread t2(thread_func, 3.14);

    //Threadsafe_Queue<int> queue;
    //queue.push(105);
    //queue.push(107);
    //queue.push(109);
    //queue.push(135);

    //auto res0 = queue.try_pop();
    //auto res1 = queue.try_pop();
    //auto res2 = queue.try_pop();
    //auto res3 = queue.try_pop();
    //auto res4 = queue.try_pop();
    //auto res5 = queue.try_pop();

    return 0;
}
