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
char *FileSystem::superBlock = nullptr;
char *FileSystem::inodeBlocks = nullptr;
int FileSystem::totalInodes = 0;

//implemented only for direct pointers
/* void FileSystem::debugInodes(char block){
    if (block.Inodes->Valid){
        fprintf(stdout, "Size: %d\n", block.Inodes->Size);
        for (int i = 0; i < 5; i ++){
            continue;
        }
    }
} */

size_t FileSystem::ceilDiv(size_t a, size_t b)
{
        return (a / b) + ((a % b) != 0);
}
/* bool FileSystem::writeBlock(Disk *disk, int blocknum, Block *block)
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
 */
FileSystem::FileSystem(Disk *disk)
{
    this->INODES_PER_BLOCK = disk->BLOCK_SIZE / 128;
    this->POINTERS_PER_INODE = 5;
    this->POINTERS_PER_BLOCK = disk->BLOCK_SIZE / 4;
    this->superBlock = new char[Disk::BLOCK_SIZE];
    this->totalInodes=FileSystem::ceilDiv(disk->blocks, 10) * sizeof(Disk::BLOCK_SIZE);
    this->inodeBlocks = new char[this->totalInodes];
}

FileSystem::~FileSystem()
{
    delete bitmap;
    delete superBlock;
    delete inodeBlocks;
}

void FileSystem::debug(Disk *disk)
{
    SuperBlock* auxBlock = reinterpret_cast<SuperBlock*>(superBlock);
    if (auxBlock->MagicNumber != MAGIC_NUMBER)
    {
            fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxBlock->MagicNumber, MAGIC_NUMBER);
            return;
    }
    else{
    fprintf(stdout, "Magic number identified.\n");
    fprintf(stdout, "Superblock:\n");
    fprintf(stdout, "\tBlocks: %d\n", auxBlock->Blocks);
    fprintf(stdout, "\tInode blocks: %d\n", auxBlock->InodeBlocks);
    fprintf(stdout, "\tInodes: %d\n", auxBlock->Inodes);

    //debug inodes
    /* for (int i = 1; i < block.Super.InodeBlocks + 1; i ++){
        disk->so_read(i, inode_block.Data);
        fprintf(stdout, "Inode %d:", i);
        FileSystem::debugInodes(inode_block);
    } */
    }
}

bool FileSystem::format(Disk *disk)
{
    //clear all data
    ftruncate(disk->descriptor, 0);
    //scriem superblock-ul
    SuperBlock *auxBlock = reinterpret_cast<SuperBlock*>(superBlock);
    auxBlock->MagicNumber = 0x05112002;
    auxBlock->Blocks = disk->blocks;
    auxBlock->InodeBlocks = ceilDiv(disk->blocks, 10);
    auxBlock->Inodes = auxBlock->InodeBlocks * INODES_PER_BLOCK;
    bitmap = new bool[disk->blocks - auxBlock->InodeBlocks - 1];

    //initialize inode
    Inode* inodes=reinterpret_cast<Inode*>(inodeBlocks);

    for(int i=0;i<totalInodes;i++){
        inodes[i].Valid=0;
    }

    free(inodes);


    //DEBUG
    //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
    //
    disk->so_write(0, superBlock);
}

bool FileSystem::mount(Disk *disk){

}

bool FileSystem::getInode(size_t inode)
{
   Inode *inodes=reinterpret_cast<Inode*>(this->inodeBlocks);

   if(!inodes[inode].Valid){
        free(inodes);
        return false;
   }

    free(inodes);
    return true;
}

ssize_t FileSystem::create(uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions){
    
    Inode *inodes=reinterpret_cast<Inode*>(this->inodeBlocks);

    for(int i=0;i<this->totalInodes;i++){
        if(!inodes[i].Valid){
            inodes[i].Valid=1;
            inodes[i].Size=0;
            inodes[i].Direct=(u_int32_t*)malloc(sizeof(uint32_t)*POINTERS_PER_INODE);
            inodes[i].OwnerUserID=_OwnerUserID;
            inodes[i].OwnerGroupID=_OwnerGroupID;
            inodes[i].Permissions=_Permissions;

            //filename is copied from shell

            free(inodes);
            return i;
        }    
    }

    //if does not return, has reached the maximum
    fprintf(stderr,"Reached the maximum size\n");

    return -1;
}

bool FileSystem::remove(size_t inumber){

}
statDetails FileSystem::stat(size_t inumber){

}

ssize_t FileSystem::read(size_t inumber, char *data, size_t length, size_t offset){

}
ssize_t FileSystem::write(size_t inumber, char *data, size_t length, size_t offset){
    
    // create inode if doesn t exist
    if(!getInode(inumber))
        inumber=create(1,1,1);

    
}
