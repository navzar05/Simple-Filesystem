#include "../../includes/disk_driver.h"
#include "disk_driver.h"

size_t Disk::BLOCK_SIZE = 0;


void Disk::disk_open(const char *path, size_t nblocks, size_t block_size)
{
    if (block_size % 512 != 0) {
        fprintf(stderr, "Bad block size. It must be a multiple of 512. (block_size = %ld)\n", block_size);
        return;
    }
    this->descriptor = open(path, O_RDWR | O_CREAT, 0666);
    if (this->descriptor < 0){
        fprintf(stderr, "Failed to open diskimage. (path: %s, nblocks: %ld)\n", path, nblocks);
        return;
    }
    this->blocks = nblocks;
    this->BLOCK_SIZE = block_size;
}

bool Disk::so_read(size_t blocknum, char *data, size_t size) {
    if (blocknum >= blocks) {
        fprintf(stderr, "Failed to read data. (blocknum %ld, max. index %ld)\n", blocknum, blocks);
        return -1;
    }

    if (data == nullptr){
        fprintf(stderr, "Nothing to write.\n");
        return -1;
    }

    lseek(this->descriptor, Disk::BLOCK_SIZE * blocknum, SEEK_SET);

    ssize_t readbytes = read(this->descriptor, data, size);
    if (readbytes < 0) {
        fprintf(stderr, "Failed to read block from disk. (blocknum: %ld)\n", blocknum);
        return -1;
    }

/*     if (size < Disk::BLOCK_SIZE) {
        memset(data + size, 0, Disk::BLOCK_SIZE - size); // Zero out the rest
    } */
    return 0;
}


bool Disk::so_write(size_t blocknum, char *data, size_t size) {
    if (blocknum >= blocks) {
        fprintf(stderr, "Failed to write data. (blocknum %ld, max. index %ld)\n", blocknum, blocks);
        return -1;
    }

    lseek(this->descriptor, Disk::BLOCK_SIZE * blocknum, SEEK_SET);

    if (write(this->descriptor, data, size) < 0) {
        fprintf(stderr, "Failed to write to block from disk. (blocknum: %ld)\n", blocknum);
        return -1;
    }

    if (size < Disk::BLOCK_SIZE) {
        // Write 0x00 for the remaining part of the block
        char zeroPadding[Disk::BLOCK_SIZE - size];
        memset(zeroPadding, 0, Disk::BLOCK_SIZE - size);
        write(this->descriptor, zeroPadding, Disk::BLOCK_SIZE - size);
    }

    return 0;
}
