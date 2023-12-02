#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string.h>
#include "disk_driver.h"

const static uint32_t MAX_FILENAME_LENGTH = 100;
const static uint32_t MAGIC_NUMBER = 0x05112002;

struct SuperBlock {
    uint32_t MagicNumber;
    uint32_t Blocks;
    uint32_t InodeBlocks;
    uint32_t Inodes;
    };

//un i-node ocupa 128 de octeti
struct Inode {
    uint32_t Valid;
    uint32_t Size;
    uint32_t *Direct; //[POINTERS_PER_INODE];
    uint32_t Indirect;

    uint32_t OwnerUserID;
    uint32_t OwnerGroupID;

    uint32_t Permissions;

    char Filename[MAX_FILENAME_LENGTH];
};

struct statDetails{
    uint32_t Valid;
    uint32_t Size;
    int32_t OwnerUserID;
    uint32_t OwnerGroupID;
    uint32_t Permissions;
};

union Block {
    SuperBlock  Super;
    Inode       *Inodes; //[INODES_PER_BLOCK];
    uint32_t    *Pointers; //[POINTERS_PER_BLOCK];
    char        *Data; //[Disk::BLOCK_SIZE];
};

class FileSystem {
public:
    static uint32_t INODES_PER_BLOCK;   //= 32;
    static uint32_t POINTERS_PER_INODE; //= 5;
    static uint32_t POINTERS_PER_BLOCK; //= 1024;

    static bool *bitmap;
    static Block *superBlock;
    static Block *inodeBlocks;

    static void debugInodes(Block block); //pentru debbuging
    static size_t ceilDiv(size_t a, size_t b);
    static bool writeBlock(Disk *disk, int blocknum, Block* block);
    static bool readBlock(Disk *disk, int blocknum, Block* block);
public:
    FileSystem(Disk *disk);
    ~FileSystem();

    static void debug(Disk *disk);
    static bool format(Disk *disk);

    bool mount(Disk *disk);
    bool getInode(size_t);

    ssize_t create(uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions);
    bool    remove(size_t inumber);
    statDetails stat(size_t inumber);

    ssize_t read(size_t inumber, char *data, size_t length, size_t offset);
    ssize_t write(size_t inumber, char *data, size_t length, size_t offset);
};
#endif