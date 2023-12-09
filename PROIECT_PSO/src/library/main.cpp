/* #include "../../includes/disk_driver.h" */
#include "../../includes/filesystem.h"

int main(){
    Disk *disk = new Disk();
    disk->disk_open("./bin/file.txt", 20);
    char* block1 = new char[Disk::BLOCK_SIZE];
    char buffer[1024] = { 0 };
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

    size_t testInode = fs->create(1, 1, 0666);

    printf("Created inode <%ld>\n", testInode);

    fs->fs_write(testInode, "Ana are mere", sizeof("Ana are mere"), 0);

    fs->fs_read(testInode, buffer, 1024, 2);

    printf("Data read from inode <%ld>: %s\n", testInode, buffer);

    size_t testInode1 = fs->create(1, 1, 0666);

    printf("Created inode <%ld>\n", testInode1);


    fs->fs_write(testInode1, "Ana are avioane", sizeof("Ana are avioane"), 0);

    fs->fs_read(testInode1, buffer, 1024, 2);

    printf("Data read inode <%ld>: %s\n", testInode, buffer);

    delete[] block1;
    delete disk;
    delete fs;
}
