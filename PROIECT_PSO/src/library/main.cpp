/* #include "../../includes/disk_driver.h" */
#include "../../includes/filesystem.h"

int main(){
    Disk *disk = new Disk();
    disk->disk_open("./bin/file.txt", 5);
    Block* block1 = (Block*)calloc(sizeof(Block), 1);
    FileSystem *fs = new FileSystem(disk);
    FileSystem::format(disk);
    FileSystem::readBlock(disk, 0, block1);
    printf("Read from file:%x %d %d %d\n", block1->Super.MagicNumber, block1->Super.Blocks, block1->Super.InodeBlocks, block1->Super.Inodes);
    delete disk;
    delete fs;
}
