#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/file.h>


struct Disk {
    uint32_t descriptor;
    size_t blocks; //numarul de blocuri disponibile pe fisier
    size_t writes;
    size_t reads;
    bool mounted;

public:
    static size_t BLOCK_SIZE; //= 4096;

    Disk(){};
    ~Disk();

    void disk_open(const char *path, size_t nblocks, size_t block_size = 4096);
    void disk_close();

    size_t size() const { return blocks; }


    bool so_read(size_t blocknum, char *data, size_t size);
    bool so_write(size_t blocknum, char *data, size_t size);
};

#endif // DISK_H