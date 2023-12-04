#include "../../includes/fileSystemAPI.h"

fileSystemAPI *fileSystemAPI::instance = nullptr;
User *fileSystemAPI::users = nullptr;

fileSystemAPI::fileSystemAPI(const char *disk_path, size_t disk_blocks)
{
    //initialize variables
    this->diskPath = disk_path;
    this->diskBlocks = disk_blocks;
    this->users = new User[MAX_USERS]();

    //read users who already exists
    readUsersFile();
}

fileSystemAPI::~fileSystemAPI()
{
    //write users in file
    writeUsersFile();
    delete this->users;
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
    //check if user exist
    for(int i = 0; i < totalUsers; i ++){

        //compare
        if(strncmp(users[i].username, username, strlen(username) + 1) == 0){
            fprintf(stderr, "User %s already exist!\n", username);
            return false;
        }
    }

    users[totalUsers].username = new char[strlen(username) + 1];
    users[totalUsers].password = new char[strlen(password) + 1];

    memcpy((char*)users[totalUsers].username, username, strlen(username) + 1);
    memcpy((char*)users[totalUsers].password, password, strlen(password) + 1);

    users[totalUsers].userID = userID;
    users[totalUsers].groupID = 0;
    users[totalUsers].permissions = 0;    
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
    return false;
}

uint32_t fileSystemAPI::getFilePermissions(const char *filename)
{
    return 0;
}

ssize_t fileSystemAPI::createFile(const char *filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions)
{
    return ssize_t();
}

bool fileSystemAPI::removeFile(const char *filename)
{
    return false;
}

statDetails fileSystemAPI::getFileStat(const char *filename)
{
    return statDetails();
}

ssize_t fileSystemAPI::readFile(const char *filename, char *data, size_t length, size_t offset)
{
    return ssize_t();
}

ssize_t fileSystemAPI::writeFile(const char *filename, const char *data, size_t length, size_t offset)
{
    return ssize_t();
}

bool fileSystemAPI::execute(const char *filename)
{
    return false;
}

void fileSystemAPI::readUsersFile()
{
    FILE *f=fopen(USERS_FILE, "r");
    int rc;

    fscanf(f,"Total_Users:\t%d\n\n", &totalUsers);
    fscanf(f,"Username\tPassword\tUserID\tGroupID\tPermissions\n");

    //read user by user
    for(int i = 0; i < totalUsers; i ++){

        users[i].username = new char[USERNAME_LENGTH]();
        users[i].password = new char[PASSWORD_LENGTH]();

        rc = fscanf(f,"%s\t%s\t%d\t%d\t%d\n", users[i].username, users[i].password, &users[i].userID, &users[i].groupID, &users[i].permissions);

        if(rc != 5){
            fprintf(stderr, "Error on reading users from users_file.txt, read %d variables!\n", rc);
            return;
        }

        //printf("Users[%d] has: %s %s %d %d %d\n", i, users[i].username, users[i].password, users[i].userID, users[i].groupID, users[i].permissions);
    }

    fclose(f);
}

void fileSystemAPI::writeUsersFile()
{
    FILE *f=fopen(USERS_FILE, "w");

    fprintf(f,"Total_Users:\t%d\n\n", totalUsers);
    fprintf(f,"Username\tPassword\tUserID\tGroupID\tPermissions\n");

    //write user by user then free the memory
    for(int i = 0; i < totalUsers; i ++){
        fprintf(f,"%s\t%s\t%d\t%d\t%d\n", users[i].username, users[i].password, users[i].userID, users[i].groupID, users[i].permissions);

        delete users[i].username;
        delete users[i].password;
    }

    fclose(f);

}
