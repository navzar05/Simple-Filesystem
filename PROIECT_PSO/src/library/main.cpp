/* #include "../../includes/disk_driver.h" */
#include "../../includes/fileSystemAPI.h"

int main(){
    Disk *disk = new Disk();
    disk->disk_open("./bin/file.txt", 20);
    char* block1 = new char[Disk::BLOCK_SIZE];
    char buffer[1024] = { 0 };
    FileSystem *fs = new FileSystem(disk);
    FileSystem::format(disk);

   /*  printf("\nUnmount test:\n");
    FileSystem::unmount(disk);

    printf("\nMount test:\n");
    FileSystem::mount(disk);

    printf("\nDebug function test:\n");
    FileSystem::debug(disk);

    printf("Size of Inode: %d\n", sizeof(Inode));

    // printf("\nso_read() function test on superblock:\n");
    // disk->so_read(0, block1);
    // SuperBlock* auxBlock1 = reinterpret_cast<SuperBlock*>(block1);
    // printf("Read from file:%x %d %d %d\n", auxBlock1->MagicNumber, auxBlock1->Blocks, auxBlock1->InodeBlocks, auxBlock1->Inodes);

    size_t testInode = fs->create("sefu.txt",1,1, 0666);

    fs->fs_write(testInode, "Ana are mere", sizeof("Ana are mere"), 0);

    fs->fs_read(testInode, buffer, 1024, 0);

    size_t testInode1 = fs->create("sefu1.txt",1,1, 0666);

    fs->fs_write(testInode1, "Ana are mere si pere", sizeof("Ana are mere si pere"), 0);

    fs->fs_read(testInode1, buffer, 1024, 0);

    printf("Data read: %s\n", buffer);

    delete[] block1;

    delete fs; */

    //FileSystemAPI tests
     printf("\n\n Tests on fileSystemAPI\n\n");

    fileSystemAPI* fsAPI = fileSystemAPI::getInstance("./bin/file.txt", 100);

    fsAPI->createUser("root", "rtqgoqmvp123.", 1);
    fsAPI->createUser("SefuThau", "euSuntsmecheru", 8);
    fsAPI->createUser("SefuThau", "euSuntsmecheru", 4);
    fsAPI->createUser("Ciocanul","idolu_la_femei",2);
    fsAPI->createUser("Ionel","cuceritorul",3);
    fsAPI->createUser("Ciocanul","idolu_la_femei",5);

    fileSystemAPI::destroyInstance();

    delete[] block1;

    delete fs;

    return 0;
}
