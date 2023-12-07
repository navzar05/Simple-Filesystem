#include "../../includes/fileSystemAPI.h"

fileSystemAPI *fileSystemAPI::instance = nullptr;
User *fileSystemAPI::users = nullptr;
FileSystem *fileSystemAPI::myFileSystem = nullptr;

fileSystemAPI::fileSystemAPI(const char *disk_path, size_t disk_blocks)
{
    //initialize variables
    this->diskBlocks = disk_blocks;
    this->users = new User[MAX_USERS]();

    //read users who already exists
    readUsersFile();

    //initialise Disk
    disk = new Disk();
    disk->disk_open(disk_path, disk_blocks);

    //initialise File System
    myFileSystem = new FileSystem(disk);
    
    formatFileSystem();
}

fileSystemAPI::~fileSystemAPI()
{
    //write users in file
    writeUsersFile(USERS_FILE);
    writeUsersFile(PASSWORDS_FILE);

    //delete this->users;
   // delete this->myFileSystem;
}

bool fileSystemAPI::hasPermissions(const char *filename, uint32_t mode)
{
    size_t inumber = myFileSystem->getInumber(filename);
    Inode inode = myFileSystem->getInode(inumber);
    uint32_t tmp;
    uint32_t mask;

    //is the owner
    if(inode.OwnerUserID == users[currentUser].userID){
        
        //select owner permissions
        mask = 0700;
        tmp = (mask & inode.Permissions);

    }

    //has the same group
    else if(inode.OwnerGroupID == users[currentUser].groupID){

        // select group permissions
        mask = 0070;
        tmp = (mask & inode.Permissions);
    }

    //none of them
    else  {

        //select other permissions
        mask = 0007;
        tmp = (mask & inode.Permissions);
    }

    //has  permission for mode
    if((tmp & mode) == mode)
        return true;

    return false;
}

fileSystemAPI *fileSystemAPI::getInstance(const char *disk_path, size_t disk_blocks)
{   
    //create instance if doesn't exist
    if(!instance)
        instance = new fileSystemAPI(disk_path, disk_blocks);

    return instance;
}

void fileSystemAPI::destroyInstance()
{
    //destroy instance if exists
    if(instance){
        delete instance;
        instance = nullptr;
    }
}

bool fileSystemAPI::createUser(const char *username, const char *password, uint32_t userID)
{
    size_t index_user;

    //check if user exist
    for(int i = 0; i < totalUsers; i ++){

        //compare
        if(strncmp(users[i].username, username, strlen(username) + 1) == 0){
            fprintf(stderr, "User %s already exist!\n", username);
            return false;
        }
    }

    for(int i = 0; i < MAX_USERS; i ++){
        if(users[i].userID == 0){
            index_user = i;
            break;
        }
    }

    users[index_user].username = new char[strlen(username) + 1];
    users[index_user].password = new char[strlen(password) + 1];

    memcpy((char*)users[index_user].username, username, strlen(username) + 1);
    memcpy((char*)users[index_user].password, password, strlen(password) + 1);

    users[index_user].userID = userID;
    users[index_user].groupID = 0;
    users[index_user].permissions = 6;    
    totalUsers++;

    return true;
}

bool fileSystemAPI::deleteUser(uint32_t userID)
{
    //check if user exist
    for(int i = 0; i < totalUsers; i++){
        if(users[i].userID == userID){
            delete users[i].username;
            delete users[i].password;
            users[i].userID = 0;
            users[i].groupID = 0;
            users[i].permissions = 0;
            totalUsers--;
            return true;
        }
    }

    fprintf(stderr, "User with id %d doesn't exist!\n", userID);
    return false;
}

bool fileSystemAPI::setUserGroup(uint32_t userID, uint32_t groupID)
{
    for(int i = 0; i < totalUsers; i ++){
        if(users[i].userID == userID){
            users[i].groupID = groupID;
            return true;
        }
    }

    fprintf(stderr, "User with ID %d not found!\n", userID);
    return false;
}

bool fileSystemAPI::addUserToGroup(uint32_t userID, uint32_t groupID)
{


    return false;
}

bool fileSystemAPI::setFilePermissions(const char *filename, uint32_t permissions)
{
    size_t inumber = myFileSystem->getInumber(filename);

    if(inumber == -1)
        return false;

    Inode inodes = myFileSystem->getInode(inumber);

    //only the owner can change the permissions
    if(users[currentUser].userID == inodes.OwnerUserID){
        inodes.Permissions = permissions;
        return true;
    }

    return false;
}

uint32_t fileSystemAPI::getFilePermissions(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);
    Inode inode = myFileSystem->getInode(inumber);

    return inode.Permissions;
}

bool fileSystemAPI::mountFileSystem()
{
    return FileSystem::mount(disk);
}

bool fileSystemAPI::unmountFileSystem()
{
    return FileSystem::unmount(disk);
}

bool fileSystemAPI::formatFileSystem()
{
    

    return FileSystem::format(disk);
}

ssize_t fileSystemAPI::createFile(const char *filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions)
{
    size_t ret;

    ret = myFileSystem->create(filename, ownerUserID, ownerGroupID, permissions);

    return ret;
}

bool fileSystemAPI::removeFile(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);

    if(inumber == -1)
        return false;

    return myFileSystem->remove(inumber);
}

statDetails fileSystemAPI::getFileStat(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);
    
    statDetails stats = myFileSystem->stat(inumber);

    return stats;
}

ssize_t fileSystemAPI::readFile(const char *filename, char *data, size_t length, size_t offset)
{
    size_t inumber = myFileSystem->getInumber(filename);

    //file doesn't exist
    if(inumber == -1)
        return -1;

    size_t totalRead;
    Inode inode = myFileSystem->getInode(inumber);

    //read if has permissions
    if(hasPermissions(filename, READ_PERMISSION))
        totalRead = myFileSystem->fs_read(inumber, data, length, offset);
    
    return totalRead;
}

ssize_t fileSystemAPI::writeFile(const char *filename, const char *data, size_t length, size_t offset)
{
    size_t totalWrite = 0, inumber;
    Inode inode;

    inumber = myFileSystem->getInumber(filename);

    //create if doesn't exist
    if(inumber == -1){
        createFile(filename, users[currentUser].userID, users[currentUser].groupID, 0644);
        inumber = myFileSystem->getInumber(filename);
    }

    inode = myFileSystem->getInode(inumber);

    //write if has the permissions
    if(hasPermissions(filename, WRITE_PERMISSION))
        totalWrite = myFileSystem->fs_write(inumber, data, length, offset);

    return totalWrite;
}

bool fileSystemAPI::execute(const char *filename)
{

    return false;
}

void fileSystemAPI::readUsersFile()
{
    char *data, *token;
    ssize_t length, inumber;

    inumber = myFileSystem->getInumber(USERS_FILE);

    if(inumber == -1){
        return;
    }

    length = myFileSystem->getInode(inumber).Size;

    readFile(USERS_FILE, data, length, 0);

    token = strtok(data, ":");

    while(token != NULL){

        //read username
        memcpy(users[totalUsers].username, token, strlen(token) + 1);

        //ignore password
        token = strtok(NULL, ":");
        
        //take userID
        token = strtok(NULL, ":");
        users[totalUsers].userID = atoi(token);

        //take groupID
        token = strtok(NULL, ":");
        users[totalUsers].groupID = atoi(token);

        //take permissions
        token = strtok(NULL, "\n");
        users[totalUsers].permissions = atoi(token);

        //go next
        token = strtok(NULL, ":");
        totalUsers ++;
    }

}

void fileSystemAPI::writeUsersFile(const char *filename)
{   
    size_t inumber, length = 2*USERNAME_LENGTH, sizeRead = 0;
    char *data, *line;

    inumber = myFileSystem->getInumber(filename);

    if(inumber == -1){
        createFile(filename, 1, 1, 0644);
        inumber = myFileSystem->getInumber(filename);
    }

    data = new char[totalUsers*length]();

    for(int i = 0; i < totalUsers; i ++){

        line = new char [length]();

        if(strncmp(USERS_FILE, filename, strlen(USERS_FILE)) == 0){
            snprintf(line,length,"%s:x:%d:%d:%d\n", users[i].username, users[i].userID, users[i].groupID, users[i].permissions);
        }
        else{
            snprintf(line ,length, "%s:%s\n", users[i].username, users[i].password);
        }

        memcpy(data + sizeRead, line, strlen(line));
        sizeRead += strlen(line);
        delete line;
    }

    myFileSystem->fs_write(inumber, data, sizeRead, 0);
}
