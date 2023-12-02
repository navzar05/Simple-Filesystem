/* #include "../../includes/disk_driver.h" */
#include "../../includes/filesystem.h"

int main(){
    Disk *disk = new Disk();
    disk->disk_open("./bin/file.txt", 5);
    char* block1 = new char[Disk::BLOCK_SIZE];
    FileSystem *fs = new FileSystem(disk);
    FileSystem::format(disk);

    printf("\nUnmount test:\n");
    FileSystem::unmount(disk);

    printf("\nMount test:\n");
    FileSystem::mount(disk);

    printf("\nDebug function test:\n");
    FileSystem::debug(disk);

    printf("\nso_read() function test on superblock:\n");
    disk->so_read(0, block1);
    SuperBlock* auxBlock1 = reinterpret_cast<SuperBlock*>(block1);
    printf("Read from file:%x %d %d %d\n", auxBlock1->MagicNumber, auxBlock1->Blocks, auxBlock1->InodeBlocks, auxBlock1->Inodes);
    delete disk;
    delete fs;
    delete[] block1;
}
