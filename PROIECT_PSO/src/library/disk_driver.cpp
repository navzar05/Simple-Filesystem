#include "../../includes/disk_driver.h"

size_t Disk::BLOCK_SIZE = 0;

Disk::~Disk(){
    close(this->descriptor);
}

uint32_t Disk::datalen(char *data)
{
    uint32_t len = 0;
    while(data[len] != 0)
        len++;
    return len;
}

void Disk::disk_open(const char *path, size_t nblocks, size_t block_size)
{
    if (block_size % 512 != 0) {
        fprintf(stderr, "Bad block size. It must be a multiple of 512. (block_size = %d)\n", block_size);
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

bool Disk::so_read(int blocknum, char *data)
{
    int readbytes = 0;
    if (blocknum > blocks) {
        fprintf(stderr, "Failed to read data. (blocknum %d, max. index %d)\n", blocknum, blocks);
        return -1;
    }

    if (data == nullptr){
        fprintf(stderr, "Nothing to write.\n");
        return -1;
    }

    lseek(this->descriptor, Disk::BLOCK_SIZE * blocknum, SEEK_SET);
    //printf("%d\n", lseek(this->descriptor, 0, SEEK_CUR));
    if (readbytes = read(this->descriptor, data, Disk::BLOCK_SIZE) < 0)
        fprintf(stderr, "Failed to read block from disk. (blocknum: %d)\n", blocknum);
    printf("%d\n", readbytes);
    return 0;
}

bool Disk::so_write(int blocknum, char *data){

    if (blocknum > blocks) {
        fprintf(stderr, "Failed to write data. (blocknum %d, max. index %d)\n", blocknum, blocks);
        return -1;
    }

    fprintf(stdout, "Data len: %d\n", Disk::datalen(data));
    lseek(this->descriptor, this->BLOCK_SIZE * blocknum, SEEK_SET);
    if (write(this->descriptor, data, Disk::BLOCK_SIZE) < 0){
        fprintf(stderr, "Failed to write to block from disk. (blocknum: %d)\n", blocknum);
    }
    return 0;
}