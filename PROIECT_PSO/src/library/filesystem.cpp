#include "../../includes/filesystem.h"
#include "../../includes/fileSystemAPI.h"

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
        printf("Load direct block %ld\n", inodeBlocks[inumber].Direct[j]);
        if (inodeBlocks[inumber].Direct[j] != 0) {
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock, Disk::BLOCK_SIZE);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
        } else {
            allocBlock(&inodeBlocks[inumber].Direct[j]);
           /*  mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock, Disk::BLOCK_SIZE);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE); */
        }
    }

    delete[] auxBlock;
    auxBlock = nullptr;
    //printf("Direct blocks mapped.\n");
    return 0;
}

bool FileSystem::loadIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
    char *auxBlockRead = new char[Disk::BLOCK_SIZE]; // pointer auxiliar pentru citirea blocurilor de date
    if (inodeBlocks[inumber].Indirect == 0)
        FileSystem::allocBlock(&inodeBlocks[inumber].Indirect);

    //aducem blocul de pointeri pe stack
    mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers, Disk::BLOCK_SIZE);

    uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

    size_t block_index = 0;
    for (int j = start_blk; j <= end_blk; j ++, block_index++) {
        memset(auxBlockRead, 0, Disk::BLOCK_SIZE);
        if (pointers[j] != 0) {
            mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead, Disk::BLOCK_SIZE);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE);
        } else {
            FileSystem::allocBlock(&pointers[j]);
            /* mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead, Disk::BLOCK_SIZE);
            memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE); */
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
        mountedDisk->so_write(inodeBlocks[inumber].Direct[j], start + block_index * Disk::BLOCK_SIZE, Disk::BLOCK_SIZE);

    //printf("Direct blocks saved.\n");
    return 0;
}
bool FileSystem::saveIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
{
    char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
    mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers, Disk::BLOCK_SIZE);

    uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

    size_t block_index = 0;
    for (int j = start_blk; j <= end_blk; j ++, block_index++)
        mountedDisk->so_write(pointers[j], start + block_index * Disk::BLOCK_SIZE, Disk::BLOCK_SIZE);

    delete auxBlockPointers;

    //printf("Indirect blocks saved.\n");
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
                FileSystem::bitmap[i] = true;
                printf("Found an empty block. %d\n", i);
                return 0;
            }
        }
        fprintf(stderr, "Bitmap full.\n");
        exit(-1);
        return -1;
}


bool FileSystem::initBitmap(const Inode* inodeBlock)
{
    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);

    bitmap = new bool[auxSuperBlock->Blocks - auxSuperBlock->InodeBlocks - 1];
    memset(bitmap, 0, sizeof(bool) * (auxSuperBlock->Blocks - auxSuperBlock->InodeBlocks - 1));

    for (int i = 0; i < auxSuperBlock->Inodes; i ++) {
            if (inodeBlock[i + getStartOfDataBlocks()].Valid)
                bitmap[i] = 1;
    }
    return 0;

}

FileSystem::FileSystem(Disk *disk)
{
    this->INODES_PER_BLOCK = disk->BLOCK_SIZE / 128;
    this->POINTERS_PER_INODE = 5;
    this->POINTERS_PER_BLOCK = disk->BLOCK_SIZE / 4;

    this->superBlock = new char[Disk::BLOCK_SIZE]{};
    this->totalInodes = FileSystem::ceilDiv(disk->blocks, 10) * Disk::BLOCK_SIZE;
    this->inodeBlocks = new char[this->totalInodes]{};
}

FileSystem::~FileSystem()
{
    this->unmount(this->mountedDisk);
    //printf("in fs desctructor\n");
    FileSystem::mountedDisk = nullptr;
/*     if (FileSystem::bitmap != nullptr){
        delete[] FileSystem::bitmap;
        FileSystem::bitmap = nullptr;
    }
    printf("deleted bitmap\n"); */
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
    printf("Formating disk...\n");
    ftruncate(disk->descriptor, 0);

    //scriem superblock-ul
    SuperBlock *auxBlock = reinterpret_cast<SuperBlock*>(superBlock);
    auxBlock->MagicNumber = 0x05112002;
    auxBlock->Blocks = disk->blocks;
    auxBlock->InodeBlocks = ceilDiv(disk->blocks, 10);
    auxBlock->Inodes = auxBlock->InodeBlocks * INODES_PER_BLOCK;
    bitmap = new bool[disk->blocks - auxBlock->InodeBlocks - 1];
    memset(bitmap, 0, sizeof(bool) * disk->blocks - auxBlock->InodeBlocks - 1);

    //alocam inodeBlocks
    inodeBlocks = new char[auxBlock->InodeBlocks * Disk::BLOCK_SIZE];

    //DEBUG
    //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
    //
    disk->so_write(0, superBlock, Disk::BLOCK_SIZE);
    FileSystem::mountedDisk = disk;
    disk->mounted = 1;

    //ocupy superblock and blocks for inode
    // for(int i = 0; i < auxBlock->InodeBlocks + 1; i ++){
    //     bitmap[i] = true;
    // }
    printf("Disk formated\n");

}

bool FileSystem::mount(Disk *disk)
{
    if (disk->mounted) {
        fprintf(stderr, "A file system already mounted.\n");
        return 1;
    }
    if (disk->so_read(0, superBlock, Disk::BLOCK_SIZE) < 0) {
        fprintf(stderr, "Error on reading superblock.\n");
        return 1;
    }

    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);

    if (auxSuperBlock->MagicNumber != MAGIC_NUMBER) {
        fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxSuperBlock->MagicNumber, MAGIC_NUMBER);
        return 1;
    }

    printf("Mounting...\n");

    //Inode* auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    printf("Initializing i-nodes...\n");

    //read are limita, de reimplementat citire in chunck-uri
    for (int i = 1; i <= auxSuperBlock->InodeBlocks; i ++) {
        printf("\treading i-node <%ld>\n", i - 1);
        disk->so_read(i, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
    }

    FileSystem::initBitmap(reinterpret_cast<Inode*>(FileSystem::inodeBlocks));

    disk->mounted = 1;

    FileSystem::mountedDisk = disk;

    printf("Filesystem mounted.\n");

    return 0;
}

bool FileSystem::unmount(Disk *disk)
{
    if (disk->mounted == 0) {
        printf("FS already unmounted.\n");
        return 0;
    }

    SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    printf("Unmounting...\n");

    printf("Saving super block...\n");
    disk->so_write(0, FileSystem::superBlock, sizeof(SuperBlock));

    printf("Saving i-nodes...\n");
    for (int i = 1; i <= auxSuperBlock->InodeBlocks; i ++) {
        printf("\tSaving inode block %d\n", i - 1);
        //write(STDOUT_FILENO, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
        disk->so_write(i, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
    }

    disk->mounted = 0;

    FileSystem::mountedDisk = nullptr;

    printf("File system unmounted.\n");
    return 0;
}

Inode FileSystem::getInode(size_t inumber)
{
    Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);

    return inodes[inumber];
}

size_t FileSystem::getInumber(const char *filename)
{
    Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);
    SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);

    //get the inode of the filename
    for(int i = 0; i < auxSuperBlock->Inodes; i ++){
        
        if(inodes[i].Valid && strncmp(inodes[i].Filename, filename, (strlen(filename) + 1)) == 0){
            printf("I found file= %s at index= %d\n", filename, i);
            return i;
        }
    }

    //the filename does not exist
    fprintf(stderr, "File %s does not exist!\n", filename);
    return -1;
}

size_t FileSystem::create(const char *filename, uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions)
{
    if(strlen(filename) > MAX_FILENAME_LENGTH){
        fprintf(stderr, "Length incorect at filename= %s\n", filename);
        return -1;
    }

    Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);
    for (int i = 0; i < this->totalInodes; i++) {
        if (!inodes[i].Valid) {
            inodes[i].Valid = 1;
            inodes[i].Size = 0;
            // inodes[i].Direct = new uint32_t[FileSystem::POINTERS_PER_INODE]{};

            if (!inodes[i].Direct) {
                fprintf(stderr, "Failed to allocate memory for inode direct blocks.\n");
                return -1;
            }

            inodes[i].OwnerUserID = _OwnerUserID;
            inodes[i].OwnerGroupID = _OwnerGroupID;
            inodes[i].Permissions = _Permissions;

            memset(inodes[i].Filename, '\0', MAX_FILENAME_LENGTH);

            printf("Inode created with index %d.\n", i);
            memcpy(inodes[i].Filename, filename, strlen(filename));

/*             if(checkImportantFiles(filename, i))
                inodes[i].Size = 4095;
            else
                inodes[i].Size = 0;
 */
            printf("Inode with inumber= %d filename= %s valid= %d size= %d  userID= %d groupID= %d permissions= %d created.\n", i, inodes[i].Filename, inodes[i].Valid, inodes[i].Size,inodes[i].OwnerUserID, inodes[i].OwnerGroupID, inodes[i].Permissions );

            return i;
        }
    }

    //reached the maximum size
    fprintf(stderr,"Reached the maximum size\n");
    return -1;
}

bool FileSystem::remove(size_t inumber)
{
    if (inumber >= this->totalInodes) {
        fprintf(stderr, "Invalid inode number.\n");
        return false;
    }

    Inode* inodes = reinterpret_cast<Inode*>(this->inodeBlocks);

    if (!inodes[inumber].Valid) {
        fprintf(stderr, "Inode %zu is not in use.\n", inumber);
        return false;
    }

    // Free direct blocks
    for (int i = 0; i < POINTERS_PER_INODE; i++) {
        if (inodes[inumber].Direct[i]) {
            bitmap[inodes[inumber].Direct[i]] = false;
        }
    }

    delete[] inodes[inumber].Direct; // Correctly deallocate the memory

    // Handle indirect blocks
    if (inodes[inumber].Indirect != 0) {
        char *data = new char[Disk::BLOCK_SIZE]{};
        mountedDisk->so_read(inodes[inumber].Indirect, data, Disk::BLOCK_SIZE);
        auto pointers = reinterpret_cast<uint32_t*>(data);

        size_t indirectBlocks = ceilDiv(inodes[inumber].Size, Disk::BLOCK_SIZE) - POINTERS_PER_INODE;
        for (int i = 0; i < indirectBlocks; i++) {
            if (pointers[i]) {
                bitmap[pointers[i]] = false;
            }
        }

        mountedDisk->so_write(inodes[inumber].Indirect, data, Disk::BLOCK_SIZE);
        inodes[inumber].Indirect = 0;
    }

    // Invalidate the inode
    inodes[inumber].Valid = 0;
    inodes[inumber].OwnerGroupID = 0;
    inodes[inumber].OwnerUserID = 0;
    inodes[inumber].Size = 0;

    //set filename to zero
    memset(inodes[inumber].Filename, '\0', MAX_FILENAME_LENGTH);

    return true;
}

statDetails FileSystem::stat(size_t inumber){
    Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);

    statDetails inodeDetails;

    //set statDetails
    inodeDetails.Valid = inodes[inumber].Valid;
    inodeDetails.Size = inodes[inumber].Size;
    inodeDetails.OwnerUserID = inodes[inumber].OwnerUserID;
    inodeDetails.OwnerGroupID = inodes[inumber].OwnerGroupID;
    inodeDetails.Permissions = inodes[inumber].Permissions;

    inodes = NULL;

    return inodeDetails;
}

//Doar pentru length corect
size_t FileSystem::fs_read(size_t inumber, char *data, size_t length, size_t offset)
{
    SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
    Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
    char *start = nullptr;

  /*   for(int i = fileSystemAPI::inumberUsersFile; i <= fileSystemAPI::inumberPasswordsFile; i ++)
        printf("Inodes in fs_read: inumber= %d valid= %d filename= %s!\n", i, auxInodeBlocks[i].Valid, auxInodeBlocks[i].Filename);
 */
    if (!auxInodeBlocks[inumber].Valid)  {
        fprintf(stderr, "Error on filesystem read. I-node invalid <%ld>.\n", inumber);
        return -1;
    }

    //number of blocks ocuppied by the file to be read
    size_t blocks_of_file = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE);

    size_t minblock = FileSystem::floorDiv(offset, Disk::BLOCK_SIZE);
    size_t maxblock = minblock + FileSystem::floorDiv(length, Disk::BLOCK_SIZE);

    //printf("blocks of file %ld minblock %ld maxblock %ld\n", blocks_of_file, minblock, maxblock);

    if (minblock > blocks_of_file || blocks_of_file == 0)
        return 0;

    if (maxblock > blocks_of_file - 1) {
        maxblock = blocks_of_file - 1;
        length = auxInodeBlocks[inumber].Size - offset;
    }

    //printf("blocks of file %ld minblock %ld maxblock %ld\n", blocks_of_file, minblock, maxblock);


    start = (char*)mmap(NULL, (maxblock - minblock + 1) * Disk::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);


    //printf("Memory mapped\n");

    //LOADING
    loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);

    //printf("Pages loaded.\n");

    //printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);

    //printf("%s\n", start);

    memcpy(data , start + (offset % Disk::BLOCK_SIZE) * sizeof(char), length);

    munmap(start, (maxblock - minblock + 1) * Disk::BLOCK_SIZE);
    fflush(stdout);

    return length;
}

size_t FileSystem::fs_write(size_t inumber, const char *data, size_t length, size_t offset)
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
    //printf("minblock write: %ld maxblock write: %ld\n", minblock, maxblock);
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
    //printf("%ld", auxInodeBlocks[inumber])

    loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);

    //WRITING
    //printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);
    memcpy(start + (offset % Disk::BLOCK_SIZE), data, length * sizeof(char));

    //printf("Data written.\n\t Data: %s\n\t Size: %ld\n", data, length);

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

    //printf("Data saved on disk. Size of file: %ld\n", auxInodeBlocks[inumber].Size);

    fflush(stdout);
    return length;
}
