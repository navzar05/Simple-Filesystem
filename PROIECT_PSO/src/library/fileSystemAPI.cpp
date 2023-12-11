#include "../../includes/fileSystemAPI.h"

fileSystemAPI *fileSystemAPI::instance = nullptr;
User *fileSystemAPI::users = nullptr;
FileSystem *fileSystemAPI::myFileSystem = nullptr;
Disk *fileSystemAPI::disk = nullptr;
Group *fileSystemAPI::groups = nullptr;

size_t fileSystemAPI::totalUsers = 0;
size_t fileSystemAPI::totalGroups = 0;
size_t fileSystemAPI::diskBlocks = 0;
size_t fileSystemAPI::currentUser = 0;

bool fileSystemAPI::isUsersFile = false;
bool fileSystemAPI::isPasswordsFile = false;
bool fileSystemAPI::isGroupsFile = false;

fileSystemAPI::fileSystemAPI(Disk *disk_path, size_t disk_blocks)
{
    //initialize variables0
    this->diskBlocks = disk_blocks;
    this->users = new User[MAX_USERS]{};
    this->groups = new Group[MAX_GROUPS]{};
    this->disk = disk_path;
    disk_path->blocks = disk_blocks;

    //initialise File System
    myFileSystem = new FileSystem(disk);

    int ret = myFileSystem->mount(fileSystemAPI::disk);

    //printf("Ret: %d\n", ret);

    if (ret != 0) {
        formatFileSystem();
        createFile(USERS_FILE, 1, 1, 0644);
        createFile(PASSWORDS_FILE, 1, 1, 0644);
        createFile(GROUPS_FILE, 1, 1, 0644);
    }

    readImportantFile(USERS_FILE);
    readImportantFile(PASSWORDS_FILE);
    readImportantFile(GROUPS_FILE);
}

fileSystemAPI::~fileSystemAPI()
{
    //write in important files
    writeImportantFile(USERS_FILE);
    writeImportantFile(PASSWORDS_FILE);
    writeImportantFile(GROUPS_FILE);

    unmountFileSystem();
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

fileSystemAPI *fileSystemAPI::getInstance(Disk *disk_path, size_t disk_blocks)
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
    printf("Enter createUser()!\n");

    size_t index_user;
    bool isSpace = false;

    //can't create if is maximum users
    if(totalUsers == MAX_USERS){
        fprintf(stderr, "Maximum users reached!\n");
    }

    //length of username and password should be lower than the defined length
    if(strlen(username) > USERNAME_LENGTH){
        fprintf(stderr, "Length incorrect at username!\n");
        return false;
    }

    else if(strlen(password) > PASSWORD_LENGTH){
        fprintf(stderr, "Length incorrect at password!\n");
        return false;
    }

    //check if user exist
    for(int i = 0; i < totalUsers; i ++){

        //compare
        if(strncmp(users[i].username, username, strlen(username) + 1) == 0){
            fprintf(stderr, "User %s already exist!\n", username);
            return false;
        }
    }

    //find valid user index
    for(int i = 0; i < MAX_USERS; i ++){
        if(users[i].userID == 0){
            index_user = i;
            isSpace = true;
            break;
        }
    }

    if(!isSpace){
        fprintf(stderr, "Maximum size at users reached!\n");
        return false;
    }

    //initialise username and password
    users[index_user].username = new char[strlen(username) + 1]{};
    users[index_user].password = new char[strlen(password) + 1]{};

    //copy username and password
    memcpy(users[index_user].username, username, strlen(username));
    memcpy(users[index_user].password, password, strlen(password));

    //set userID, groupID and permissons
    users[index_user].userID = userID;
    users[index_user].groupID = 0;
    users[index_user].permissions = 6;
    totalUsers++;

    printf("S-a creat la index= %d: %s %s %d %d %d\n", index_user,users[index_user].username,users[index_user].password, users[index_user].userID, users[index_user].groupID, users[index_user].permissions);

    printf("Exit create ok!\n");
    return true;
}

bool fileSystemAPI::deleteUser(uint32_t userID)
{
    //printf("Enter delete!\n");
    //check if user exist
    for(int i = 0; i < totalUsers; i++){
        if(users[i].userID == userID){

            //reset if exists in a group
            for(int j = 0; j < totalGroups; j ++){
                for(int k = 0; k < groups[j].nrUsers; k ++){
                    if(groups[j].usersID[k] == userID)
                        groups[j].usersID[k] = 0;
                }
            }

            delete users[i].username;
            delete users[i].password;
            users[i].userID = 0;
            users[i].groupID = 0;
            users[i].permissions = 0;

            printf("Exit delete ok!\n");
            return true;
        }
    }

    fprintf(stderr, "User with id %d doesn't exist!\n", userID);
    return false;
}

bool fileSystemAPI::setUserGroup(uint32_t userID, uint32_t groupID)
{
    //check if has space
    for(int i = 0; i < totalGroups; i ++){
        if(groups[i].groupID == groupID && groups[i].nrUsers == MAX_USERS){
            fprintf(stderr, "No more space for another user in group= %s\n", groups[i].groupname);
            return false;
        }
    }

    //find the user
    for(int i = 0; i < totalUsers; i ++){
        if(users[i].userID == userID){

            //check if already has the groupID desired
            if(users[i].groupID == groupID){
                fprintf(stderr, "User= %s already has the groupID= %d\n", users[i].username, groupID);
                return false;
            }

            //reset groupID for user if he is a traitor
            users[i].groupID = 0;

            //find the groupID
            for(int j = 0; j < totalGroups; j ++){

                //delete user if exist in another group
                for(int k = 0; k < groups[j].nrUsers; k ++){
                    if(groups[j].usersID[k] == userID && groups[j].groupID != groupID){
                        groups[j].usersID[k] == 0;
                    }
                }

                //change if exists and update the users from group
                if(groups[j].groupID == groupID){
                    users[i].groupID = groupID;
                    groups[j].usersID[groups[j].nrUsers] = userID;
                    groups[j].nrUsers ++;

                    printf("User= %d changed his group to= %d\n", userID, groupID);
                    return true;
                }
            }

            fprintf(stderr, "Group with ID= %d not found!\n", groupID);
            return false;
        }
    }

    fprintf(stderr, "User with ID= %d not found!\n", userID);
    return false;
}

bool fileSystemAPI::createGroup(const char *groupname, uint32_t groupID)
{   
    size_t index;
    bool isSpace = false;

    //check groupname length
    if(strlen(groupname) > GROUP_LENGTH){
        fprintf(stderr, "Length incorrect at groupname!\n");
        return false;
    }
    
    //check if already exists
    for(int i = 0; i < totalGroups; i ++){
        if(strncmp(groups[i].groupname, groupname, (strlen(groupname) + 1)) == 0){
            fprintf(stderr,"Group with name= %s already exists!\n");
            return false;
        }

        else if(groups[i].groupID == groupID){
            fprintf(stderr, "Group with ID= %d already exists!\n", groupID);
            return false;
        }
    }

    //set index
    for(int i = 0; i < MAX_GROUPS; i ++){
        if(groups[i].groupID == 0){
            index = i;
            isSpace = true;
            break;
        }
    }

    //no space
    if(!isSpace){
        fprintf(stderr, "Maximum groups reached!\n");
        return false;
    }

    //allocate
    groups[index].groupname = new char[strlen(groupname) + 1]{};
    groups[index].usersID = new uint32_t[MAX_USERS]{};

    memcpy(groups[index].groupname, groupname, strlen(groupname));
    groups[index].groupID = groupID;
    groups[index].nrUsers = 0;
    totalGroups++;

    printf("Group with name= %s and ID= %d created!\n", groups[index].groupname, groups[index].groupID);

    return true;
}

bool fileSystemAPI::deleteGroup(uint32_t groupID)
{
    for(int i = 0; i < totalGroups; i ++){
        if(groups[i].groupID == groupID){
            
            //reset groupID if exists in a user to 0
            for(int j = 0; j < groups[i].nrUsers; j ++){
                for(int k = 0; k < totalUsers; k ++){
                    if(groups[i].usersID[j] == users[k].userID)
                        users[k].groupID = 0;
                }
            }

            delete groups[i].groupname;
            delete groups[i].usersID;

            printf("Group with name= %s and ID= %d deleted!\n", groups[i].groupname, groups[i].groupID);

            return true;
        }
    }

    fprintf(stderr, "Group with ID= %d not found!\n", groupID);
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
    return myFileSystem->unmount(disk);
}

bool fileSystemAPI::formatFileSystem()
{
    return myFileSystem->format(disk);
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

    //handle the allocation of data
    data = new char[length + 1]{};

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
    printf("\tEnter readImportantFile() with file= %s!\n", filename);
    char *data, *token, *str;
    int index = 0;
    size_t inumber;

    data = new char[Disk::BLOCK_SIZE + 1]{};

    //find inumber and read
    inumber = myFileSystem->getInumber(filename);
    myFileSystem->fs_read(inumber, data, Disk::BLOCK_SIZE, 0);

    printf("\tData with length= %d data= %s\n", Disk::BLOCK_SIZE, data);

    //check if is populate
    if(data[0] == '\0'){
        fprintf(stderr, "\tFisierul %s nu este populat!\n", filename);
        return;
    }

    printf("Fisierul %s este populat!\n", filename);

    //allocate for strtok in another string where I have permissions
    str = new char[Disk::BLOCK_SIZE + 1]{};
    memcpy(str, data, Disk::BLOCK_SIZE);

    //see what file is
    seeTypeImportantFile(filename);

    //choose how to tokenize
    if(isGroupsFile)
        token = strtok(str, "\n");
    else
        token = strtok(str, ":");

    while(token != NULL){
        printf("token at start= %s\n", token);
        //call specific function
        if(isUsersFile)
            readUsersFile(token, index);

        else if(isPasswordsFile)
            readPassswordsFile(token, index);

        else if(isGroupsFile)
            readGroupsFile(token, index);

        //go next
        if(isGroupsFile)
            token = strtok(NULL, ":\n");
        else
            token = strtok(NULL, ":");

        index ++;

        printf("token at the end= %s\n", token);
    }

    //set total
    if(isUsersFile)
        totalUsers = index;

    else if(isGroupsFile)
        totalGroups = index;

    for(int i = 0; i < totalUsers; i ++){
        printf("S-a citit de la index= %d: %s %s %d %d %d\n", i,users[i].username,users[i].password, users[i].userID, users[i].groupID, users[i].permissions);
    }

    delete data;

    printf("Exit readUsers ok!\n");
}

void fileSystemAPI::readUsersFile(const char *token, int index)
{
    //read username 
    printf("username= %s\t", token);
    users[index].username = new char[strlen(token) + 1]{};
    memcpy(users[index].username, token, strlen(token));
    token = strtok(NULL, ":");

    //ignore password
    printf("password= %s\t", token);
    token = strtok(NULL, ":");

    //take userID
    printf("userID= %s\t", token);
    users[index].userID = atoi(token);
    token = strtok(NULL, ":");

    //take groupID
    printf("groupID= %s\t", token);
    users[index].groupID = atoi(token);
    token = strtok(NULL, "\n");

    //take permissions
    printf("permissions= %s\n", token);
    users[index].permissions = atoi(token);
}

void fileSystemAPI::readPassswordsFile(const char *token, int index)
{
    //read username
    printf("username= %s\t", token);
    token = strtok(NULL, "\n");

    //read password
    printf("password= %s\n", token);
    users[index].password = new char[strlen(token) + 1]{};
    memcpy(users[index].password, token, strlen(token));
}

void fileSystemAPI::readGroupsFile(const char *token, int index)
{
    printf("Enter readGroupsFile!\n");
    char *str2, *token2;

    //alloc
    str2 = new char[strlen(token) + 1]{};
    memcpy(str2, token, strlen(token));

    printf("str2= %s\n", str2);

    token2 = strtok(str2, ":");

    printf("token2= %s\n", token2);

    //read groupname
    printf("groupname= %s\t", token2);
    groups[index].groupname = new char[strlen(token2) + 1]{};
    token2 = strtok(NULL, ":");

    //skip after x
    token2 = strtok(NULL, ":");

    //take groupID
    printf("groupID= %s\t", token2);
    groups[index].groupID = atoi(token2);
    token2 = strtok(NULL, ":\n");

    //check if has users
    if(token2 == NULL){
        printf("Has no users!\n");
        return;
    }
    
    //read user by user
    while(token2 != NULL){
        printf(" user[%d]= %s\n",groups[index].nrUsers, token2);
        groups[index].usersID[groups[index].nrUsers] = atoi(token2);
        groups[index].nrUsers ++;

        token2 = strtok(NULL, ":");
    }

    delete str2;
}

void fileSystemAPI::writeImportantFile(const char *filename)
{
    printf("Enter writeImportantFile() with file= %s!\n", filename);

    size_t inumber, length = 2*USERNAME_LENGTH, sizeRead = 0, totalToRead;
    char *data, *line;

    //get the inumber for the current file
    inumber = myFileSystem->getInumber(filename);

    data = new char[Disk::BLOCK_SIZE + 1]{};

    //see what file is
    seeTypeImportantFile(filename);

    //select how much to read
    if(isGroupsFile)
        totalToRead = totalGroups;
    else
        totalToRead = totalUsers;

    printf("Have totalGroups= %d\n", totalGroups);

    //write in line
    for(int i = 0; i < totalToRead; i ++){

        line = new char [length + 2]{};

        //select how to write
        if(isUsersFile)
            writeUsersFile(line, length, i);

        else if(isPasswordsFile)
            writePasswordsFile(line, length, i);

        else if(isGroupsFile)
            writeGroupsFile(line, length, i);
        

        memcpy(data + sizeRead, line, strlen(line));
        sizeRead += strlen(line);
        delete line;
    }

    myFileSystem->fs_write(inumber, data, Disk::BLOCK_SIZE, 0);

    printf("Data = \n%s", data);

    delete data;

    printf("Exit writeUsers ok!\n");
}

void fileSystemAPI::writeUsersFile(char *line, size_t length, int i)
{
    printf("Enter writeUsersFile!\n");

    if(users[i].userID != 0)
        snprintf(line, length,"%s:x:%d:%d:%d\n", users[i].username, users[i].userID, users[i].groupID, users[i].permissions);
    else
        line[0] = '\0';
}

void fileSystemAPI::writePasswordsFile(char *line, size_t length, int i)
{
    printf("Enter writePasswordsFile!\n");

    if(users[i].userID != 0)
        snprintf(line, length, "%s:%s\n", users[i].username, users[i].password);
    else
        line[0] = '\0';    
}

void fileSystemAPI::writeGroupsFile(char *line, size_t length, int i)
{   
    printf("Enter writeGroupsFile!\n");

    int totalWritten = 0;

    //read name and groupID
    int written = snprintf(line, length, "%s:x:%d", groups[i].groupname, groups[i].groupID);
    totalWritten += written;

    //read every user from that group
    for(int j = 0; j < groups[i].nrUsers; j ++){
        written = snprintf(line + totalWritten, length - totalWritten, ":%d", groups[i].usersID[j]);
        totalWritten += written;
    }

    snprintf(line + totalWritten, length - totalWritten, "\n");
}

void fileSystemAPI::seeTypeImportantFile(const char *filename)
{
    isUsersFile = false;
    isPasswordsFile = false;
    isGroupsFile = false;

    if(strncmp(USERS_FILE, filename, (strlen(filename) + 1)) == 0)
        isUsersFile = true;

    else if(strncmp(PASSWORDS_FILE, filename, (strlen(filename) + 1)) == 0)
        isPasswordsFile = true;

    else if(strncmp(GROUPS_FILE, filename, (strlen(filename) + 1)) == 0)
        isGroupsFile = true;
}

