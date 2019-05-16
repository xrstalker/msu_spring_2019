#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>

using count_map = std::map<std::string, uint64_t>;
using sort_pair = std::pair<uint64_t, std::string>;

int
main(int argc, char *argv[])
{
    /* check args and open file */
    if (argc != 2) {
        std::cerr << "Usage: test infile" << std::endl;
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in.is_open()) {
        std::cerr << "Can't open " << argv[1] << std::endl;
        return 1;
    }

    /* count word statistics */
    std::map<std::string, uint64_t> counter;
    std::string word;
    while (in >> word) {
        counter[word] += 1;
    }

    /* sort word statistics */
    std::vector<sort_pair> buf;
    for (auto it = counter.begin(); it != counter.end(); ++it) {
        buf.push_back(sort_pair(it->second, it->first));
    } 
    std::sort(buf.begin(), buf.end());

    /* print results */
    std::cout << "Most frequent words:" << std::endl;
    int cnt = 0;
    for (auto it = buf.rbegin(); it != buf.rend() && cnt < 5; ++cnt, ++it) {
        std::cout << it->second << ' ' << it->first << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "Least frequent words:" << std::endl;
    cnt = 0;
    for (auto it = buf.begin(); it != buf.end() && cnt < 5; ++cnt, ++it) {
        std::cout << it->second << ' ' << it->first << std::endl;
    }
    std::cout << std::endl;

    /* close file */
    in.close();
    return 0;
}
