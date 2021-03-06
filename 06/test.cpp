#include <iostream>
#include <string>
#include <thread>
#include <array>
#include <exception>

#include <cstdio>
#include <cstdint>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


class Error: public std::exception {
    std::string message;
public:
    Error(const std::string &message): message(message) { }
    virtual const char *what() const noexcept
    {
        return message.c_str();
    }
};


class FileError: public Error {
public:
    FileError(const std::string &message): Error(std::string("FileError: ") + message) { }
};


class MapError: public Error {
public:
    MapError(const std::string &message): Error(std::string("MapError: ") + message) { }
};

class MemoryMapping {
    std::string filename;
    int fd;
    size_t size;
    void *ptr;
    int mmap_opt;
    bool remove;
public:
    MemoryMapping(const std::string &filename, int file_opt, int mmap_opt, size_t supp_size=0, int file_mode=0, bool remove=false):
        filename(filename), ptr(nullptr), mmap_opt(mmap_opt), remove(remove)
    {
        if (file_mode) {
            fd = open(filename.c_str(), file_opt, file_mode);
        } else {
            fd = open(filename.c_str(), file_opt);
        }
        if (fd == -1) {
            throw FileError(std::string("Can't open file '") + filename + "'. errno=" + std::to_string(errno));
        }

        if (supp_size) {
            set_size(supp_size);
        } else {
            size = lseek(fd, 0, SEEK_END);
        }
        if (size % sizeof(uint64_t)) {
            close(fd);
            throw FileError(std::string("Invalid size of file '") + filename + "'. errno=" + std::to_string(errno));
        }
        
        ptr = mmap(nullptr, size || 1, mmap_opt, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            close(fd);
            throw MapError(std::string("Can't mmap file '") + filename + "'. errno=" + std::to_string(errno));
        }
    }

    void set_size(size_t new_size)
    {
        int res = ftruncate(fd, new_size);
        if (res == -1) {
            throw FileError("Can't truncate file '" + filename + "'. errno=" + std::to_string(errno));
        }
        if (ptr != nullptr || ptr != MAP_FAILED) {
            munmap(ptr, size || 1);
        }
        size = new_size;
        ptr = mmap(nullptr, size, mmap_opt, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            throw MapError(std::string("Can't mmap file '") + filename + "'. errno=" + std::to_string(errno));
        }
    }

    template<typename T=uint64_t*>
    T get_ptr() 
    {
        return static_cast<T>(ptr);
    }

    size_t get_size()
    {
        return size;
    }

    size_t get_num()
    {
        return size / sizeof(uint64_t);
    }

    ~MemoryMapping()
    {
        if (ptr != nullptr && ptr != MAP_FAILED) {
            munmap(ptr, size || 1);
        }
        close(fd);
        if (remove) {
            std::remove(filename.c_str());
        }
    }
};

class InputMapping: public MemoryMapping {
public:
    InputMapping(const std::string &filename):
        MemoryMapping(filename, O_RDONLY, PROT_READ)
    {

    }
};

class OutputMapping: public MemoryMapping {
public:
    OutputMapping(const std::string &filename, size_t supp_size):
        MemoryMapping(filename, O_RDWR | O_CREAT, PROT_WRITE, supp_size, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)
    {

    }

};

class TempMapping: public MemoryMapping {
public:
    TempMapping(const std::string &filename, size_t supp_size=0):
        MemoryMapping(filename, O_RDWR | O_CREAT, PROT_WRITE, supp_size, S_IRUSR | S_IWUSR, true)
    {

    }
};

void split_file(MemoryMapping &part1_m, MemoryMapping &part2_m, MemoryMapping &src_m)
{
    size_t num = src_m.get_num();
    
    size_t s1 = (num+1) / 2 * 8;
    part1_m.set_size(s1);
    std::memcpy(part1_m.get_ptr(), src_m.get_ptr(), s1);
    
    size_t s2 = num / 2 * 8;
    part2_m.set_size(s2);
    std::memcpy(part2_m.get_ptr(), src_m.get_ptr<char*>() + s1, s2);
}

void sort_file(MemoryMapping &dst_m, MemoryMapping &src_m)
{
    uint64_t *src = src_m.get_ptr();
    uint64_t *dst = dst_m.get_ptr();
    dst_m.set_size(src_m.get_size());
    size_t num = src_m.get_num();
    size_t step = 1;
    while (step < num) {
        size_t s = 0;
        size_t p = 0;
        while (s < num) {
            size_t i = 0;
            size_t j = step;
            while (i < step && j < 2*step && s+j < num) {
                if (src[s+i] < src[s+j])
                    dst[p++] = src[s + i++];
                else
                    dst[p++] = src[s + j++];
            }
            while (i < step) {
                dst[p++] = src[s + i++];
            }
            while (j < 2*step && s+j < num) {
                dst[p++] = src[s + j++];
            }
            s += 2 * step;
        }
        std::swap(dst, src);
        step *= 2;
    }
    /* move results after sorting to dst file */
    /* because of last swap, current result in src */
    if (src != dst_m.get_ptr()) {
        std::memcpy(dst_m.get_ptr(), src, dst_m.get_size());
    }
}


void merge_two_sorted_files(MemoryMapping &dst_m, MemoryMapping &part1_m, MemoryMapping &part2_m)
{
    uint64_t *dst = dst_m.get_ptr();
    uint64_t *p1 = part1_m.get_ptr();
    uint64_t *p2 = part2_m.get_ptr();
    size_t n1 = part1_m.get_num();
    size_t n2 = part2_m.get_num();
    size_t i = 0;
    size_t j = 0;
    size_t k = 0;
    while (i < n1 && j < n2) {
        if (p1[i] < p2[j])
            dst[k++] = p1[i++];
        else
            dst[k++] = p2[j++];
    }
    while (i < n1)
        dst[k++] = p1[i++];
    while (j < n2)
        dst[k++] = p2[j++];
}

int main(int argc, char *argv[])
{
    if (argc != 2 && argc != 3) {
        std::cerr << "Invalid arguments. Usage: test infile [outfile]" << std::endl;
        return 1;
    }

    const char *in_fn = argv[1];
    const char *out_fn = argv[1];
    if (argc == 3) out_fn = argv[2];

    try {
        InputMapping in(in_fn);
        OutputMapping out(out_fn, in.get_size());
        
        TempMapping s1("file1.tmp");
        TempMapping s2("file2.tmp");
        TempMapping d1("file3.tmp");
        TempMapping d2("file4.tmp");

        split_file(s1, s2, in);

        std::thread second_thread = std::thread(sort_file, std::ref(d1), std::ref(s1));
        sort_file(d2, s2);
        second_thread.join();

        merge_two_sorted_files(out, d1, d2);
    }
    catch (Error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
