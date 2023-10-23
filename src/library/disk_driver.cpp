#include "../../includes/disk_driver.h"

uint32_t Disk::descriptor = 0;
size_t Disk::Blocks = 0;
uint8_t Disk::Mounts = 0;

Disk::~Disk(){
    close(this->descriptor);
}

uint32_t Disk::datalen(char *data){
    uint32_t len = 0;
    while(data[len] != 0)
        len++;
    return len;
}

void Disk::so_open(const char *path, size_t nblocks){
    this->descriptor = open(path, O_RDWR | O_CREAT);
    if (this->descriptor < 0){
        fprintf(stderr, "Failed to open diskimage. (path: %s, nblocks: %ld)\n", path, nblocks);
        exit(-1);
    }
    this->Blocks = nblocks;
}

void Disk::so_read(int blocknum, char *data){
    if (data == nullptr){
        data = new char[this->BLOCK_SIZE];
    }
    lseek(this->descriptor, this->BLOCK_SIZE * blocknum, SEEK_SET);
    if (read(this->descriptor, data, this->BLOCK_SIZE) < 0){
        fprintf(stderr, "Failed to read block from disk. (blocknum: %d)\n", blocknum);
        delete[] data;
    }
}

void Disk::so_write(int blocknum, char *data){
    fprintf(stdout, "Data len: %d\n", Disk::datalen(data));
    lseek(this->descriptor, this->BLOCK_SIZE * blocknum, SEEK_SET);
    if (write(this->descriptor, data, Disk::datalen(data)) < 0){
        fprintf(stderr, "Failed to write to block from disk. (blocknum: %d)\n", blocknum);
    }
}