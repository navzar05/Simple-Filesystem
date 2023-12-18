#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "disk_driver.h"

const static uint32_t MAX_FILENAME_LENGTH = 100 - 16; //de la pointer
const static uint32_t MAGIC_NUMBER = 0x05112002;

struct __attribute__ ((packed)) SuperBlock {
uint32_t MagicNumber;
uint32_t Blocks;
uint32_t InodeBlocks;
uint32_t Inodes;
};

//un i-node ocupa 128 de octeti
struct __attribute__ ((packed)) Inode {
    uint32_t Valid;
    uint32_t Size;
    uint32_t Direct[5];
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

    char Filename[MAX_FILENAME_LENGTH];
};

class FileSystem {
private:
    static uint32_t INODES_PER_BLOCK;   //= 32;
    static uint32_t POINTERS_PER_INODE; //= 5;
    static uint32_t POINTERS_PER_BLOCK; //= 1024;

    static bool *bitmap;
    static char *superBlock;
    static char *inodeBlocks;
    static Disk *mountedDisk;
    static size_t totalInodes;

    static void debugInodes(char* inodeBlock); //pentru debbuging
    static size_t floorDiv(size_t a, size_t b);
    static size_t ceilDiv(size_t a, size_t b);

    static int loadPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int loadDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int loadIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int savePages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int saveDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int saveIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk);
    static int allocBlock(uint32_t *pointer); //Cauta primul bloc gol si seteaza valoarea lui pointer cu indexul lui.

    static size_t getInodeBlockFromInumber(size_t inumber);
    static int initBitmap(const Inode* inodeBlock);

    int ilock(size_t inumber);
    int iunlock(size_t inumber);
public:
    FileSystem(Disk *disk);
    ~FileSystem();

    static void debug(Disk *disk);
    static int format(Disk *disk);

    static int mount(Disk *disk);
    static int unmount(Disk *disk);
    size_t getInumber(const char *filename);
    Inode getInode(size_t inumber);

    int create(const char *filename, uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions);
    int    remove(size_t inumber);
    statDetails stat(size_t inumber);

    char *getSuperBlock();
    char *getInodeBlocks();

    bool setFilePermissions(size_t inumber, uint32_t permissions);

    size_t fs_read(size_t inumber, char *data, size_t length, size_t offset);
    size_t fs_write(size_t inumber, const char *data, size_t length, size_t offset);
};
#endif