#include <iostream>
#include <atomic>
#include <thread>

void proc(std::atomic<int> &x, int limit, int order)
{
    limit -= 1 - order; 
    while (x < limit) {
        if (x % 2 == order) {
            if (order)
                std::cout << "pong" << std::endl;
            else
                std::cout << "ping" << std::endl;
            ++x;
        }
    }
}

int
main()
{
    int N = 1000000;
    std::atomic<int> x(0);
    std::thread t1 = std::thread(proc, std::ref(x), N, 0);
    proc(x, N, 1);
    t1.join();
    return 0; 
}
