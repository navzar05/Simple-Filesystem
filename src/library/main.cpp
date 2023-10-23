#include "../../includes/disk_driver.h"

int main(){
    Disk disk;
    disk.so_open("./bin/file.txt", 5);
    disk.so_write(0, (char*)"Ana are mere");
    disk.so_write(1, (char*)"Ana are avioane");
}
