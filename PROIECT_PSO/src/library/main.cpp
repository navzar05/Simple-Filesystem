/* #include "../../includes/disk_driver.h" */
#include "../../includes/shellProgram.h"

#define BLOCKS 30

int main(){
    Disk *disk = new Disk();
    disk->disk_open("./bin/file.txt", BLOCKS);

    ShellProgram *myShell = ShellProgram::getInstance(disk, BLOCKS);

    

    /*
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

    printf("Size of Inode: %d\n", sizeof(Inode));

    printf("\nso_read() function test on superblock:\n");
    disk->so_read(0, block1);
    SuperBlock* auxBlock1 = reinterpret_cast<SuperBlock*>(block1);
    printf("Read from file:%x %d %d %d\n", auxBlock1->MagicNumber, auxBlock1->Blocks, auxBlock1->InodeBlocks, auxBlock1->Inodes);

    size_t testInode = fs->create("sefu.txt",1,1, 0666);

    fs->fs_write(testInode, "Ana are mere", sizeof("Ana are mere"), 0);

    fs->fs_read(0, buffer, 1024, 0);

    size_t testInode1 = fs->create("sefu1.txt",1,1, 0666);

    fs->fs_write(testInode1, "Ana are mere si pere", sizeof("Ana are mere si pere"), 0);

    fs->fs_read(testInode1, buffer, 1024, 0);

    printf("Data read: %s\n", buffer);

    delete fs;

    delete[] block1;

    //FileSystemAPI tests

    printf("\n\n Tests on fileSystemAPI\n\n");

    fileSystemAPI* fsAPI = fileSystemAPI::getInstance(disk, 30);

    fsAPI->createUser("root", "rtqgoqmvp123.", 1);
    fsAPI->createGroup("root", 1);
    fsAPI->setUserGroup(1, 1);

    fsAPI->setCurrentUser(1);

    char data[] = "Hello everyone!\n";
    fsAPI->writeFile("scrisoare.txt", data, strlen(data), 0);
    fsAPI->writeFile("test.txt", data, strlen(data), 0);

    char *data2;
    data2 = new char[sizeof(data) + 1]{};

    fsAPI->readFile("scrisoare.txt", data2, sizeof(data), 0);

    fsAPI->createUser("Stefan Raileanu", "miaaMmaiua", 2);
    fsAPI->createUser("Rzvy", "euSunt Rzvy", 3);
    fsAPI->createUser("Fanelu", "fanetos", 4);

    fsAPI->createGroup("tocilarii", 2);
    fsAPI->setUserGroup(2,3);
    fsAPI->setUserGroup(3,2);
    fsAPI->setUserGroup(4,2);
    fsAPI->setUserGroup(2,2);

    fsAPI->deleteUser(2);

    fsAPI->setCurrentUser(3);
    fsAPI->writeFile("test.txt", data2, sizeof(data), 0);
    fsAPI->setFilePermissions("test.txt", 0666);

    fileSystemAPI::destroyInstance();

    printf("Final data: %s", data2);

    */
    
    return 0;
}
