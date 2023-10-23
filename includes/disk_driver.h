#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

struct Disk {
    static uint32_t descriptor;
    static size_t Blocks;
    static uint8_t Mounts;
    
    u_int32_t datalen(char *data);

public:
    const static size_t BLOCK_SIZE = 4096;

    Disk(){};
    ~Disk();

    void so_open(const char *path, size_t nblocks);

    size_t size() const { return Blocks; }

    bool mounted() const { return Mounts > 0; }
    void mount() { Mounts++; }
    void unmount() { if (Mounts > 0) Mounts--; }

    void so_read(int blocknum,char *data);
    void so_write(int blocknum,char *data);
};