#ifndef MMAP_UTILS_H
#define MMAP_UTILS_H

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <cstring>

class MMapFile {
public:
    MMapFile(const std::string& path, bool write = false)
        : fd(-1), data(nullptr), size(0) {
        int flags = write ? O_RDWR : O_RDONLY;
        fd = open(path.c_str(), flags);
        if (fd == -1) {
            throw std::runtime_error("Failed to open file: " + path);
        }

        size = lseek(fd, 0, SEEK_END);
        if (size == -1) {
            close(fd);
            throw std::runtime_error("Failed to get file size");
        }

        int prot = write ? (PROT_READ | PROT_WRITE) : PROT_READ;
        data = mmap(nullptr, size, prot, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            close(fd);
            throw std::runtime_error("mmap failed: " + std::string(strerror(errno)));
        }
    }

    ~MMapFile() {
        if (data && data != MAP_FAILED) munmap(data, size);
        if (fd != -1) close(fd);
    }

    void* getData() const { return data; }
    size_t getSize() const { return size; }

    void sync(bool blocking = true) {
        if (msync(data, size, blocking ? MS_SYNC : MS_ASYNC) != 0) {
            throw std::runtime_error("msync failed: " + std::string(strerror(errno)));
        }
    }

private:
    int fd;
    void* data;
    size_t size;
};

#endif
