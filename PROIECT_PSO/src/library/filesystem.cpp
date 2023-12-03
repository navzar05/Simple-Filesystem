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
SuperBlock *FileSystem::auxBlock = nullptr;
Inode *FileSystem::inodes = nullptr;
int FileSystem::totalInodes = 0;
Disk *FileSystem::myDisk = nullptr;

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
    myDisk=disk;
    this->INODES_PER_BLOCK = disk->BLOCK_SIZE / 128;
    this->POINTERS_PER_INODE = 5;
    this->POINTERS_PER_BLOCK = disk->BLOCK_SIZE / 4;
    this->superBlock = new char[Disk::BLOCK_SIZE];
    this->totalInodes=FileSystem::ceilDiv(disk->blocks, 10) * sizeof(Disk::BLOCK_SIZE);
    this->inodeBlocks = new char[this->totalInodes]();
}

FileSystem::~FileSystem()
{
    delete bitmap;
    delete superBlock;
    delete inodeBlocks;
}

void FileSystem::debug(Disk *disk)
{
    if (auxSuperBlock->MagicNumber != MAGIC_NUMBER)
    {
            fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxSuperBlock->MagicNumber, MAGIC_NUMBER);
            return;
    }
    else{
    fprintf(stdout, "Magic number identified.\n");
    fprintf(stdout, "Superblock:\n");
    fprintf(stdout, "\tBlocks: %d\n", auxSuperBlock->Blocks);
    fprintf(stdout, "\tInode blocks: %d\n", auxSuperBlock->InodeBlocks);
    fprintf(stdout, "\tInodes: %d\n", auxSuperBlock->Inodes);

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
    auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    auxSuperBlock->MagicNumber = 0x05112002;
    auxSuperBlock->Blocks = disk->blocks;
    auxSuperBlock->InodeBlocks = ceilDiv(disk->blocks, 10);
    auxSuperBlock->Inodes = auxSuperBlock->InodeBlocks * INODES_PER_BLOCK;
    bitmap = new bool[disk->blocks - auxSuperBlock->InodeBlocks - 1]();

    //initialize inode
    inodes=reinterpret_cast<Inode*>(inodeBlocks);

    for(int i = 0; i < totalInodes; i ++){
        inodes[i].Valid=0;
    }

    inodes=NULL;

    //DEBUG
    //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
    //
    disk->so_write(0, superBlock);

    //set first blocks used
    bitmap[0]=true;
    for(int i=1;i<auxSuperBlock->InodeBlocks;i++)
        bitmap[i]=true;
}

bool FileSystem::mount(Disk *disk){

}

bool FileSystem::getInode(size_t inode)
{
   if(!inodes[inode].Valid){
        inodes=NULL;
        return false;
   }

   inodes=NULL;
   return true;
}

size_t FileSystem::setBlock()
{
    for(size_t i=0;i<auxSuperBlock->Blocks;i++){
        if(!bitmap[i]){
            bitmap[i]=true;
            return i;
        }
    }

    fprintf(stderr, "All blocks are occupied!\n");
    return -1;
}

void FileSystem::initializeIndirect(size_t inumber, char *indirectData, uint32_t *pointers)
{
    if(inodes[inumber].Indirect == 0){
        inodes[inumber].Indirect = setBlock();
    }

    indirectData = new char[Disk::BLOCK_SIZE];
    myDisk->so_read(inodes[inumber].Indirect, indirectData);
    pointers = reinterpret_cast<uint32_t*>(indirectData);
}

ssize_t FileSystem::create(uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions){
    
    Inode *inodes=reinterpret_cast<Inode*>(this->inodeBlocks);

    for(int i=0;i<this->totalInodes;i++){
        if(!inodes[i].Valid){
            inodes[i].Valid=1;
            inodes[i].Size=0;
            inodes[i].Direct=new uint32_t[POINTERS_PER_INODE]();
            inodes[i].OwnerUserID=_OwnerUserID;
            inodes[i].OwnerGroupID=_OwnerGroupID;
            inodes[i].Permissions=_Permissions;

            //filename is copied from shell

            inodes=NULL;
            return i;
        }    
    }

    //if does not return, has reached the maximum
    fprintf(stderr,"Reached the maximum size\n");

    return -1;
}

bool FileSystem::remove(size_t inumber){
    size_t indexPointer = ceilDiv(inodes[inumber].Size,Disk::BLOCK_SIZE);

    for(int i = 0; i < POINTERS_PER_INODE; i ++){
        //free that blocks
        if(inodes[inumber].Direct[i])
            bitmap[inodes[inumber].Direct[i]]=false;
    }

    delete inodes[inumber].Direct;

    if(indexPointer >= POINTERS_PER_INODE){
        char *data = new char[Disk::BLOCK_SIZE]();
        myDisk->so_read(inodes[inumber].Indirect, data);
        uint32_t *pointers=reinterpret_cast<uint32_t*>(data);
        
        //start from index 0 in Indirect Pointer
        indexPointer -= POINTERS_PER_INODE;
        for(int i = 0; i <= indexPointer; i ++){

            //invalidate all blocks from indirect pointer
            if(pointers[i] != 0){
                bitmap[pointers[i]] = false;
                pointers[i] = 0;
            }
        }

        //clear the block pointed indirect
        myDisk->so_write(inodes[inumber].Indirect, data);
        inodes[inumber].Indirect = 0;
        
        delete data;
        delete pointers;
    }

    inodes[inumber].Valid=0;
    inodes[inumber].OwnerGroupID=0;
    inodes[inumber].OwnerUserID=0;
    inodes[inumber].Size=0;
}

statDetails FileSystem::stat(size_t inumber){

}

ssize_t FileSystem::read(size_t inumber, char *data, size_t length, size_t offset){

}
ssize_t FileSystem::write(size_t inumber, char *data, size_t length, size_t offset){

    // create inode if doesn t exist
    if(!getInode(inumber))
        inumber=create(1,1,1);

    if(inodes[inumber].Size < offset){
        fprintf(stderr, "Offsetul depaseste dimensiunea fisierului\n");
        return -1;
    }

    size_t blockNum = 0, indexWrite = 0, indexPointer = 0;
    char *mem, *indirectData;
    uint32_t *pointers;

    //select pointer to start
    
    indexPointer=ceilDiv(inodes[inumber].Size,offset);
    
    //select from where to write in block
    indexWrite = offset - indexPointer * Disk::BLOCK_SIZE;

    mem = (char*)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1 ,0);

    memcpy(mem, data, length);

    //size will become for now size-offset
    inodes[inumber].Size -= offset;

    //write all in blocks of 4096
    while(length){

        //save block in inode but we need to see if in direct or indirect pointer
        if(indexPointer == POINTERS_PER_INODE){

            initializeIndirect(inumber, indirectData, pointers);
            pointers[indexPointer - POINTERS_PER_INODE] = setBlock();
            blockNum = pointers[indexPointer - POINTERS_PER_INODE];
        }
        else if(indexPointer < POINTERS_PER_INODE){

            if(inodes[inumber].Direct[indexPointer] == 0)
                inodes[inumber].Direct[indexPointer] = setBlock();

            blockNum = inodes[inumber].Direct[indexPointer];
        }
        else{

            blockNum = pointers[indexPointer - POINTERS_PER_INODE];
        }

        //write block on disk
        myDisk->so_write(blockNum, mem + indexWrite);

        //increment
        indexWrite += Disk::BLOCK_SIZE;
        inodes[inumber].Size += Disk::BLOCK_SIZE;
        indexPointer++;

        if(length <= Disk::BLOCK_SIZE){
            length=0;
        }
        else{
            length -= Disk::BLOCK_SIZE;
        }
    }
    
    inodes=NULL;
    munmap(mem,length);
}