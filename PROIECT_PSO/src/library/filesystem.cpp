#include "../../includes/filesystem.h"
#include "filesystem.h"

/* const static uint32_t MAGIC_NUMBER       = 0xf0f03410;
const static uint32_t INODES_PER_BLOCK   = 128;
const static uint32_t POINTERS_PER_INODE = 5;
const static uint32_t POINTERS_PER_BLOCK = 1024;
 */

// In filesystem.cpp
uint32_t FileSystem::INODES_PER_BLOCK = 0;   // Adjust the initial value as needed
uint32_t FileSystem::POINTERS_PER_INODE = 0;  // Adjust the initial value as needed
uint32_t FileSystem::POINTERS_PER_BLOCK = 0; // Adjust the initial value as needed
bool *FileSystem::bitmap = nullptr;
Block *FileSystem::superBlock = nullptr;
Block *FileSystem::inodeBlocks = nullptr;


//implemented only for direct pointers
void FileSystem::debugInodes(Block block){
    if (block.Inodes->Valid){
        fprintf(stdout, "Size: %d\n", block.Inodes->Size);
        for (int i = 0; i < 5; i ++){
            continue;
        }
    }
}

size_t FileSystem::ceilDiv(size_t a, size_t b)
{
        return (a / b) + ((a % b) != 0);
}
bool FileSystem::writeBlock(Disk *disk, int blocknum, Block *block)
{
    char* blockToWrite = (char*)calloc(Disk::BLOCK_SIZE, sizeof(char));
    if (!blockToWrite) {
        fprintf(stderr, "Failed to allocate memory in writeBlock.\n");
        return false;
    }

    memcpy(blockToWrite, block, Disk::BLOCK_SIZE);
    disk->so_write(blocknum, blockToWrite);
    free(blockToWrite);

    return true;
}

bool FileSystem::readBlock(Disk *disk, int blocknum, Block *block)
{
    char *blockRead = (char*)calloc(Disk::BLOCK_SIZE, sizeof(char));
    if (!blockRead) {
        fprintf(stderr, "Failed to allocate memory in readBlock.\n");
        return false;
    }

    bool readSuccess = disk->so_read(blocknum, blockRead);
    if (readSuccess) {
        memcpy(block, blockRead, Disk::BLOCK_SIZE);
    }
    free(blockRead);

    return readSuccess;
}

FileSystem::FileSystem(Disk *disk)
{
    this->INODES_PER_BLOCK = disk->BLOCK_SIZE / 128;
    this->POINTERS_PER_INODE = 5;
    this->POINTERS_PER_BLOCK = disk->BLOCK_SIZE / 4;
    this->bitmap = (bool*)calloc(disk->blocks, sizeof(bool));
    this->superBlock = (Block*)calloc(1, sizeof(Block));
    this->inodeBlocks = (Block*)calloc(FileSystem::ceilDiv(disk->blocks, 10), sizeof(Block));
}

FileSystem::~FileSystem()
{
    free(this->bitmap);
    this->bitmap = NULL;
    free(superBlock);
    this->superBlock = NULL;
    free(inodeBlocks);
    inodeBlocks = NULL;
}

void FileSystem::debug(Disk *disk)
{
        Block block, inode_block;
        disk->so_read(0, block.Data);
        if (block.Super.MagicNumber != MAGIC_NUMBER)
        {
                fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", block.Super.MagicNumber, MAGIC_NUMBER);
                return;
        }
        else{
        fprintf(stdout, "Magic number identified.\n");
        fprintf(stdout, "Superblock:\n");
        fprintf(stdout, "\tBlocks: %d\n", block.Super.Blocks);
        fprintf(stdout, "\tInode blocks: %d\n", block.Super.InodeBlocks);
        fprintf(stdout, "\tInodes: %d\n", block.Super.Inodes);

        //debug inodes
        for (int i = 1; i < block.Super.InodeBlocks + 1; i ++){
            disk->so_read(i, inode_block.Data);
            fprintf(stdout, "Inode %d:", i);
            FileSystem::debugInodes(inode_block);
        }
    }
}
bool FileSystem::format(Disk *disk)
{
    //clear all data
    ftruncate(disk->descriptor, 0);
    superBlock->Super.MagicNumber = 0x05112002;
    superBlock->Super.Blocks = disk->blocks;
    superBlock->Super.InodeBlocks = ceilDiv(disk->blocks, 10);
    superBlock->Super.Inodes = superBlock->Super.InodeBlocks * INODES_PER_BLOCK;
    printf("%d %d %d\n", superBlock->Super.Blocks, superBlock->Super.InodeBlocks, superBlock->Super.Inodes);
    writeBlock(disk, 0, superBlock);
}

bool FileSystem::mount(Disk *disk){

}

ssize_t FileSystem::create(uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions){

}
bool FileSystem::remove(size_t inumber){

}
statDetails FileSystem::stat(size_t inumber){

}

ssize_t FileSystem::read(size_t inumber, char *data, size_t length, size_t offset){

}
ssize_t FileSystem::write(size_t inumber, char *data, size_t length, size_t offset){

}