#include <iostream>
#include <cstdio>
#include <algorithm>
#include "numbers.dat"

// Data
// Size

const int MAX_N = 100001;

int prime[MAX_N];

void
init_prime()
{
    std::fill(prime, &prime[MAX_N], 1);
    prime[0] = 0;
    prime[1] = 0;
    for (int i = 4; i < MAX_N; i += 2)
        prime[i] = 0;
    for (int p = 3; p < MAX_N; p += 2) {
        if (prime[p] == 1) {
            for (int i = 2*p; i < MAX_N; i += p) {
                prime[i] = 0;
            }
        }
    }
}

int
main(int argc, char *argv[])
{
    if (argc % 2 != 1 || argc == 1)
        return -1;
    
    int *l = new int[argc/2];
    int *r = new int[argc/2];

    for (int i = 0; i < argc/2; i++) {
        int read = std::sscanf(argv[1+2*i], "%d", &l[i]) 
            + std::sscanf(argv[2+2*i], "%d", &r[i]);
        if (!(read = 2 && 0 <= l[i] && r[i] < MAX_N))
            return -1;
    }
    
    init_prime();

    for (int i = 0; i < argc/2; ++i) {
        auto beg = std::upper_bound(Data, Data+Size, l[i]-1);
        auto end = std::upper_bound(Data, Data+Size, r[i]);

        if (beg == Data+Size || beg[0] != l[i] || end == Data || end[-1] != r[i]) {
            std::cout << 0 << std::endl;
            continue;
        }
        
        int res = 0;
        for (auto i = beg; i < end; ++i) {
            res += prime[*i];
        }
        std::cout << res << std::endl;
    }
}
