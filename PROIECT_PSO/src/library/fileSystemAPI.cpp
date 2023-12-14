#include "../../includes/fileSystemAPI.h"

FileSystemAPI *FileSystemAPI::instance = nullptr;
User *FileSystemAPI::users = nullptr;
FileSystem *FileSystemAPI::myFileSystem = nullptr;
Group *FileSystemAPI::groups = nullptr;
Disk *FileSystemAPI::disk = nullptr;

size_t FileSystemAPI::totalUsers = 0;
size_t FileSystemAPI::totalGroups = 0;
size_t FileSystemAPI::diskBlocks = 0;
size_t FileSystemAPI::currentUser = 0;

bool FileSystemAPI::isUsersFile = false;
bool FileSystemAPI::isPasswordsFile = false;
bool FileSystemAPI::isGroupsFile = false;
bool *FileSystemAPI::bitmapUsers = nullptr;
bool *FileSystemAPI::bitmapGroups = nullptr;

FileSystemAPI::FileSystemAPI(Disk *disk_path, size_t disk_blocks)
{
    //initialize variables
    this->diskBlocks = disk_blocks;
    this->users = new User[MAX_USERS]{};
    this->groups = new Group[MAX_GROUPS]{};
    this->disk = disk_path;
    this->bitmapUsers = new bool[MAX_USERS]{};
    this->bitmapGroups = new bool[MAX_GROUPS]{};

    //initialise File System
    myFileSystem = new FileSystem(disk);

    //format if is not mounted
    if(!mountFileSystem())
        formatFileSystem();

    //create imporant files
    createFile(USERS_FILE, 1, 1, 0644);
    createFile(PASSWORDS_FILE, 1, 1, 0644);
    createFile(GROUPS_FILE, 1, 1, 0644);
   
    //read if was data before start the File System
    readImportantFile(USERS_FILE);
    readImportantFile(PASSWORDS_FILE);
    readImportantFile(GROUPS_FILE);
}

FileSystemAPI::~FileSystemAPI()
{
    //write in important files
    writeImportantFile(USERS_FILE);
    writeImportantFile(PASSWORDS_FILE);
    writeImportantFile(GROUPS_FILE);

    //unmount and delete
    unmountFileSystem();
    delete[] this->users;
    delete[] this->groups;
    delete[] this->bitmapUsers;
    delete[] this->bitmapGroups;
    //delete this->myFileSystem;
}

bool FileSystemAPI::hasPermissions(const char *filename, uint32_t mode)
{
    size_t inumber = myFileSystem->getInumber(filename);
    Inode inode = myFileSystem->getInode(inumber);
    uint32_t tmp, mask, userRights;

    //is the owner
    if(inode.OwnerUserID == users[currentUser].userID){

        //select owner permissions and shift 6 bytes
        mask = 0700;
        tmp = (mask & inode.Permissions);
        tmp = (tmp >> 6);
    }

    //has the same group
    else if(inode.OwnerGroupID == users[currentUser].groupID){

        // select group permissions and shif 3 bytes
        mask = 0070;
        tmp = (mask & inode.Permissions);
        tmp = (tmp >> 3);
    }

    //none of them
    else  {

        //select other permissions
        mask = 0007;
        tmp = (mask & inode.Permissions);
    }

    //select from current user his rights
    userRights = (users[currentUser].permissions & mode);

    //has  permission for mode
    if((tmp & userRights) == mode)
        return true;

    fprintf(stderr, "User= %s has can't do mode= %d on file= %s\n", users[currentUser].username, mode, filename);
    return false;
}

FileSystemAPI *FileSystemAPI::getInstance(Disk *disk_path, size_t disk_blocks)
{
    //create instance if doesn't exist
    if(!instance)
        instance = new FileSystemAPI(disk_path, disk_blocks);

    return instance;
}

void FileSystemAPI::destroyInstance()
{
    //destroy instance if exists
    if(instance){
        delete instance;
        instance = nullptr;
    }
}

bool FileSystemAPI::createUser(const char *username, const char *password, uint32_t userID)
{
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
            fprintf(stderr, "User with username= %s already exists!\n", username);
            return false;
        }

        else if(users[i].userID == userID){
            fprintf(stderr, "User with ID= %d already exists!\n", userID);
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

    //no space
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

    //set userID, groupID and permissons and bitmap
    users[index_user].userID = userID;
    users[index_user].groupID = 0;
    users[index_user].permissions = 06;
    bitmapUsers[users[index_user].userID] = true;
    totalUsers++;

    printf("User= %s with pass= %s and ID= %d created!\n", users[index_user].username, users[index_user].password, users[index_user].userID);

    return true;
}

bool FileSystemAPI::deleteUser(uint32_t userID)
{
    //check if user exist
    for(int i = 0; i < totalUsers; i++){

        if(users[i].userID == userID){

            //reset if exists in a group
            for(int j = 0; j < totalGroups; j ++){
                for(int k = 0; k < groups[j].nrUsers; k ++){
                    if(groups[j].usersID[k] == userID){
                        groups[j].usersID[k] = 0;   
                    }
                }
            }

            //free memory
            delete users[i].username;
            delete users[i].password;
            users[i].userID = 0;
            users[i].groupID = 0;
            users[i].permissions = 0;
            bitmapUsers[i] = false;

            return true;
        }
    }

    fprintf(stderr, "User with id %d doesn't exist!\n", userID);
    return false;
}

bool FileSystemAPI::setUserGroup(uint32_t userID, uint32_t groupID)
{
    printf("Enter setUserGroup()\n");

    bool hasChanged = false, findGroup = false;

    //check if has space
    for(int i = 0; i < totalGroups; i ++){

        if(groups[i].groupID == groupID && groups[i].nrUsers == MAX_USERS){
            fprintf(stderr, "No more space for another user in group= %s\n", groups[i].groupname);
            return false;
        }
        else if(groups[i].groupID == groupID)
                findGroup = true;
    }

    if(!findGroup){
        fprintf(stderr, "Group with ID= %d not found!\n", groupID);
        return false;
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
                        groups[j].nrUsers --;
                    }
                }

                //change if exists and update the users from group
                if(groups[j].groupID == groupID){
                    users[i].groupID = groupID;

                    groups[j].usersID[groups[j].nrUsers] = userID;
                    groups[j].nrUsers ++;

                    hasChanged = true;
                }
            }
        }
    }

    if(hasChanged)
        return true;

    fprintf(stderr, "User with ID= %d not found!\n", userID);
    return false;
}

bool FileSystemAPI::createGroup(const char *groupname, uint32_t groupID)
{   
    printf("Enter createGroup()\n");
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
            fprintf(stderr,"Group with name= %s already exists!\n", groupname);
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
    groups[index].usersID = new int[MAX_USERS]{};

    //set group and bitmap
    memcpy(groups[index].groupname, groupname, strlen(groupname));
    groups[index].groupID = groupID;
    groups[index].nrUsers = 0;
    bitmapGroups[groups[index].groupID] = true;
    totalGroups++;

    return true;
}

bool FileSystemAPI::deleteGroup(uint32_t groupID)
{
    printf("Enter deleteGroup()\n");
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
            bitmapGroups[i] = false;

            printf("Group with name= %s and ID= %d deleted!\n", groups[i].groupname, groups[i].groupID);

            return true;
        }
    }

    fprintf(stderr, "Group with ID= %d not found!\n", groupID);
    return false;
}

bool FileSystemAPI::setFilePermissions(const char *filename, uint32_t permissions)
{
    size_t inumber = myFileSystem->getInumber(filename);

    if(inumber == -1)
        return false;

    Inode inode = myFileSystem->getInode(inumber);

    //only the owner can change the permissions
    if(users[currentUser].userID == inode.OwnerUserID){
        inode.Permissions = permissions;
        return true;
    }

    fprintf(stderr, "User with id= %d doesn't have permissions for file= %s \n", users[currentUser].userID, inode.Filename);
    return false;
}

uint32_t FileSystemAPI::getFilePermissions(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);
    Inode inode = myFileSystem->getInode(inumber);

    return inode.Permissions;
}

bool FileSystemAPI::mountFileSystem()
{
    return myFileSystem->mount(disk);
}

bool FileSystemAPI::unmountFileSystem()
{
    return myFileSystem->unmount(disk);
}

bool FileSystemAPI::formatFileSystem()
{
    return myFileSystem->format(disk);
}

ssize_t FileSystemAPI::createFile(const char *filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions)
{
    printf("Enter createFile()!\n");
    size_t ret;

    ret = myFileSystem->create(filename, ownerUserID, ownerGroupID, permissions);

    return ret;
}

bool FileSystemAPI::removeFile(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);

    if(inumber == -1)
        return false;

    return myFileSystem->remove(inumber);
}

statDetails FileSystemAPI::getFileStat(const char *filename)
{
    size_t inumber = myFileSystem->getInumber(filename);

    statDetails stats = myFileSystem->stat(inumber);

    return stats;
}

ssize_t FileSystemAPI::readFile(const char *filename, char *data, size_t length, size_t offset)
{
    printf("Enter readFile()\n");
    size_t inumber = myFileSystem->getInumber(filename);

    //file doesn't exist
    if(inumber == -1)
        return -1;

    size_t totalRead;
    Inode inode = myFileSystem->getInode(inumber);

    //read if has permissions
    if(hasPermissions(filename, READ_PERMISSION))
        totalRead = myFileSystem->fs_read(inumber, data, length, offset);

    printf("totalRead= %d and data is: %s", totalRead, data);

    return totalRead;
}

ssize_t FileSystemAPI::writeFile(const char *filename, const char *data, size_t length, size_t offset)
{
    printf("Enter writeFile()\n");
    size_t totalWrite = 0, inumber;
    Inode inode;

    inumber = myFileSystem->getInumber(filename);

    //create if doesn't exist
    if(inumber == -1){
        inumber = createFile(filename, users[currentUser].userID, users[currentUser].groupID, 0644);
    }

    //write if has permissions
    if(hasPermissions(filename, WRITE_PERMISSION))
        totalWrite = myFileSystem->fs_write(inumber, data, length, offset);

    return totalWrite;
}

void FileSystemAPI::readImportantFile(const char *filename)
{
    char *data, *token, *str, *internalToken;
    int index = 0;
    size_t inumber;

    data = new char[Disk::BLOCK_SIZE + 1]{};

    //find inumber and read
    inumber = myFileSystem->getInumber(filename);
    myFileSystem->fs_read(inumber, data, Disk::BLOCK_SIZE, 0);

    //check if is populate
    if(data[0] == '\0'){
        fprintf(stderr, "\tFisierul %s nu este populat!\n", filename);
        return;
    }

    //allocate for strtok in another string where I have permissions
    str = new char[Disk::BLOCK_SIZE + 1]{};
    memcpy(str, data, Disk::BLOCK_SIZE);

    //see what file is
    seeTypeImportantFile(filename);

    //choose how to tokenize
    if(!isGroupsFile)
        token = strtok(str, ":");
    else
        token = strtok_r(str, "\n", &internalToken);

    while(token != NULL){

        //call specific function
        if(isUsersFile)
            readUsersFile(token, index);

        else if(isPasswordsFile)
            readPassswordsFile(token, index);

        else if(isGroupsFile)
            readGroupsFile(token, index);

        //go next
        if(!isGroupsFile)
            token = strtok(NULL, ":");
        else
            token = strtok_r(NULL, "\n", &internalToken);

        index ++;
    }

    //set total
    if(isUsersFile)
        totalUsers = index;

    else if(isGroupsFile)
        totalGroups = index;

    //for(int i = 0; i < totalUsers; i ++){
        //printf("S-a citit userul de la index= %d: %s %s %d %d %d\n", i,users[i].username,users[i].password, users[i].userID, users[i].groupID, users[i].permissions);
    //}

    for(int i = 0; i < totalGroups; i ++){
        //printf("S-a citit grupul de la index= %d: %s %d\n", i, groups[i].groupname, groups[i].groupID);

        for(int j = 0; j < groups[i].nrUsers; j ++){
            //printf(":%d", groups[i].usersID[j]);
        }
        printf("\n");
    }

    delete data;

    //printf("Exit readUsers ok!\n");
}

void FileSystemAPI::readUsersFile(const char *token, int index)
{
    //read username 
    users[index].username = new char[strlen(token) + 1]{};
    memcpy(users[index].username, token, strlen(token));
    token = strtok(NULL, ":");

    //ignore password
    token = strtok(NULL, ":");

    //take userID
    users[index].userID = atoi(token);
    token = strtok(NULL, ":");

    //take groupID
    users[index].groupID = atoi(token);
    token = strtok(NULL, "\n");

    //take permissions
    users[index].permissions = atoi(token);
    
    //set bitmap for users
    bitmapUsers[users[index].userID] = true;
}

void FileSystemAPI::readPassswordsFile(const char *token, int index)
{
    //read username
    token = strtok(NULL, "\n");

    //read password
    users[index].password = new char[strlen(token) + 1]{};
    memcpy(users[index].password, token, strlen(token));
}

void FileSystemAPI::readGroupsFile(const char *token, int index)
{
    printf("Enter readGroupsFile!\n");
    char *str2, *token2;
    bool hasUsers = false;
    int countChar = 0;

    //search for users
    for(int i = 0; i < strlen(token); i ++){
        if(token[i] == ':')
            countChar++;
    }

    if(countChar > 2)
        hasUsers = true;

    //alloc users in group
    groups[index].usersID = new int[MAX_USERS]{};

    //create a copy
    str2 = new char[strlen(token) + 1]{};
    memcpy(str2, token, strlen(token));

    printf("str2= %s\n", str2);

    token2 = strtok(str2, ":\n");

    //read groupname
    printf("groupname= %s\n", token2);
    groups[index].groupname = new char[strlen(token2) + 1]{};
    memcpy(groups[index].groupname, token2, strlen(token2));
    token2 = strtok(NULL, ":\n");

    //skip after x
    token2 = strtok(NULL, ":\n");

    //take groupID
    printf("groupID= %s\n", token2);
    groups[index].groupID = atoi(token2);

    //set bitmap for group
    bitmapGroups[groups[index].groupID] = true;

    //check if has users
    if(!hasUsers){
        printf("Has no users!\n");
        return;
    }
    
    token2 = strtok(NULL, ":\n");

    //read user by user
    while(token2 != NULL){
        groups[index].usersID[groups[index].nrUsers] = atoi(token2);
        groups[index].nrUsers ++;

        token2 = strtok(NULL, ":\n");
    }

    delete str2;
}

void FileSystemAPI::writeImportantFile(const char *filename)
{
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

    delete data;
}

void FileSystemAPI::writeUsersFile(char *line, size_t length, int i)
{
    if(users[i].userID != 0)
        snprintf(line, length,"%s:x:%d:%d:%d\n", users[i].username, users[i].userID, users[i].groupID, users[i].permissions);
    else
        line[0] = '\0';
}

void FileSystemAPI::writePasswordsFile(char *line, size_t length, int i)
{
    if(users[i].userID != 0)
        snprintf(line, length, "%s:%s\n", users[i].username, users[i].password);
    else
        line[0] = '\0';    
}

void FileSystemAPI::writeGroupsFile(char *line, size_t length, int i)
{   
    int totalWritten = 0;

    //read name and groupID
    int written = snprintf(line, length, "%s:x:%d", groups[i].groupname, groups[i].groupID);
    totalWritten += written;

    //read every user from that group
    for(int j = 0; j < groups[i].nrUsers; j ++){
        if(groups[i].usersID[j] != 0){
            written = snprintf(line + totalWritten, length - totalWritten, ":%d", groups[i].usersID[j]);
            totalWritten += written;
        }
    }

    snprintf(line + totalWritten, length - totalWritten, "\n");
}

void FileSystemAPI::seeTypeImportantFile(const char *filename)
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

bool FileSystemAPI::setCurrentUser(uint32_t userID)
{
    for(int i = 0; i < totalUsers; i ++){
        if(users[i].userID == userID){
            currentUser = i;
            return true;
        }
    }

    fprintf(stderr, "User with ID= %d doesn't exist!\n");
    return false;
}

uint32_t FileSystemAPI::setGroupID()
{
    for(int i = 0; i < totalGroups; i ++){
        if(!bitmapGroups)
            return i;
    }

    return 0;
}

bool FileSystemAPI::checkCredentials(const char *username, const char *password)
{
    for(int i = 0; i < totalUsers; i ++){

        //check for username
        if(strncmp(users[i].username, username, (strlen(users[i].username) + 1)) == 0){
            
            //check for password
            if(strncmp(users[i].password, password, (strlen(users[i].password) + 1)) == 0)
                return true;
            else{
                fprintf(stderr, "Password not match!\n");
                return false;
            }
        }
    }

    fprintf(stderr, "User= %s doesn't exist!\n", username);
    return false;
}

uint32_t FileSystemAPI::setUserID()
{   
    for(int i = 2; i < MAX_USERS; i ++){
        if(!bitmapUsers[i]){
            return i;
        }
    }

    return 0;
}
