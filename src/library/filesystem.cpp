#include "../../includes/filesystem.h"

const static uint32_t MAGIC_NUMBER       = 0xf0f03410;
const static uint32_t INODES_PER_BLOCK   = 128;
const static uint32_t POINTERS_PER_INODE = 5;
const static uint32_t POINTERS_PER_BLOCK = 1024;

//implemented only for direct pointers
void FileSystem::debugInodes(Block block){
    if (block.Inodes->Valid){
        fprintf(stdout, "Size: %d\n", block.Inodes->Size);
        for (int i = 0; i < 5; i ++){
            
        }
    }
}

void FileSystem::debug(Disk *disk){
    Block block, inode_block;
    disk->so_read(0, block.Data);
    if (block.Super.MagicNumber != MAGIC_NUMBER){
        fprintf(stderr, "Disk is not formated. Bad magic number: it is %d not %d.\n", block.Super.MagicNumber, MAGIC_NUMBER);
        return;
    }
    else{
        fprintf(stdout, "Magic number identified.\n");
        fprintf(stdout, "Superblock:\n");
        fprintf(stdout, "\tBlocks: %d\n", block.Super.Blocks);
        fprintf(stdout, "\tInode blocks: %d\n", block.Super.InodeBlocks);
        fprintf(stdout, "\tInodes: %d\n", block.Super.Inodes);
        
        //debug inodes
        for (int i = 1; i < block.Super.InodeBlocks + 1; i ++){
            disk->so_read(i, inode_block.Data);
            fprintf(stdout, "Inode %d:", i);
            FileSystem::debugInodes(inode_block);
        }
    }
}
bool FileSystem::format(Disk *disk){
    
}

bool FileSystem::mount(Disk *disk){
    
}

ssize_t FileSystem::create(){
    
}
bool    FileSystem::remove(size_t inumber){
    
}
ssize_t FileSystem::stat(size_t inumber){
    
}

ssize_t FileSystem::read(size_t inumber, char *data, size_t length, size_t offset){
    
}
ssize_t FileSystem::write(size_t inumber, char *data, size_t length, size_t offset){

}