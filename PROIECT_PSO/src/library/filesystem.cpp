#include "../../includes/filesystem.h"
#include "filesystem.h"

uint32_t FileSystem::INODES_PER_BLOCK = 0;
uint32_t FileSystem::POINTERS_PER_INODE = 0;
uint32_t FileSystem::POINTERS_PER_BLOCK = 0;
bool *FileSystem::bitmap = nullptr;
char *FileSystem::superBlock = nullptr;
char *FileSystem::inodeBlocks = nullptr;
Disk *FileSystem::mountedDisk = nullptr;
size_t FileSystem::totalInodes = 0;


size_t FileSystem::ceilDiv(size_t a, size_t b)
{
    return (a / b) + ((a % b) != 0);
}
bool FileSystem::loadDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t n)
{
    char *auxBlock = new char[Disk::BLOCK_SIZE];
    for (int i = 0; i < n; i ++) {
        memset(auxBlock, 0, Disk::BLOCK_SIZE);
        if (inodeBlocks[inumber].Direct[i] != 0) {
            mountedDisk->so_read(inodeBlocks[inumber].Direct[i], auxBlock);
            memcpy(start + i * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
        } else {
            allocBlock(&inodeBlocks[inumber].Direct[i]);
            mountedDisk->so_read(inodeBlocks[inumber].Direct[i], auxBlock);
            memcpy(start + i * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
        }
    }

    delete[] auxBlock;
    auxBlock = nullptr;
    printf("Direct blocks mapped.\n");
    return 0;
}

bool FileSystem::loadIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t n)
{
    return 0;
}
bool FileSystem::saveDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t n)
{
    for (int i = 0; i < n; i ++)
        mountedDisk->so_write(inodeBlocks[inumber].Direct[i], start + i * Disk::BLOCK_SIZE);

    printf("Pages saved.\n");
    return 0;
}
size_t FileSystem::getStartOfDataBlocks()
{
        SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
        return auxSuperBlock->Blocks - auxSuperBlock->InodeBlocks - 1;
}
bool FileSystem::allocBlock(uint32_t *pointer)
{
    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
    printf("number of blocks from allocBlock: %d\n", auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks() + 1);
        for (int i = 0; i < auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks() + 1; i ++) {
            if (FileSystem::bitmap[i] == 0) {
                (*pointer) = i + FileSystem::getStartOfDataBlocks();
                FileSystem::bitmap[i] = 1;
                return 0;
            }
        }
        fprintf(stderr, "Bitmap full.\n");
        return -1;
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
    FileSystem::mountedDisk = nullptr;
    if (bitmap != nullptr){
        delete bitmap;
        bitmap = nullptr;
    }
    if (superBlock != nullptr){
        delete superBlock;
        superBlock = nullptr;
    }
    if (inodeBlocks != nullptr){
        delete inodeBlocks;
        inodeBlocks = nullptr;
    }
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
    memset(bitmap, 0, sizeof(bool) * disk->blocks - auxBlock->InodeBlocks - 1);

    //DEBUG
    //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
    //
    disk->so_write(0, superBlock);
    FileSystem::mountedDisk = disk;
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

    FileSystem::mountedDisk = disk;

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

    FileSystem::mountedDisk = nullptr;

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
            inodes[i].Direct = new uint32_t[FileSystem::POINTERS_PER_INODE];
            inodes[i].OwnerUserID=_OwnerUserID;
            inodes[i].OwnerGroupID=_OwnerGroupID;
            inodes[i].Permissions=_Permissions;

            //filename is copied from shell
            printf("Inode created.\n");
            return i;
        }
    }

    //if does not return, has reached the maximum
    fprintf(stderr,"Reached the maximum size\n");

    return -1;
}

bool FileSystem::remove(size_t inumber)
{
    Inode *inodes=reinterpret_cast<Inode*>(this->inodeBlocks);


}

statDetails FileSystem::stat(size_t inumber)
{

}

ssize_t FileSystem::fs_read(size_t inumber, char *data, size_t length, size_t offset)
{
    SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    char *start = nullptr;

    if (!auxInodeBlocks[inumber].Valid)  {
        fprintf(stderr, "Error on filesystem read. I-node invalid <%ld>.\n", inumber);
        return -1;
    }

    size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE);

    //DEBUG
    printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);

    start = (char*)mmap(NULL, blocks*Disk::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    FileSystem::loadDirectPages(start, inumber, auxInodeBlocks, blocks);

    printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);

    memcpy(data , start + offset * sizeof(char), length);

    munmap(start, blocks*Disk::BLOCK_SIZE);

    return length;
}

ssize_t FileSystem::fs_write(size_t inumber, char *data, size_t length, size_t offset)
{
    SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    char *start = nullptr;

    if (!auxInodeBlocks[inumber].Valid)  {
        fprintf(stderr, "Error on filesystem write. I-node invalid <%ld>.\n", inumber);
        return -1;
    }

    size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE);
    size_t blocksToWrite = FileSystem::ceilDiv(length, Disk::BLOCK_SIZE);
    //DEBUG
    printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);
    printf("Blocks to write to inode <%ld>: %ld\n", inumber, blocksToWrite);
    //

    start = (char*)mmap(NULL, (blocksToWrite < blocks ? blocks : blocksToWrite) * Disk::BLOCK_SIZE,
     PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    FileSystem::loadDirectPages(start, inumber, auxInodeBlocks, (blocksToWrite < blocks ? blocks : blocksToWrite));

    printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);
    memcpy(start + offset * sizeof(char), data, length * sizeof(char));

    printf("Data written.\n\t Data: %s\n\t Size: %ld\n", data, length);

    FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, (blocksToWrite < blocks ? blocks : blocksToWrite));

    munmap(start, (blocksToWrite < blocks ? blocks : blocksToWrite) * Disk::BLOCK_SIZE);

    auxInodeBlocks[inumber].Size += length;

    printf("Data saved on disk. Size of file: %ld\n", auxInodeBlocks[inumber].Size);

    return length;
}
