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
size_t FileSystem::totalInodes = 0;

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

    //DEBUG
    //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
    //
    disk->so_write(0, superBlock);
    disk->mounted = 1;
}

bool FileSystem::mount(Disk *disk)
{
    if (disk->mounted) {
        fprintf(stderr, "A file system already mounted.\n");
        return -1;
    }
    if (disk->so_read(0, superBlock) < 0) {
        fprintf(stderr, "Error on reading superblock.\n");
        return -1;
    }

    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);

    if (auxSuperBlock->MagicNumber != MAGIC_NUMBER) {
        fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxSuperBlock->MagicNumber, MAGIC_NUMBER);
        return -1;
    }

    printf("Mounting...\n");

    //Inode* auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    printf("Initializing i-nodes...\n");
    lseek(disk->descriptor, Disk::BLOCK_SIZE, SEEK_SET);

    //read are limita, de reimplementat citire in chuck-uri
    read(disk->descriptor, FileSystem::inodeBlocks, auxSuperBlock->InodeBlocks * auxSuperBlock->Inodes);

    printf("Filesystem mounted.\n");

    return 0;
}

bool FileSystem::unmount(Disk *disk)
{
    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    printf("Unmounting...\n");

    printf("Saving super block...\n");
    disk->so_write(0, FileSystem::superBlock);

    printf("Saving i-nodes...\n");
    for (int i = 1; i < auxSuperBlock->InodeBlocks; i ++)
        disk->so_write(i, FileSystem::inodeBlocks + (i * Disk::BLOCK_SIZE));

    disk->mounted = 0;
    printf("File system unmounted.\n");
    return 0;
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

    return true;
}
statDetails FileSystem::stat(size_t inumber){

}

ssize_t FileSystem::fs_read(size_t inumber, char *data, size_t length, size_t offset)
{
/*     SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    void *start = nullptr;

    if (!auxInodeBlocks[inumber].Valid)  {
        fprintf(stderr, "Error on filesystem read. I-node invalid <%ld>.\n", inumber);
        return -1;
    }

    size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE);
    //DEBUG
    printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);
    //

    start = mmap(NULL, blocks*Disk::BLOCK_SIZE, PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0); */


}

ssize_t FileSystem::fs_write(size_t inumber, char *data, size_t length, size_t offset){

}