#include "disk_driver.h"

const static uint32_t MAGIC_NUMBER       = 0x96960240;
const static uint32_t INODES_PER_BLOCK   = 128;
const static uint32_t POINTERS_PER_INODE = 5;
const static uint32_t POINTERS_PER_BLOCK = 1024;

struct SuperBlock {        
    uint32_t MagicNumber; 
    uint32_t Blocks;      
    uint32_t InodeBlocks;  
    uint32_t Inodes;        
    };

struct Inode {             
    uint32_t Valid;        
    uint32_t Size;         
    uint32_t Direct[POINTERS_PER_INODE]; 
    uint32_t Indirect;      
    };

union Block {
    SuperBlock  Super;                     
    Inode       Inodes[INODES_PER_BLOCK]; 
    uint32_t    Pointers[POINTERS_PER_BLOCK];
    char        Data[Disk::BLOCK_SIZE];
    };

class FileSystem {
private:
    static void debugInodes(Block block);
public:
    static void debug(Disk *disk);
    static bool format(Disk *disk);

    bool mount(Disk *disk);

    ssize_t create();
    bool    remove(size_t inumber);
    ssize_t stat(size_t inumber);

    ssize_t read(size_t inumber, char *data, size_t length, size_t offset);
    ssize_t write(size_t inumber, char *data, size_t length, size_t offset);
};
