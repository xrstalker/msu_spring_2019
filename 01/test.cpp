#include <cstdio>
#include <iostream>
#include <algorithm>
#include <vector>
/* Data, Size */
#include "numbers.dat"


#define MAX_N 100000

/* Primality test for numbers in range [0, MAX_N] using sieve of Eratosthenes */
class Primes {
    int *prime;
    int n;
public:
    Primes(int max_prime)
    {
        n = max_prime;
        prime = new int[n+1];
        std::fill(prime, &prime[n+1], 1);
        prime[0] = 0;
        prime[1] = 0;

        for (int i = 4; i <= n; i += 2)
            prime[i] = 0;
        
        for (int p = 3; p <= n; p += 2)
            if (prime[p] == 1)
                for (int i = 2*p; i <= n; i += p)
                    prime[i] = 0;
    }

    ~Primes()
    {
        delete[] prime;
    }

    int 
    operator[](int i) 
    {
        /* we could add border check */
        return prime[i];
    }
};

int 
parse_args(int argc, char *argv[], std::vector<int> &v)
{
    if (argc % 2 != 1 || argc == 1)
        return 0;
    
    int x;
    int read;
    for (int i = 1; i < argc; i++) {
        int read = std::sscanf(argv[i], "%d", &x);
        if (read != 1) {
            return 0;
        }
        v.push_back(x);
    }
    return argc-1;
}

int
main(int argc, char *argv[])
{
    std::vector<int> v;
    Primes prime(MAX_N);

    if (!parse_args(argc, argv, v)) {
        return -1;
    }

    for (int i = 0; i < argc/2; ++i) {
        int l = v[i*2];
        int r = v[i*2 + 1];
        int res = 0;
        auto beg = std::lower_bound(Data, Data+Size, l);
        auto end = std::upper_bound(Data, Data+Size, r);

        if (beg == Data+Size || beg[0] != l || l < 0
              || end == Data || end[-1] != r || r > MAX_N) {
            std::cout << 0 << std::endl;
            continue;
        }
        
        for (auto i = beg; i < end; ++i) {
            res += prime[*i];
        }
        std::cout << res << std::endl;
    }
}
