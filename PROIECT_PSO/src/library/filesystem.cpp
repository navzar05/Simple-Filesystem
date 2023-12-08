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


size_t FileSystem::floorDiv(size_t a, size_t b)
{
        return size_t(a / b);
}

size_t FileSystem::ceilDiv(size_t a, size_t b)
{
    return (a / b) + ((a % b) != 0);
}

bool FileSystem::loadPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    if (start_blk > 5) {
        loadIndirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
    }
    else if (end_blk > 5) {
        loadDirectPages(start, inumber, inodeBlocks, start_blk, FileSystem::POINTERS_PER_INODE);
        loadIndirectPages(start, inumber, inodeBlocks, FileSystem::POINTERS_PER_INODE + 1, end_blk);
    }
    else {
        loadDirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
    }
    return 0;
}
bool FileSystem::loadDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{

    size_t block_index = 0;
    char *auxBlock = new char[Disk::BLOCK_SIZE];
    for (int j = start_blk; j <= end_blk; j ++, block_index++) {
        memset(auxBlock, 0, Disk::BLOCK_SIZE);
        if (inodeBlocks[inumber].Direct[j] != 0) {
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
        } else {
            allocBlock(&inodeBlocks[inumber].Direct[j]);
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
        }
    }

    delete[] auxBlock;
    auxBlock = nullptr;
    printf("Direct blocks mapped.\n");
    return 0;
}

bool FileSystem::loadIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
    char *auxBlockRead = new char[Disk::BLOCK_SIZE]; // pointer auxiliar pentru citirea blocurilor de date
    if (inodeBlocks[inumber].Indirect == 0)
        FileSystem::allocBlock(&inodeBlocks[inumber].Indirect);

    //aducem blocul de pointeri pe stack
    mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers);

    uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

    size_t block_index = 0;
    for (int j = start_blk; j <= end_blk; j ++, block_index++) {
        memset(auxBlockRead, 0, Disk::BLOCK_SIZE);
        if (pointers[j] != 0) {
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE);
        } else {
            FileSystem::allocBlock(&pointers[j]);
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE);
        }
    }

    delete[] auxBlockPointers;
    delete[] auxBlockRead;

    return 0;
}

bool FileSystem::saveDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    size_t block_index = 0;
    for (int j = start_blk; j <= end_blk; j ++, block_index++)
        mountedDisk->so_write(inodeBlocks[inumber].Direct[j], start + block_index * Disk::BLOCK_SIZE);

    printf("Direct blocks saved.\n");
    return 0;
}
bool FileSystem::saveIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
    mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers);

    uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

    size_t block_index = 0;
    for (int j = start_blk; j <= end_blk; j ++, block_index++)
        mountedDisk->so_write(pointers[j], start + block_index * Disk::BLOCK_SIZE);

    delete auxBlockPointers;

    printf("Indirect blocks saved.\n");
    return 0;
}
bool FileSystem::savePages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    if (start_blk > 5) {
        saveIndirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
    }
    else if (end_blk > 5) {
        saveDirectPages(start, inumber, inodeBlocks, start_blk, FileSystem::POINTERS_PER_INODE);
        saveIndirectPages(start, inumber, inodeBlocks, FileSystem::POINTERS_PER_INODE + 1, end_blk);
    }
    else {
        saveDirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
    }
    return 0;
    return 0;
}
size_t FileSystem::getStartOfDataBlocks()
{
        SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
        return auxSuperBlock->InodeBlocks + 1;
}
bool FileSystem::allocBlock(uint32_t *pointer)
{
    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
    printf("number of blocks from allocBlock: %ld\n", auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks());
        for (int i = 0; i < auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks(); i ++) {
            if (FileSystem::bitmap[i] == 0) {
                (*pointer) = i + FileSystem::getStartOfDataBlocks();
                FileSystem::bitmap[i] = 1;
                return 0;
            }
        }
        fprintf(stderr, "Bitmap full.\n");
        return -1;
}

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

bool FileSystem::remove(size_t inumber){
    Inode *inodes=reinterpret_cast<Inode*>(this->inodeBlocks);
    size_t indexPointer = ceilDiv(inodes[inumber].Size, Disk::BLOCK_SIZE);

    for(int i = 0; i < POINTERS_PER_INODE; i ++){
        //free that blocks
        if(inodes[inumber].Direct[i])
            bitmap[inodes[inumber].Direct[i]]=false;
    }

    delete inodes[inumber].Direct;

    if(indexPointer >= POINTERS_PER_INODE){
        char *data = new char[Disk::BLOCK_SIZE]();
        mountedDisk->so_read(inodes[inumber].Indirect, data);
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
        mountedDisk->so_write(inodes[inumber].Indirect, data);
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

//Doar pentru length corect
ssize_t FileSystem::fs_read(size_t inumber, char *data, size_t length, size_t offset)
{
    SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    char *start = nullptr;

    if (!auxInodeBlocks[inumber].Valid)  {
        fprintf(stderr, "Error on filesystem read. I-node invalid <%ld>.\n", inumber);
        return -1;
    }

    size_t minblock = FileSystem::floorDiv(offset, Disk::BLOCK_SIZE);
    size_t maxblock = minblock + FileSystem::floorDiv(length, Disk::BLOCK_SIZE);
    /* size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE); //cate blocuri avem pentru i-node
    //DEBUG
    printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);
    //
 */
    start = (char*)mmap(NULL, (maxblock - minblock + 1) * Disk::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);


    //LOADING
    loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);
   /*  if (blocks <= FileSystem::POINTERS_PER_INODE)
        FileSystem::loadDirectPages(start, inumber, auxInodeBlocks, blocks);
    else {
        FileSystem::loadDirectPages(start, inumber, auxInodeBlocks, FileSystem::POINTERS_PER_INODE);
        FileSystem::loadIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
         auxInodeBlocks, blocks - FileSystem::POINTERS_PER_INODE);
    } */

    printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);

    memcpy(data , start + (offset % Disk::BLOCK_SIZE) * sizeof(char), length);

    //SAVING
    /* if (blocks <= FileSystem::POINTERS_PER_INODE)
        FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, blocks);
    else {
        FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, FileSystem::POINTERS_PER_INODE);
        FileSystem::saveIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
         auxInodeBlocks, blocks - FileSystem::POINTERS_PER_INODE);
    } */

    savePages(start, inumber, auxInodeBlocks, minblock, maxblock);

    munmap(start, (maxblock - minblock + 1) * Disk::BLOCK_SIZE);

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


    size_t minblock = FileSystem::floorDiv(offset, Disk::BLOCK_SIZE);
    size_t maxblock = minblock + FileSystem::floorDiv(length, Disk::BLOCK_SIZE);
    printf("minblock write: %ld maxblock write: %ld\n", minblock, maxblock);
    /* size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE); //cate blocuri avem pentru i-node
    size_t blocksToWrite = FileSystem::ceilDiv(length, Disk::BLOCK_SIZE); //cate blocuri au fost cerute a fi scrise
    size_t totalBlocks = (blocksToWrite < blocks ? blocks : blocksToWrite); //maximul dintre blocks si blocksToWrite
    //DEBUG
    printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);
    printf("Blocks to write to inode <%ld>: %ld\n", inumber, blocksToWrite);
    printf("Total blocks of inode <%ld>: %ld\n", inumber, totalBlocks); */
    //

    start = (char*)mmap(NULL, (maxblock - minblock + 1) * Disk::BLOCK_SIZE,
     PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    //LOADING
    loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);

    //WRITING
    printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);
    memcpy(start + (offset % Disk::BLOCK_SIZE) * sizeof(char), data, length * sizeof(char));

    printf("Data written.\n\t Data: %s\n\t Size: %ld\n", data, length);

    ///SAVING
    savePages(start, inumber, auxInodeBlocks, minblock, maxblock);
    /* if (totalBlocks <= FileSystem::POINTERS_PER_INODE)
        FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, totalBlocks);
    else {
        FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, FileSystem::POINTERS_PER_INODE);
        FileSystem::saveIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
         auxInodeBlocks, totalBlocks - FileSystem::POINTERS_PER_INODE);
    } */
    //FileSystem::saveIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
    //auxInodeBlocks, totalBlocks - FileSystem::POINTERS_PER_INODE);

    munmap(start, (maxblock - minblock + 1) * Disk::BLOCK_SIZE);

    auxInodeBlocks[inumber].Size += length;

    printf("Data saved on disk. Size of file: %ld\n", auxInodeBlocks[inumber].Size);

    return length;
}