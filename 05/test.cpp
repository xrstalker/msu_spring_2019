#include <iostream>
#include <thread>

void proc(volatile int *x, int limit, int order)
{
    limit -= 1 - order; 
    while (*x < limit) {
        if (*x % 2 == order) {
            if (order)
                std::cout << "pong" << std::endl;
            else
                std::cout << "ping" << std::endl;
            ++*x;
        }
    }
}

int
main()
{
    int N = 1000000;
    volatile int x = 0;
    std::thread t1 = std::thread(proc, &x, N, 0);
    proc(&x, N, 1);
    t1.join();
    return 0; 
}
