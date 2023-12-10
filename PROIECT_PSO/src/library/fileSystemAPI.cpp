#include "../../includes/fileSystemAPI.h"

fileSystemAPI *fileSystemAPI::instance = nullptr;
User *fileSystemAPI::users = nullptr;
FileSystem *fileSystemAPI::myFileSystem = nullptr;
Disk *fileSystemAPI::disk = nullptr;
size_t fileSystemAPI::totalUsers = 0;
size_t fileSystemAPI::diskBlocks = 0;
size_t fileSystemAPI::currentUser = 0;
size_t fileSystemAPI::inumberUsersFile = 0;
size_t fileSystemAPI::inumberPasswordsFile = 0;
size_t fileSystemAPI::inumberGroupsFile = 0;

fileSystemAPI::fileSystemAPI(const char *disk_path, size_t disk_blocks)
{
    printf("sizeof(User): %d\n", sizeof(User));

    //initialize variables
    this->diskBlocks = disk_blocks;
    this->users = new User[MAX_USERS]();
    this->totalUsers = 0;

    //initialise Disk
    disk = new Disk();
    disk->disk_open(disk_path, disk_blocks);

    //initialise File System
    myFileSystem = new FileSystem(disk);

    formatFileSystem();

    createFile(USERS_FILE, 1, 1, 0644);
    createFile(PASSWORDS_FILE, 1, 1, 0644);

    // char *data = new char[Disk::BLOCK_SIZE];
    // disk->so_read(inumberUsersFile, data);
    // disk->so_read(inumberPasswordsFile, data);
    // delete data;

    //read users who already exists
    readImportantFile(USERS_FILE);
}

fileSystemAPI::~fileSystemAPI()
{
    //write users in file
    writeImportantFile(USERS_FILE);
    writeImportantFile(PASSWORDS_FILE);

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

    printf("Enter createUser()!\n");

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

    memcpy(users[index_user].username, username, USERNAME_LENGTH + 1);
    memcpy(users[index_user].password, password, PASSWORD_LENGTH + 1);

    users[index_user].username[USERNAME_LENGTH] = '\0';
    users[index_user].password[PASSWORD_LENGTH] = '\0';

    users[index_user].userID = userID;
    users[index_user].groupID = 0;
    users[index_user].permissions = 6;
    totalUsers++;

    printf("S-a creat la index= %d: %s %s %d %d %d\n", index_user,users[index_user].username,users[index_user].password, users[index_user].userID, users[index_user].groupID, users[index_user].permissions);

    //printf("Exit create ok!\n");
    return true;
}

bool fileSystemAPI::deleteUser(uint32_t userID)
{
    //printf("Enter delete!\n");
    //check if user exist
    for(int i = 0; i < totalUsers; i++){
        if(users[i].userID == userID){
            delete users[i].username;
            delete users[i].password;
            users[i].userID = 0;
            users[i].groupID = 0;
            users[i].permissions = 0;
            totalUsers--;

            printf("Exit delete ok!\n");
            return true;
        }
    }

    fprintf(stderr, "User with id %d doesn't exist!\n", userID);
    //printf("Exit delete with error!\n");
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
    return myFileSystem->mount(disk);
}

bool fileSystemAPI::unmountFileSystem()
{
    return myFileSystem->mount(disk);
}

bool fileSystemAPI::formatFileSystem()
{
    return myFileSystem->format(disk);
}

ssize_t fileSystemAPI::createFile(const char *filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions)
{
    size_t ret;
    
    ret = myFileSystem->create(filename, ownerUserID, ownerGroupID, permissions);

    //printf("(Inside createFile)First: ret= %d\n%s\n and second:\n %s\n", ret, myFileSystem->inodeBlocks, FileSystem::inodeBlocks);

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

void fileSystemAPI::readImportantFile(const char *filename)
{
    char *data, *token;

    data = new char[Disk::BLOCK_SIZE + 1]();

    myFileSystem->fs_read(inumberUsersFile, data, Disk::BLOCK_SIZE, 0);
    
    printf("Data with length= %d data= %s\n", Disk::BLOCK_SIZE, data);

    if(data[0] == '\0'){
        fprintf(stderr, "Fisierul %s nu este populat!\n", filename);
        return;
    }

    printf("Fisierul %s este populat!\n", filename);

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

    printf("Data with length= %d data= %s\n", Disk::BLOCK_SIZE, data);
    printf("Exit readUsers ok!\n");

}

void fileSystemAPI::writeImportantFile(const char *filename)
{   
    printf("Enter writeImportantFile() with file= %s!\n", filename);

    size_t inumber, length = 2*USERNAME_LENGTH, sizeRead = 0;
    char *data, *line;

    inumber = myFileSystem->getInumber(filename);

    data = new char[Disk::BLOCK_SIZE]();

    for(int i = 0; i < totalUsers; i ++){

        line = new char [length]();
        
        if(strncmp(USERS_FILE, filename, strlen(USERS_FILE)) == 0){
            snprintf(line, length,"%s:x:%d:%d:%d\n", users[i].username, users[i].userID, users[i].groupID, users[i].permissions);
        }
        else{
            snprintf(line, length, "%s:%s\n", users[i].username, users[i].password);
        }

        memcpy(data + sizeRead, line, strlen(line));
        sizeRead += strlen(line);
        delete line;
    }

    myFileSystem->fs_write(inumber, data, Disk::BLOCK_SIZE, 0);

    printf("Data = \n%s", data);

    //printf("Exit writeUsers ok!\n");
}
