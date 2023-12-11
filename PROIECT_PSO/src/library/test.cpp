// #include "../../includes/fileSystemAPI.h"

// size_t Disk::BLOCK_SIZE = 0;

// Disk::~Disk(){
//     close(this->descriptor);
// }

// void Disk::disk_open(const char *path, size_t nblocks, size_t block_size)
// {
//     if (block_size % 512 != 0) {
//         fprintf(stderr, "Bad block size. It must be a multiple of 512. (block_size = %ld)\n", block_size);
//         return;
//     }
//     this->descriptor = open(path, O_RDWR | O_CREAT, 0666);
//     if (this->descriptor < 0){
//         fprintf(stderr, "Failed to open diskimage. (path: %s, nblocks: %ld)\n", path, nblocks);
//         return;
//     }
//     this->blocks = nblocks;
//     this->BLOCK_SIZE = block_size;
// }

// bool Disk::so_read(size_t blocknum, char *data, size_t size) {
//     if (blocknum >= blocks) {
//         fprintf(stderr, "Failed to read data. (blocknum %ld, max. index %ld)\n", blocknum, blocks);
//         return -1;
//     }

//     if (data == nullptr){
//         fprintf(stderr, "Nothing to write.\n");
//         return -1;
//     }

//     lseek(this->descriptor, Disk::BLOCK_SIZE * blocknum, SEEK_SET);

//     ssize_t readbytes = read(this->descriptor, data, size);
//     if (readbytes < 0) {
//         fprintf(stderr, "Failed to read block from disk. (blocknum: %ld)\n", blocknum);
//         return -1;
//     }

// /*     if (size < Disk::BLOCK_SIZE) {
//         memset(data + size, 0, Disk::BLOCK_SIZE - size); // Zero out the rest
//     } */
//     return 0;
// }


// bool Disk::so_write(size_t blocknum, char *data, size_t size) {
//     if (blocknum >= blocks) {
//         fprintf(stderr, "Failed to write data. (blocknum %ld, max. index %ld)\n", blocknum, blocks);
//         return -1;
//     }

//     lseek(this->descriptor, Disk::BLOCK_SIZE * blocknum, SEEK_SET);

//     if (write(this->descriptor, data, size) < 0) {
//         fprintf(stderr, "Failed to write to block from disk. (blocknum: %ld)\n", blocknum);
//         return -1;
//     }

//     if (size < Disk::BLOCK_SIZE) {
//         // Write 0x00 for the remaining part of the block
//         char zeroPadding[Disk::BLOCK_SIZE - size];
//         memset(zeroPadding, 0, Disk::BLOCK_SIZE - size);
//         write(this->descriptor, zeroPadding, Disk::BLOCK_SIZE - size);
//     }

//     return 0;
// }


// uint32_t FileSystem::INODES_PER_BLOCK = 0;
// uint32_t FileSystem::POINTERS_PER_INODE = 0;
// uint32_t FileSystem::POINTERS_PER_BLOCK = 0;
// bool *FileSystem::bitmap = nullptr;
// char *FileSystem::superBlock = nullptr;
// char *FileSystem::inodeBlocks = nullptr;
// Disk *FileSystem::mountedDisk = nullptr;
// size_t FileSystem::totalInodes = 0;


// size_t FileSystem::floorDiv(size_t a, size_t b)
// {
//         return size_t(a / b);
// }

// size_t FileSystem::ceilDiv(size_t a, size_t b)
// {
//     return (a / b) + ((a % b) != 0);
// }

// bool FileSystem::loadPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     if (start_blk > 5) {
//         loadIndirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
//     }
//     else if (end_blk > 5) {
//         loadDirectPages(start, inumber, inodeBlocks, start_blk, FileSystem::POINTERS_PER_INODE);
//         loadIndirectPages(start, inumber, inodeBlocks, FileSystem::POINTERS_PER_INODE + 1, end_blk);
//     }
//     else {
//         loadDirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
//     }
//     return 0;
// }
// bool FileSystem::loadDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     size_t block_index = 0;
//     char *auxBlock = new char[Disk::BLOCK_SIZE];
//     for (int j = start_blk; j <= end_blk; j ++, block_index++) {
//         memset(auxBlock, 0, Disk::BLOCK_SIZE);
//         printf("Load direct block %ld\n", inodeBlocks[inumber].Direct[j]);
//         if (inodeBlocks[inumber].Direct[j] != 0) {
//             mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock, Disk::BLOCK_SIZE);
//             memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE);
//         } else {
//             allocBlock(&inodeBlocks[inumber].Direct[j]);
//            /*  mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlock, Disk::BLOCK_SIZE);
//             memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlock, Disk::BLOCK_SIZE); */
//         }
//     }

//     delete[] auxBlock;
//     auxBlock = nullptr;
//     printf("Direct blocks mapped.\n");
//     return 0;
// }

// bool FileSystem::loadIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
//     char *auxBlockRead = new char[Disk::BLOCK_SIZE]; // pointer auxiliar pentru citirea blocurilor de date
//     if (inodeBlocks[inumber].Indirect == 0)
//         FileSystem::allocBlock(&inodeBlocks[inumber].Indirect);

//     //aducem blocul de pointeri pe stack
//     mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers, Disk::BLOCK_SIZE);

//     uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

//     size_t block_index = 0;
//     for (int j = start_blk; j <= end_blk; j ++, block_index++) {
//         memset(auxBlockRead, 0, Disk::BLOCK_SIZE);
//         if (pointers[j] != 0) {
//             mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead, Disk::BLOCK_SIZE);
//             memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE);
//         } else {
//             FileSystem::allocBlock(&pointers[j]);
//             mountedDisk->so_read(inodeBlocks[inumber].Direct[j], auxBlockRead, Disk::BLOCK_SIZE);
//             memcpy(start + block_index * Disk::BLOCK_SIZE, auxBlockRead, Disk::BLOCK_SIZE);
//         }
//     }

//     delete[] auxBlockPointers;
//     delete[] auxBlockRead;

//     return 0;
// }

// bool FileSystem::saveDirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     size_t block_index = 0;
//     for (int j = start_blk; j <= end_blk; j ++, block_index++)
//         mountedDisk->so_write(inodeBlocks[inumber].Direct[j], start + block_index * Disk::BLOCK_SIZE, Disk::BLOCK_SIZE);

//     printf("Direct blocks saved.\n");
//     return 0;
// }
// bool FileSystem::saveIndirectPages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     char *auxBlockPointers = new char[Disk::BLOCK_SIZE]; //pointer pentru blocul cu pointeri
//     mountedDisk->so_read(inodeBlocks[inumber].Indirect, auxBlockPointers, Disk::BLOCK_SIZE);

//     uint32_t *pointers = reinterpret_cast<uint32_t*>(auxBlockPointers);

//     size_t block_index = 0;
//     for (int j = start_blk; j <= end_blk; j ++, block_index++)
//         mountedDisk->so_write(pointers[j], start + block_index * Disk::BLOCK_SIZE, Disk::BLOCK_SIZE);

//     delete auxBlockPointers;

//     printf("Indirect blocks saved.\n");
//     return 0;
// }
// bool FileSystem::savePages(char *start, size_t inumber, Inode *inodeBlocks, size_t start_blk, size_t end_blk)
// {
//     if (start_blk > 5) {
//         saveIndirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
//     }
//     else if (end_blk > 5) {
//         saveDirectPages(start, inumber, inodeBlocks, start_blk, FileSystem::POINTERS_PER_INODE);
//         saveIndirectPages(start, inumber, inodeBlocks, FileSystem::POINTERS_PER_INODE + 1, end_blk);
//     }
//     else {
//         saveDirectPages(start, inumber, inodeBlocks, start_blk, end_blk);
//     }
//     return 0;
// }
// size_t FileSystem::getStartOfDataBlocks()
// {
//         SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
//         return auxSuperBlock->InodeBlocks + 1;
// }
// bool FileSystem::allocBlock(uint32_t *pointer)
// {
//     SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);
//     printf("number of blocks from allocBlock: %ld\n", auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks());
//         for (int i = 0; i < auxSuperBlock->Blocks - FileSystem::getStartOfDataBlocks(); i ++) {
//             if (FileSystem::bitmap[i] == 0) {
//                 (*pointer) = i + FileSystem::getStartOfDataBlocks();
//                 FileSystem::bitmap[i] = true;
//                 printf("Found an empty block. %d\n", i);
//                 return 0;
//             }
//         }
//         fprintf(stderr, "Bitmap full.\n");
//         exit(-1);
//         return -1;
// }


// bool FileSystem::initBitmap(const Inode* inodeBlock)
// {
//     SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(FileSystem::superBlock);

//     bitmap = new bool[auxSuperBlock->Blocks - auxSuperBlock->InodeBlocks - 1];
//     memset(bitmap, 0, sizeof(bool) * (auxSuperBlock->Blocks - auxSuperBlock->InodeBlocks - 1));

//     for (int i = 0; i < auxSuperBlock->Inodes; i ++) {
//             if (inodeBlock[i + getStartOfDataBlocks()].Valid)
//                 bitmap[i] = 1;
//     }
//     return 0;

// }

// FileSystem::FileSystem(Disk *disk)
// {
//     this->INODES_PER_BLOCK = disk->BLOCK_SIZE / 128;
//     this->POINTERS_PER_INODE = 5;
//     this->POINTERS_PER_BLOCK = disk->BLOCK_SIZE / 4;

//     this->superBlock = new char[Disk::BLOCK_SIZE]{};
//     this->totalInodes=FileSystem::ceilDiv(disk->blocks, 10) * Disk::BLOCK_SIZE;
//     this->inodeBlocks = new char[this->totalInodes]{};
// }

// FileSystem::~FileSystem()
// {
//     this->unmount(this->mountedDisk);
//     //printf("in fs desctructor\n");
//     FileSystem::mountedDisk = nullptr;
// /*     if (FileSystem::bitmap != nullptr){
//         delete[] FileSystem::bitmap;
//         FileSystem::bitmap = nullptr;
//     }
//     printf("deleted bitmap\n"); */
//     if (superBlock != nullptr){
//         delete superBlock;
//         superBlock = nullptr;
//     }
//     if (inodeBlocks != nullptr){
//         delete inodeBlocks;
//         inodeBlocks = nullptr;
//     }
// }

// void FileSystem::debug(Disk *disk)
// {
//     SuperBlock* auxBlock = reinterpret_cast<SuperBlock*>(superBlock);
//     if (auxBlock->MagicNumber != MAGIC_NUMBER)
//     {
//             fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxBlock->MagicNumber, MAGIC_NUMBER);
//             return;
//     }
//     else{
//     fprintf(stdout, "Magic number identified.\n");
//     fprintf(stdout, "Superblock:\n");
//     fprintf(stdout, "\tBlocks: %d\n", auxBlock->Blocks);
//     fprintf(stdout, "\tInode blocks: %d\n", auxBlock->InodeBlocks);
//     fprintf(stdout, "\tInodes: %d\n", auxBlock->Inodes);

//     //debug inodes
//     /* for (int i = 1; i < block.Super.InodeBlocks + 1; i ++){
//         disk->so_read(i, inode_block.Data);
//         fprintf(stdout, "Inode %d:", i);
//         FileSystem::debugInodes(inode_block);
//     } */
//     }
// }

// bool FileSystem::format(Disk *disk)
// {
//     //clear all data
//     printf("Formating disk...\n");
//     ftruncate(disk->descriptor, 0);

//     //scriem superblock-ul
//     SuperBlock *auxBlock = reinterpret_cast<SuperBlock*>(superBlock);
//     auxBlock->MagicNumber = 0x05112002;
//     auxBlock->Blocks = disk->blocks;
//     auxBlock->InodeBlocks = ceilDiv(disk->blocks, 10);
//     auxBlock->Inodes = auxBlock->InodeBlocks * INODES_PER_BLOCK;
//     bitmap = new bool[disk->blocks - auxBlock->InodeBlocks - 1];
//     memset(bitmap, 0, sizeof(bool) * disk->blocks - auxBlock->InodeBlocks - 1);

//     //alocam inodeBlocks
//     inodeBlocks = new char[auxBlock->InodeBlocks * Disk::BLOCK_SIZE];

//     //DEBUG
//     //printf("%d %d %d\n", auxBlock->Blocks, auxBlock->InodeBlocks, auxBlock->Inodes);
//     //
//     disk->so_write(0, superBlock, Disk::BLOCK_SIZE);
//     FileSystem::mountedDisk = disk;
//     disk->mounted = 1;

//     //ocupy superblock and blocks for inode
//     // for(int i = 0; i < auxBlock->InodeBlocks + 1; i ++){
//     //     bitmap[i] = true;
//     // }
//     printf("Disk formated\n");

// }

// bool FileSystem::mount(Disk *disk)
// {
//     if (disk->mounted) {
//         fprintf(stderr, "A file system already mounted.\n");
//         return 1;
//     }
//     if (disk->so_read(0, superBlock, Disk::BLOCK_SIZE) < 0) {
//         fprintf(stderr, "Error on reading superblock.\n");
//         return 1;
//     }

//     SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);

//     if (auxSuperBlock->MagicNumber != MAGIC_NUMBER) {
//         fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", auxSuperBlock->MagicNumber, MAGIC_NUMBER);
//         return 1;
//     }

//     printf("Mounting...\n");

//     //Inode* auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
//     printf("Initializing i-nodes...\n");

//     //read are limita, de reimplementat citire in chunck-uri
//     for (int i = 1; i <= auxSuperBlock->InodeBlocks; i ++) {
//         printf("\treading i-node <%ld>\n", i - 1);
//         disk->so_read(i, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
//     }

//     FileSystem::initBitmap(reinterpret_cast<Inode*>(FileSystem::inodeBlocks));

//     disk->mounted = 1;

//     FileSystem::mountedDisk = disk;

//     printf("Filesystem mounted.\n");

//     return 0;
// }

// bool FileSystem::unmount(Disk *disk)
// {
//     if (disk->mounted == 0) {
//         printf("FS already unmounted.\n");
//         return 0;
//     }

//     SuperBlock* auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
//     printf("Unmounting...\n");

//     printf("Saving super block...\n");
//     disk->so_write(0, FileSystem::superBlock, sizeof(SuperBlock));

//     printf("Saving i-nodes...\n");
//     for (int i = 1; i <= auxSuperBlock->InodeBlocks; i ++) {
//         printf("\tSaving inode block %d\n", i - 1);
//         //write(STDOUT_FILENO, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
//         disk->so_write(i, FileSystem::inodeBlocks + ((i - 1) * Disk::BLOCK_SIZE), Disk::BLOCK_SIZE);
//     }

//     disk->mounted = 0;

//     FileSystem::mountedDisk = nullptr;

//     printf("File system unmounted.\n");
//     return 0;
// }

// Inode FileSystem::getInode(size_t inumber)
// {
//     Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);

//     return inodes[inumber];
// }

// size_t FileSystem::getInumber(const char *filename)
// {
//     Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);

//     //get the inode of the filename
//     for(int i = 0; i < totalInodes ; i ++){

//         if(strncmp(inodes[i].Filename, filename, (strlen(filename) + 1)) == 0){
//             printf("I found file= %s at index= %d\n", filename, i);
//             return i;
//         }
//     }

//     //the filename does not exist
//     fprintf(stderr, "File %s does not exist!\n", filename);
//     return -1;
// }

// size_t FileSystem::create(const char *filename, uint32_t _OwnerUserID, uint32_t _OwnerGroupID, uint32_t _Permissions)
// {
//     if(strlen(filename) > MAX_FILENAME_LENGTH){
//         fprintf(stderr, "Length incorect at filename= %s\n", filename);
//         return -1;
//     }

//     Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);
//     for (int i = 0; i < this->totalInodes; i++) {
//         if (!inodes[i].Valid) {
//             inodes[i].Valid = 1;
//             inodes[i].Size = 0;
//             // inodes[i].Direct = new uint32_t[FileSystem::POINTERS_PER_INODE]{};

//             if (!inodes[i].Direct) {
//                 fprintf(stderr, "Failed to allocate memory for inode direct blocks.\n");
//                 return -1;
//             }

//             inodes[i].OwnerUserID = _OwnerUserID;
//             inodes[i].OwnerGroupID = _OwnerGroupID;
//             inodes[i].Permissions = _Permissions;

//             memset(inodes[i].Filename, '\0', MAX_FILENAME_LENGTH);

//             printf("Inode created with index %d.\n", i);
//             memcpy(inodes[i].Filename, filename, strlen(filename));

// /*             if(checkImportantFiles(filename, i))
//                 inodes[i].Size = 4095;
//             else
//                 inodes[i].Size = 0;
//  */
//             printf("Inode with inumber= %d filename= %s valid= %d size= %d  userID= %d groupID= %d permissions= %d created.\n", i, inodes[i].Filename, inodes[i].Valid, inodes[i].Size,inodes[i].OwnerUserID, inodes[i].OwnerGroupID, inodes[i].Permissions );

//             return i;
//         }
//     }

//     //reached the maximum size
//     fprintf(stderr,"Reached the maximum size\n");
//     return -1;
// }

// bool FileSystem::remove(size_t inumber)
// {
//     if (inumber >= this->totalInodes) {
//         fprintf(stderr, "Invalid inode number.\n");
//         return false;
//     }

//     Inode* inodes = reinterpret_cast<Inode*>(this->inodeBlocks);

//     if (!inodes[inumber].Valid) {
//         fprintf(stderr, "Inode %zu is not in use.\n", inumber);
//         return false;
//     }

//     // Free direct blocks
//     for (int i = 0; i < POINTERS_PER_INODE; i++) {
//         if (inodes[inumber].Direct[i]) {
//             bitmap[inodes[inumber].Direct[i]] = false;
//         }
//     }

//     delete[] inodes[inumber].Direct; // Correctly deallocate the memory

//     // Handle indirect blocks
//     if (inodes[inumber].Indirect != 0) {
//         char *data = new char[Disk::BLOCK_SIZE]{};
//         mountedDisk->so_read(inodes[inumber].Indirect, data, Disk::BLOCK_SIZE);
//         auto pointers = reinterpret_cast<uint32_t*>(data);

//         size_t indirectBlocks = ceilDiv(inodes[inumber].Size, Disk::BLOCK_SIZE) - POINTERS_PER_INODE;
//         for (int i = 0; i < indirectBlocks; i++) {
//             if (pointers[i]) {
//                 bitmap[pointers[i]] = false;
//             }
//         }

//         mountedDisk->so_write(inodes[inumber].Indirect, data, Disk::BLOCK_SIZE);
//         inodes[inumber].Indirect = 0;
//     }

//     // Invalidate the inode
//     inodes[inumber].Valid = 0;
//     inodes[inumber].OwnerGroupID = 0;
//     inodes[inumber].OwnerUserID = 0;
//     inodes[inumber].Size = 0;

//     return true;
// }

// statDetails FileSystem::stat(size_t inumber){
//     Inode *inodes = reinterpret_cast<Inode*>(inodeBlocks);

//     statDetails inodeDetails;

//     //set statDetails
//     inodeDetails.Valid = inodes[inumber].Valid;
//     inodeDetails.Size = inodes[inumber].Size;
//     inodeDetails.OwnerUserID = inodes[inumber].OwnerUserID;
//     inodeDetails.OwnerGroupID = inodes[inumber].OwnerGroupID;
//     inodeDetails.Permissions = inodes[inumber].Permissions;

//     inodes = NULL;

//     return inodeDetails;
// }

// //Doar pentru length corect
// size_t FileSystem::fs_read(size_t inumber, char *data, size_t length, size_t offset)
// {
//     SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
//     Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
//     char *start = nullptr;

//   /*   for(int i = fileSystemAPI::inumberUsersFile; i <= fileSystemAPI::inumberPasswordsFile; i ++)
//         printf("Inodes in fs_read: inumber= %d valid= %d filename= %s!\n", i, auxInodeBlocks[i].Valid, auxInodeBlocks[i].Filename);
//  */
//     if (!auxInodeBlocks[inumber].Valid)  {
//         fprintf(stderr, "Error on filesystem read. I-node invalid <%ld>.\n", inumber);
//         return -1;
//     }

//     //number of blocks ocuppied by the file to be read
//     size_t blocks_of_file = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE);

//     size_t minblock = FileSystem::floorDiv(offset, Disk::BLOCK_SIZE);
//     size_t maxblock = minblock + FileSystem::floorDiv(length, Disk::BLOCK_SIZE);

//     printf("blocks of file %ld minblock %ld maxblock %ld\n", blocks_of_file, minblock, maxblock);

//     if (minblock > blocks_of_file || blocks_of_file == 0)
//         return 0;

//     if (maxblock > blocks_of_file - 1) {
//         maxblock = blocks_of_file - 1;
//         length = auxInodeBlocks[inumber].Size - offset;
//     }

//     printf("blocks of file %ld minblock %ld maxblock %ld\n", blocks_of_file, minblock, maxblock);


//     start = (char*)mmap(NULL, (maxblock - minblock + 1) * Disk::BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);


//     printf("Memory mapped\n");

//     //LOADING
//     loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);

//     printf("Pages loaded.\n");

//     printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);

//     printf("%s\n", start);

//     memcpy(data , start + (offset % Disk::BLOCK_SIZE) * sizeof(char), length);

//     munmap(start, (maxblock - minblock + 1) * Disk::BLOCK_SIZE);
//     fflush(stdout);

//     return length;
// }

// size_t FileSystem::fs_write(size_t inumber, const char *data, size_t length, size_t offset)
// {
//     SuperBlock *auxSuperBlock = reinterpret_cast<SuperBlock*>(superBlock);
//     Inode *auxInodeBlocks = reinterpret_cast<Inode*>(inodeBlocks);
//     char *start = nullptr;

//     if (!auxInodeBlocks[inumber].Valid)  {
//         fprintf(stderr, "Error on filesystem write. I-node invalid <%ld>.\n", inumber);
//         return -1;
//     }


//     size_t minblock = FileSystem::floorDiv(offset, Disk::BLOCK_SIZE);
//     size_t maxblock = minblock + FileSystem::floorDiv(length, Disk::BLOCK_SIZE);
//     printf("minblock write: %ld maxblock write: %ld\n", minblock, maxblock);
//     /* size_t blocks = FileSystem::ceilDiv(auxInodeBlocks[inumber].Size, Disk::BLOCK_SIZE); //cate blocuri avem pentru i-node
//     size_t blocksToWrite = FileSystem::ceilDiv(length, Disk::BLOCK_SIZE); //cate blocuri au fost cerute a fi scrise
//     size_t totalBlocks = (blocksToWrite < blocks ? blocks : blocksToWrite); //maximul dintre blocks si blocksToWrite
//     //DEBUG
//     printf("Blocks of inode <%ld>: %ld\n", inumber, blocks);
//     printf("Blocks to write to inode <%ld>: %ld\n", inumber, blocksToWrite);
//     printf("Total blocks of inode <%ld>: %ld\n", inumber, totalBlocks); */
//     //

//     start = (char*)mmap(NULL, (maxblock - minblock + 1) * Disk::BLOCK_SIZE,
//      PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
//     //LOADING
//     //printf("%ld", auxInodeBlocks[inumber])

//     loadPages(start, inumber, auxInodeBlocks, minblock, maxblock);

//     //WRITING
//     printf("Size of inode <%ld>: %ld\n", inumber, auxInodeBlocks[inumber].Size);
//     memcpy(start + (offset % Disk::BLOCK_SIZE), data, length * sizeof(char));

//     printf("Data written.\n\t Data: %s\n\t Size: %ld\n", data, length);

//     ///SAVING
//     savePages(start, inumber, auxInodeBlocks, minblock, maxblock);
//     /* if (totalBlocks <= FileSystem::POINTERS_PER_INODE)
//         FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, totalBlocks);
//     else {
//         FileSystem::saveDirectPages(start, inumber, auxInodeBlocks, FileSystem::POINTERS_PER_INODE);
//         FileSystem::saveIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
//          auxInodeBlocks, totalBlocks - FileSystem::POINTERS_PER_INODE);
//     } */
//     //FileSystem::saveIndirectPages(start + FileSystem::POINTERS_PER_INODE * Disk::BLOCK_SIZE, inumber,
//     //auxInodeBlocks, totalBlocks - FileSystem::POINTERS_PER_INODE);

//     munmap(start, (maxblock - minblock + 1) * Disk::BLOCK_SIZE);

//     auxInodeBlocks[inumber].Size += length;

//     printf("Data saved on disk. Size of file: %ld\n", auxInodeBlocks[inumber].Size);

//     fflush(stdout);
//     return length;
// }

// fileSystemAPI *fileSystemAPI::instance = nullptr;
// User *fileSystemAPI::users = nullptr;
// FileSystem *fileSystemAPI::myFileSystem = nullptr;
// Disk *fileSystemAPI::disk = nullptr;
// size_t fileSystemAPI::totalUsers = 0;
// size_t fileSystemAPI::diskBlocks = 0;
// size_t fileSystemAPI::currentUser = 0;
// size_t fileSystemAPI::inumberUsersFile = 0;
// size_t fileSystemAPI::inumberPasswordsFile = 1;
// size_t fileSystemAPI::inumberGroupsFile = 2;

// fileSystemAPI::fileSystemAPI(Disk *disk_path, size_t disk_blocks)
// {
//     //initialize variables
//     this->diskBlocks = disk_blocks;
//     this->users = new User[MAX_USERS]();
//     this->totalUsers = 0;
//     this->disk = disk_path;
//     disk_path->blocks = disk_blocks;

//     //initialise File System
//     myFileSystem = new FileSystem(disk);

//     int ret = myFileSystem->mount(fileSystemAPI::disk);

//     printf("Ret: %d\n", ret);

//     if (ret == 1) {
//         formatFileSystem();
//         createFile(USERS_FILE, 1, 1, 0644);
//         createFile(PASSWORDS_FILE, 1, 1, 0644);
//     }


//     /* createFile(USERS_FILE, 1, 1, 0644);
//     createFile(PASSWORDS_FILE, 1, 1, 0644); */

//     // char *data = new char[Disk::BLOCK_SIZE];
//     // disk->so_read(inumberUsersFile, data);
//     // disk->so_read(inumberPasswordsFile, data);
//     // delete data;

//     //DEBUG
//     Inode* aux = reinterpret_cast<Inode*>(myFileSystem->inodeBlocks);

//     printf("\tFile user.txt: name: %s size: %ld\n", aux[inumberUsersFile].Filename, aux[inumberUsersFile].Size);

//     //read users who already exists
//     readImportantFile(USERS_FILE);
// }

// fileSystemAPI::~fileSystemAPI()
// {
//     //write users in file
//     writeImportantFile(USERS_FILE);
//     writeImportantFile(PASSWORDS_FILE);
//     myFileSystem->unmount(disk);
//     //delete this->users;
//    // delete this->myFileSystem;
// }

// bool fileSystemAPI::hasPermissions(const char *filename, uint32_t mode)
// {
//     size_t inumber = myFileSystem->getInumber(filename);
//     Inode inode = myFileSystem->getInode(inumber);
//     uint32_t tmp;
//     uint32_t mask;

//     //is the owner
//     if(inode.OwnerUserID == users[currentUser].userID){

//         //select owner permissions
//         mask = 0700;
//         tmp = (mask & inode.Permissions);

//     }

//     //has the same group
//     else if(inode.OwnerGroupID == users[currentUser].groupID){

//         // select group permissions
//         mask = 0070;
//         tmp = (mask & inode.Permissions);
//     }

//     //none of them
//     else  {

//         //select other permissions
//         mask = 0007;
//         tmp = (mask & inode.Permissions);
//     }

//     //has  permission for mode
//     if((tmp & mode) == mode)
//         return true;

//     return false;
// }

// fileSystemAPI *fileSystemAPI::getInstance(Disk *disk_path, size_t disk_blocks)
// {
//     //create instance if doesn't exist
//     if(!instance)
//         instance = new fileSystemAPI(disk_path, disk_blocks);

//     return instance;
// }

// void fileSystemAPI::destroyInstance()
// {
//     //destroy instance if exists
//     if(instance){
//         delete instance;
//         instance = nullptr;
//     }
// }

// bool fileSystemAPI::createUser(const char *username, const char *password, uint32_t userID)
// {
//     //length of username and password should be lower than the defined length
//     if(strlen(username) > USERNAME_LENGTH || strlen(password) > PASSWORD_LENGTH){
//         fprintf(stderr, "Length given incorrect!\n");
//         return false;
//     }

//     printf("Enter createUser()!\n");

//     size_t index_user;

//     //check if user exist
//     for(int i = 0; i < totalUsers; i ++){

//         //compare
//         if(strncmp(users[i].username, username, strlen(username) + 1) == 0){
//             fprintf(stderr, "User %s already exist!\n", username);
//             return false;
//         }
//     }

//     //find valid user index
//     for(int i = 0; i < MAX_USERS; i ++){
//         if(users[i].userID == 0){
//             index_user = i;
//             break;
//         }
//     }

//     //initialise username and password
//     users[index_user].username = new char[strlen(username) + 1]{};
//     users[index_user].password = new char[strlen(password) + 1]{};

//     //copy username and password
//     memcpy(users[index_user].username, username, strlen(username));
//     memcpy(users[index_user].password, password, strlen(password));

//     //set userID, groupID and permissons
//     users[index_user].userID = userID;
//     users[index_user].groupID = 0;
//     users[index_user].permissions = 6;
//     totalUsers++;

//     printf("S-a creat la index= %d: %s %s %d %d %d\n", index_user,users[index_user].username,users[index_user].password, users[index_user].userID, users[index_user].groupID, users[index_user].permissions);

//     printf("Exit create ok!\n");
//     return true;
// }

// bool fileSystemAPI::deleteUser(uint32_t userID)
// {
//     //printf("Enter delete!\n");
//     //check if user exist
//     for(int i = 0; i < totalUsers; i++){
//         if(users[i].userID == userID){
//             delete users[i].username;
//             delete users[i].password;
//             users[i].userID = 0;
//             users[i].groupID = 0;
//             users[i].permissions = 0;
//             totalUsers--;

//             printf("Exit delete ok!\n");
//             return true;
//         }
//     }

//     fprintf(stderr, "User with id %d doesn't exist!\n", userID);
//     //printf("Exit delete with error!\n");
//     return false;
// }

// bool fileSystemAPI::setUserGroup(uint32_t userID, uint32_t groupID)
// {
//     for(int i = 0; i < totalUsers; i ++){
//         if(users[i].userID == userID){
//             users[i].groupID = groupID;
//             return true;
//         }
//     }

//     fprintf(stderr, "User with ID %d not found!\n", userID);
//     return false;
// }

// bool fileSystemAPI::addUserToGroup(uint32_t userID, uint32_t groupID)
// {


//     return false;
// }

// bool fileSystemAPI::setFilePermissions(const char *filename, uint32_t permissions)
// {
//     size_t inumber = myFileSystem->getInumber(filename);

//     if(inumber == -1)
//         return false;

//     Inode inodes = myFileSystem->getInode(inumber);

//     //only the owner can change the permissions
//     if(users[currentUser].userID == inodes.OwnerUserID){
//         inodes.Permissions = permissions;
//         return true;
//     }

//     return false;
// }

// uint32_t fileSystemAPI::getFilePermissions(const char *filename)
// {
//     size_t inumber = myFileSystem->getInumber(filename);
//     Inode inode = myFileSystem->getInode(inumber);

//     return inode.Permissions;
// }

// bool fileSystemAPI::mountFileSystem()
// {
//     return myFileSystem->mount(disk);
// }

// bool fileSystemAPI::unmountFileSystem()
// {
//     return myFileSystem->mount(disk);
// }

// bool fileSystemAPI::formatFileSystem()
// {
//     return myFileSystem->format(disk);
// }

// ssize_t fileSystemAPI::createFile(const char *filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions)
// {
//     size_t ret;

//     ret = myFileSystem->create(filename, ownerUserID, ownerGroupID, permissions);

//     //printf("(Inside createFile)First: ret= %d\n%s\n and second:\n %s\n", ret, myFileSystem->inodeBlocks, FileSystem::inodeBlocks);

//     return ret;
// }

// bool fileSystemAPI::removeFile(const char *filename)
// {
//     size_t inumber = myFileSystem->getInumber(filename);

//     if(inumber == -1)
//         return false;

//     return myFileSystem->remove(inumber);
// }

// statDetails fileSystemAPI::getFileStat(const char *filename)
// {
//     size_t inumber = myFileSystem->getInumber(filename);

//     statDetails stats = myFileSystem->stat(inumber);

//     return stats;
// }

// ssize_t fileSystemAPI::readFile(const char *filename, char *data, size_t length, size_t offset)
// {
//     size_t inumber = myFileSystem->getInumber(filename);

//     //file doesn't exist
//     if(inumber == -1)
//         return -1;

//     size_t totalRead;
//     Inode inode = myFileSystem->getInode(inumber);

//     //handle the allocation of data
//     data = new char[length + 1]{};

//     //read if has permissions
//     if(hasPermissions(filename, READ_PERMISSION))
//         totalRead = myFileSystem->fs_read(inumber, data, length, offset);

//     return totalRead;
// }

// ssize_t fileSystemAPI::writeFile(const char *filename, const char *data, size_t length, size_t offset)
// {
//     size_t totalWrite = 0, inumber;
//     Inode inode;

//     inumber = myFileSystem->getInumber(filename);

//     //create if doesn't exist
//     if(inumber == -1){
//         createFile(filename, users[currentUser].userID, users[currentUser].groupID, 0644);
//         inumber = myFileSystem->getInumber(filename);
//     }

//     inode = myFileSystem->getInode(inumber);

//     //write if has the permissions
//     if(hasPermissions(filename, WRITE_PERMISSION))
//         totalWrite = myFileSystem->fs_write(inumber, data, length, offset);

//     return totalWrite;
// }

// bool fileSystemAPI::execute(const char *filename)
// {

//     return false;
// }

// void fileSystemAPI::readImportantFile(const char *filename)
// {
//     printf("\tEnter readImportantFile() with file= %s!\n", filename);
//     char *data, *token;

//     data = new char[Disk::BLOCK_SIZE + 1]{};

//     myFileSystem->fs_read(inumberUsersFile, data, Disk::BLOCK_SIZE, 0);

//     printf("\tData with length= %d data= %s\n", Disk::BLOCK_SIZE, data);

//     if(data[0] == '\0'){
//         fprintf(stderr, "\tFisierul %s nu este populat!\n", filename);
//         return;
//     }

//     printf("Fisierul %s este populat!\n", filename);

//     token = strtok(data, ":");

//     while(token != NULL){

//         //read username
//         memcpy(users[totalUsers].username, token, strlen(token) + 1);

//         //ignore password
//         token = strtok(NULL, ":");

//         //take userID
//         token = strtok(NULL, ":");
//         users[totalUsers].userID = atoi(token);

//         //take groupID
//         token = strtok(NULL, ":");
//         users[totalUsers].groupID = atoi(token);

//         //take permissions
//         token = strtok(NULL, "\n");
//         users[totalUsers].permissions = atoi(token);

//         //go next
//         token = strtok(NULL, ":");
//         totalUsers ++;
//     }

//     for(int i = 0; i < totalUsers; i ++){
//         printf("S-a citit de la index= %d: %s %s %d %d %d\n", i,users[i].username,users[i].password, users[i].userID, users[i].groupID, users[i].permissions);
//     }

//     delete data;

//     printf("Exit readUsers ok!\n");
// }

// void fileSystemAPI::writeImportantFile(const char *filename)
// {
//     printf("Enter writeImportantFile() with file= %s!\n", filename);

//     size_t inumber, length = 2*USERNAME_LENGTH, sizeRead = 0;
//     char *data, *line;

//     inumber = myFileSystem->getInumber(filename);

//     data = new char[Disk::BLOCK_SIZE + 1]{};

//     for(int i = 0; i < totalUsers; i ++){

//         line = new char [length + 2]{};

//         if(strncmp(USERS_FILE, filename, (strlen(USERS_FILE) + 1)) == 0){
//             snprintf(line, length,"%s:x:%d:%d:%d\n", users[i].username, users[i].userID, users[i].groupID, users[i].permissions);
//         }
//         else if(strncmp(PASSWORDS_FILE, filename, (strlen(PASSWORDS_FILE) + 1)) == 0){
//             snprintf(line, length, "%s:%s\n", users[i].username, users[i].password);
//          }

//         memcpy(data + sizeRead, line, strlen(line));
//         sizeRead += strlen(line);
//         delete line;
//     }

//     myFileSystem->fs_write(inumber, data, Disk::BLOCK_SIZE, 0);

//     printf("Data = \n%s", data);

//     delete data;

//     printf("Exit writeUsers ok!\n");
// }



// int main(){
//     Disk *disk = new Disk();
//     disk->disk_open("../../bin/file.txt", 20);
//     char* block1 = new char[Disk::BLOCK_SIZE];
//     char buffer[1024] = { 0 };
//     FileSystem *fs = new FileSystem(disk);
//     //FileSystem::format(disk);

//     // printf("\nUnmount test:\n");
//     // FileSystem::unmount(disk);

//     // printf("\nMount test:\n");
//     // FileSystem::mount(disk);

//     // printf("\nDebug function test:\n");
//     // FileSystem::debug(disk);

//     // printf("Size of Inode: %d\n", sizeof(Inode));

//     // // printf("\nso_read() function test on superblock:\n");
//     // // disk->so_read(0, block1);
//     // // SuperBlock* auxBlock1 = reinterpret_cast<SuperBlock*>(block1);
//     // // printf("Read from file:%x %d %d %d\n", auxBlock1->MagicNumber, auxBlock1->Blocks, auxBlock1->InodeBlocks, auxBlock1->Inodes);

//     //size_t testInode = fs->create("sefu.txt",1,1, 0666);

//     //fs->fs_write(testInode, "Ana are mere", sizeof("Ana are mere"), 0);

//     // fs->fs_read(0, buffer, 1024, 0);

//     // size_t testInode1 = fs->create("sefu1.txt",1,1, 0666);

//     // fs->fs_write(testInode1, "Ana are mere si pere", sizeof("Ana are mere si pere"), 0);

//     // fs->fs_read(testInode1, buffer, 1024, 0);

//     // printf("Data read: %s\n", buffer);

//     // delete fs;

//     // delete[] block1;


//     //FileSystemAPI tests
//      ///*
//     printf("\n\n Tests on fileSystemAPI\n\n");

//     fileSystemAPI* fsAPI = fileSystemAPI::getInstance(disk, 30);

//    /*  fsAPI->createUser("root", "rtqgoqmvp123.", 1);
//     fsAPI->createUser("SefuThau", "euSuntsmecheru", 8);
//     fsAPI->createUser("SefuThau", "euSuntsmecheru", 4);
//     fsAPI->createUser("Ciocanul","idolu_la_femei",2);
//     fsAPI->createUser("Ionel","cuceritorul",3);
//     fsAPI->createUser("Ciocanul","idolu_la_femei",5); */

//     fileSystemAPI::destroyInstance();


//     //*/


//     return 0;
// }
