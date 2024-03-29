#include "../../includes/shellProgram.h"

ShellProgram *ShellProgram::instance = nullptr;
FileSystemAPI *ShellProgram::myAPI = nullptr;
char *ShellProgram::username = nullptr;
char *ShellProgram::password = nullptr;

uint32_t ShellProgram::userID = 0;
uint32_t ShellProgram::groupID = 0;

bool ShellProgram::executeFlag = false;
bool ShellProgram::exitFlag = false;
bool ShellProgram::returnFlag = false;

ShellProgram::ShellProgram(Disk *disk, size_t blocks)
{
    //initialise variables
    username = new char[LENGTH]{};
    password = new char[LENGTH]{};

    //set API
    myAPI = FileSystemAPI::getInstance(disk, blocks);
}

ShellProgram::~ShellProgram()
{
    //destroy instance for File System
    FileSystemAPI::destroyInstance();
}

ShellProgram *ShellProgram::getInstance(Disk *disk, size_t blocks)
{   
    //create instance
    if(!instance)
        instance = new ShellProgram(disk, blocks);

    return instance;
}

void ShellProgram::destroyInstance()
{   
    //destroy instance
    if(instance){
        delete instance;
        instance = nullptr;
    }
}

void ShellProgram::createAccount()
{
    char *confirmPassword = new char[LENGTH]{};

    //create an account
    printf("> Create an account!\n");

    //enter username
    printf("> Create username: ");
    if(fgets(username, LENGTH, stdin) != NULL){
        username[strcspn(username, "\n")] = '\0';
    }

    //enter password
    printf("> Create password: ");
    readPassword(confirmPassword);

    memcpy(password, confirmPassword, strlen(confirmPassword) + 1);
    memset(confirmPassword, '\0', strlen(confirmPassword));

    //rewrite
    printf("> Confirm password: ");
    readPassword(confirmPassword);

    //printf("password= %s confirmpassword= %s \n", password, confirmPassword);

    //check length
    if(strlen(username) < 3){
        fprintf(stderr, "Username= %s too short!\n", username);
        return;
    }
    else if(strlen(password) < 3){
        fprintf(stderr,"Password too short!\n");
        return;
    }

    //find free userID to set for the new user and create it
    if(strncmp(password, confirmPassword, LENGTH) == 0){
        userID = myAPI->setUserID();

        //check if can create
        if(myAPI->createUser(username, password, userID) == false)
            return;
    
        myAPI->setCurrentUser(userID);
        executeFlag = true;
    }
    else
        fprintf(stderr, "Parolele nu sunt identice!\n");

    delete[] confirmPassword;
}

bool ShellProgram::login()
{
    //enter username
    printf("> Enter username: ");
    if(fgets(username, LENGTH, stdin) != NULL){
        username[strcspn(username, "\n")] = '\0';
    }

    //enter password
    printf("> Enter password: ");
    readPassword(password);
    //printf("pass at login: %s\n", password);

    //check credentials and set current user if exists
    if(checkCredentials()){
        userID = myAPI->getUserID(username);
        myAPI->setCurrentUser(userID);
        groupID = myAPI->getCurrentGroupID();
        executeFlag = true;
        return true;
    }

    return false;
}

bool ShellProgram::checkCredentials()
{
    return myAPI->checkCredentials(username, password);
}

bool ShellProgram::checkRootPrivilege()
{
    //initialise password to enter
    char *passwordRoot = new char[PASSWORD_LENGTH]{};
    
    //read password
    printf("[sudo] password for %s: ", username);
    readPassword(passwordRoot);
    
    //check if has root's password
    if(strncmp(ROOT_PASSWORD, passwordRoot, (strlen(passwordRoot) + 1)) == 0)
        return true;

    printf("User= %s doesn't have root privilege!\n", username);
    return false;
}

bool ShellProgram::checkCommand(const char *shellCommand, const char *userCommand)
{
    if(strncmp(shellCommand, userCommand, (strlen(userCommand) + 1)) == 0)
        return true;

    return false;
}

bool ShellProgram::checkParameters(const char *line)
{
    //declare variables
    bool noParameters = false, oneParameter = false, twoParameters = false, flag = false;
    char *command, *str, *token;

    //create another token to check parameters
    str = new char[strlen(line) + 1]{};
    memcpy(str, line, (strlen(line) + 1));

    //this should be the command
    token = strtok(str, " \n");
    command = new char[strlen(token) + 1]{};
    memcpy(command, token, (strlen(token) + 1));

    //this should be the first parameter
    token = strtok(NULL, " \n");
    if(token == NULL)
        noParameters = true;
    
    // has no parameters like the command
    if (noParameters && checkNoParamCommand(command))
        flag = true;

    // check if has one parameter
    if (token != NULL){
        token = strtok(NULL, " \n");
        if (token == NULL)
            oneParameter = true;
    }

    // has one parameter like the command
    if (oneParameter && checkOneParamCommand(command))
        flag = true; 

    // check for the second parameter
    if (token != NULL){
        token = strtok(NULL, "\n");
        if (token == NULL)
            twoParameters = true;
    }

    // has two parameters like the command
    if(twoParameters && checkTwoParamCommand(command))
        flag = true;
        
    delete[] command;
    delete[] str;

    if (flag)
        return true;

    fprintf(stderr, "Invalid command!\n");
    return false;
}

bool ShellProgram::checkNoParamCommand(const char *command)
{
    bool flag = false;

    if  (checkCommand(SHOW_GROUPS_COMMAND, command)|| checkCommand(SHOW_USERS_COMMAND, command))
        flag = true;

    else if (checkCommand(FORMAT_COMMAND, command) || checkCommand(MOUNT_COMMAND, command))
        flag = true;

    else if (checkCommand(ALL_FILES_COMMAND, command) || checkCommand(UMNOUNT_COMMAND, command))
        flag = true;

    else if (checkCommand(EXIT_COMMAND, command) || checkCommand(RETURN_COMMAND, command))
        flag = true;

    else if (checkCommand(INSTRUCTIONS_COMMAND, command))
        flag = true;

    if (flag)
        return true;

    return false;
}

bool ShellProgram::checkOneParamCommand(const char *command)
{
    bool flag = false;

    if (checkCommand(READ_FILE_COMMAND, command) || checkCommand(ONE_FILE_COMMAND, command))
        flag = true;
    
    else if (checkCommand(GROUP_ADD_COMMAND, command) || checkCommand(SET_GROUP_COMMAND, command))
        flag = true;

    else if (checkCommand(DELETE_USER_COMMAND, command) || checkCommand(DELETE_GROUP_COMMAND, command))
        flag = true;

    else if (checkCommand(CHANGE_USER_PERMISSIONS_COMMAND, command) || checkCommand(CHANGE_GROUP_PERMISSIONS_COMMAND, command))
        flag = true;

    else if (checkCommand(CREATE_FILE_COMMAND, command) || checkCommand(DELETE_FILE_COMMAND, command))
        flag = true;

    if (flag)
        return true;

    return false;
}

bool ShellProgram::checkTwoParamCommand(const char *command)
{
    bool flag = false;

    if (checkCommand(COPY_IN_COMMAND, command) || checkCommand(COPY_OUT_COMMAND, command))
        flag = true;

    else if (checkCommand(COPY_COMMAND, command) || checkCommand(CHG_PERM_FILE_COMMAND, command))
        flag = true;

    if (flag)
        return true;

    return false;
}

void ShellProgram::prepareCommands()
{
    char command[COMMAND_LENGTH];
    returnFlag = false;

    while(!exitFlag && !returnFlag){
        
        //initialise command
        memset(command, '\0', COMMAND_LENGTH);

        //write commands
        printf("%s@%s>", username, DOMAIN_NAME);

        if(fgets(command, sizeof(command), stdin) != NULL){
            command[strcspn(command,"\n")] = '\0';
        }
        
        executeCommands(command);
    }
}

void ShellProgram::executeCommands(char *line)
{
    if(strlen(line) == 0){
        fprintf(stderr, "Incorrect command!\n");
        return;
    }

    //check if has parameters where are required
    if(!checkParameters(line))
        return;

    //initialise command and parameters
    char *parameters = strtok(line, " \n");
    char *command = new char[strlen(parameters) + 1]{};

    //set command and parameters
    memcpy(command, parameters, strlen(parameters) + 1);
    parameters = strtok(NULL, " \n");

    //select command
    CommandType tmp = selectCommand(command);

    switch(tmp){
        
        //add group
        case CommandType::GroupAddCommand:
            grpAddCommand(parameters);
            break;

        //setgroup
        case CommandType::SetGroupCommand:
            setGrpCommand(parameters);
            break;

        //delete user
        case CommandType::DeleteUserCommand:
            delUsrCommand(parameters);
            break;

        //delete group
        case CommandType::DeleteGroupCommand:
            delGrpCommand(parameters);
            break;

        //show users
        case CommandType::ShowUsersCommand:
            showUsrCommand(parameters);
            break;

        //show groups
        case CommandType::ShowGroupsCommand:
            showGrpCommand(parameters);
            break;

        //change user permissions
        case CommandType::ChangeUserPermissionsCommand:
            chUsrPermCommand(parameters);
            break;
        
        //change group permissions
        case CommandType::ChangeGroupPermissionsCommand:
            chGrpPermCommand(parameters);
            break;

        //create file
        case CommandType::CreateFileCommand:
            createFileCommand(parameters);
            break;

        //copy a file from disk on emulator
        case CommandType::CopyInCommand:
            copyInCommand(parameters);
            break;

        //copy a file from emulator to disk
        case CommandType::CopyOutCommand:
            copyOutCommand(parameters);
            break;

        //copy from emulator to emulator
        case CommandType::CopyCommand:
            copyCommand(parameters);
            break;

        //read file
        case CommandType::ReadFileCommand:
            readFileCommand(parameters);
            break;

        //delete file
        case CommandType::DeleteFileCommand:
            delFileCommand(parameters);
            break;

        //format disk
        case CommandType::FormatCommand:
            formatCommand();
            break;

        //mount disk
        case CommandType::MountCommand:
            mountCommand();
            break;

        //unmount disk
        case CommandType::UnmountCommand:
            unmountCommand();
            break;

        //show all files stats
        case CommandType::AllFilesCommand:
            showAllFilesCommand();
            break;

        //show one file stats
        case CommandType::OneFileCommand:
            showOneFileCommand(parameters);
            break;

        //change file permissionss
        case CommandType::ChgFilePermCommand:
            chgFilePermCommand(parameters);
            break;

        // exit
        case CommandType::ExitCommand:
            exitFlag = true;
            break;

        // return
        case CommandType::ReturnCommand:
            returnFlag = true;
            break;

        // help
        case CommandType::InstructionsCommand:
            showInstructions();
            break;

        //incorrect command
        case CommandType::IncorrectCommand:
            printf("Incorrect command!\n");
            break;
    }
}

void ShellProgram::run()
{
    char option;

    //login and then execute commands
    while(!exitFlag){
        printf("> ");

        printf("Do you have an account?[Y/N] ");

        option = getchar();

        fflushInputBuffer();

        if(option == 'Y' || option == 'y')
            login();

        else if (option == 'N' || option == 'n')
            createAccount();

        if(executeFlag)
            prepareCommands();

        memset(username, '\0', LENGTH);
        memset(password, '\0', LENGTH);
        executeFlag = false;
    }
}

CommandType ShellProgram::selectCommand(const char *command)
{
    //add group
    if(checkCommand(GROUP_ADD_COMMAND, command)){
        return CommandType::GroupAddCommand;
    }

    //setgroup
    else if(checkCommand(SET_GROUP_COMMAND, command)){
        return CommandType::SetGroupCommand;
    }

    //delete user
    else if(checkCommand(DELETE_USER_COMMAND, command)){
        return CommandType::DeleteUserCommand;
    }

    //delete group
    else if(checkCommand(DELETE_GROUP_COMMAND, command)){
        return CommandType::DeleteGroupCommand;
    }

    //show users
    else if(checkCommand(SHOW_USERS_COMMAND, command)){
        return CommandType::ShowUsersCommand;
    }

    //show groups
    else if(checkCommand(SHOW_GROUPS_COMMAND, command)){
        return CommandType::ShowGroupsCommand;
    }

    //change user permissions
    else if(checkCommand(CHANGE_USER_PERMISSIONS_COMMAND, command)){
        return CommandType::ChangeUserPermissionsCommand;
    }

    //change group permissions
    else if(checkCommand(CHANGE_GROUP_PERMISSIONS_COMMAND, command)){
        return CommandType::ChangeGroupPermissionsCommand;
    }

    //create file
    else if(checkCommand(CREATE_FILE_COMMAND, command))
        return CommandType::CreateFileCommand;
    
    //copy a file from disk on emulator
    else if(checkCommand(COPY_IN_COMMAND, command))
        return CommandType::CopyInCommand;

    //copy a file from emulator to disk
    else if(checkCommand(COPY_OUT_COMMAND, command))
        return CommandType::CopyOutCommand;

    //copy from emulator to emulator
    else if(checkCommand(COPY_COMMAND, command))
        return CommandType::CopyCommand;

    //read file
    else if(checkCommand(READ_FILE_COMMAND, command))
        return CommandType::ReadFileCommand;

    //delete file
    else if(checkCommand(DELETE_FILE_COMMAND, command))
        return CommandType::DeleteFileCommand;

    //format disk
    else if(checkCommand(FORMAT_COMMAND, command))
        return CommandType::FormatCommand;

    //mount disk
    else if(checkCommand(MOUNT_COMMAND, command))
        return CommandType::MountCommand;

    //unmount disk
    else if(checkCommand(UMNOUNT_COMMAND, command))
        return CommandType::UnmountCommand;

    //show all files stats
    else if(checkCommand(ALL_FILES_COMMAND, command))
        return CommandType::AllFilesCommand;

    //show one file stats
    else if(checkCommand(ONE_FILE_COMMAND, command))
        return CommandType::OneFileCommand;

    //change file permissions
    else if(checkCommand(CHG_PERM_FILE_COMMAND, command))
        return CommandType::ChgFilePermCommand;

    // exit
    else if (checkCommand(EXIT_COMMAND, command))
        return CommandType::ExitCommand;

    // return 
    else if (checkCommand(RETURN_COMMAND, command))
        return CommandType::ReturnCommand;

    // help
    else if (checkCommand(INSTRUCTIONS_COMMAND, command))
        return CommandType::InstructionsCommand;

    else
        return CommandType::IncorrectCommand;
}

void ShellProgram::grpAddCommand(char *parameters)
{
    size_t groupID = myAPI->setGroupID();
    myAPI->createGroup(parameters, groupID);
}

void ShellProgram::setGrpCommand(char *parameters)
{
    groupID = myAPI->getGroupID(parameters);
    myAPI->setUserGroup(userID, groupID);
}

void ShellProgram::chUsrPermCommand(char *parameters)
{
    if(checkRootPrivilege()){
        printf("parameters: %s\n", parameters);
        char *convertPointer;
        uint32_t permissions;

        permissions = strtol(parameters, &convertPointer, 8);

        if( (*convertPointer) != '\0'){
            fprintf(stderr, "Failed to convert in octal!\n");
            return;
        }

        if (permissions > 7 || permissions < 0){
            fprintf(stderr, "Wrong permissions given!\n");
            return;
        }

        myAPI->changeUserPermissions(userID, permissions);
    }
}

void ShellProgram::chGrpPermCommand(char *parameters)
{
    if(checkRootPrivilege()){
        printf("parameters: %s\n", parameters);
        char *convertPointer;
        uint32_t permissions;

        permissions = strtol(parameters, &convertPointer, 8);

        if( (*convertPointer) != '\0'){
            fprintf(stderr, "Failed to convert in octal!\n");
            return;
        }

        if (permissions > 7 || permissions < 0){
            fprintf(stderr, "Wrong permissions given!\n");
            return;
        }

        myAPI->changeGroupPermissions(groupID, permissions);
    }
}

void ShellProgram::delUsrCommand(char *parameters)
{
    if(checkRootPrivilege()){
        size_t userDeleteID = myAPI->getUserID(parameters);
        myAPI->deleteUser(userDeleteID);
    }
}

void ShellProgram::delGrpCommand(char *parameters)
{
    if(checkRootPrivilege()){
        size_t groupDeleteID = myAPI->getGroupID(parameters);
        myAPI->deleteGroup(groupDeleteID);
    }
}

void ShellProgram::showUsrCommand(char *parameters)
{
    myAPI->showUsers();
}

void ShellProgram::showGrpCommand(char *parameters)
{
    myAPI->showGroups();
}

void ShellProgram::createFileCommand(char *parameters)
{
    myAPI->createFile(parameters, userID, groupID, FILE_PERMISSIONS);
}

void ShellProgram::copyInCommand(char *parameters)
{
    //declare and initialise src and dest
    size_t bytesRead, bytesToRead = 4096, offset = 0;
    char *src, *dst;
    char data[Disk::BLOCK_SIZE + 1];

    //set source
    src = new char[strlen(parameters) + 1]{};
    memcpy(src, parameters, strlen(parameters) + 1);

    parameters = strtok(NULL, " \n");
    
    if(parameters == NULL){
        fprintf(stderr, "Incorrect parameters!\n");
        delete[] src;
        return;
    }

    //set destination
    dst = new char[strlen(parameters) + 1]{};
    memcpy(dst, parameters, strlen(parameters) + 1);
    
    FILE *f = fopen(src, "r");

    if(f == NULL){
        fprintf(stderr, "Error on opening file= %s\n", src);
        return;
    }

    //check permissions
    if( !myAPI->hasPermissions(dst, WRITE_PERMISSION)){
        fprintf(stderr, "User= %s doesn't have permissions for file= %s to write\n", username, dst);
        delete[] src;
        delete[] dst;
        return;
    }

    //read all from file on disk and write it on emulator
    while(!ferror(f) && !feof(f)){
        
        memset(data, '\0', Disk::BLOCK_SIZE + 1);

        bytesRead = fread(data, sizeof(char), bytesToRead, f);
        myAPI->writeFile(dst, data, bytesRead, offset);

        offset += bytesRead;
    }

    fclose(f);

    delete[] src;
    delete[] dst;
}

void ShellProgram::copyOutCommand(char *parameters)
{
    //declare and initialise variables
    size_t byteRead, bytesToRead = 4096, offset = 0, size; 
    char *src, *dst;
    char data[Disk::BLOCK_SIZE + 1];

    //set source
    src = new char[strlen(parameters) + 1]{};
    memcpy(src, parameters, strlen(parameters) + 1);

    parameters = strtok(NULL, " \n");
    
    //check has destination
    if(parameters == NULL){
        fprintf(stderr, "Incorrect parameters!\n");
        delete[] src;
        return;
    }

    //check permissions
    if(!myAPI->hasPermissions(src, READ_PERMISSION)){
        fprintf(stderr, "User= %s doesn't have permissions for file= %s to read\n", username, src);
        delete[] src;
        return;
    }

    //set destination
    dst = new char[strlen(parameters) + 1]{};
    memcpy(dst, parameters, strlen(parameters) + 1);
    
    //get size
    size = myAPI->getSizeInode(src);
    if(size == 0)
        return;

    //open to write in
    FILE *f = fopen(dst, "w");
    if(f == NULL){
        fprintf(stderr, "Error on opening file= %s\n", dst);
        return;
    }

    while(size > offset){

        memset(data, '\0', Disk::BLOCK_SIZE + 1);

        byteRead = myAPI->readFile(src, data, bytesToRead, offset);
        fprintf(f, "%s", data);

        offset += byteRead;
    }

    fclose(f);

    delete[] src;
    delete[] dst;

}

void ShellProgram::copyCommand(char *parameters)
{
    //declare and initialise variables
    size_t byteRead, bytesToRead = 4096, offset = 0, size; 
    char *src, *dst;
    char data[Disk::BLOCK_SIZE + 1];

    //set source
    src = new char[strlen(parameters) + 1]{};
    memcpy(src, parameters, strlen(parameters) + 1);

    parameters = strtok(NULL, " \n");
    
    if(parameters == NULL){
        fprintf(stderr, "Incorrect parameters!\n");
        delete[] src;
        return;
    }

    //set destination
    dst = new char[strlen(parameters) + 1]{};
    memcpy(dst, parameters, strlen(parameters) + 1);

    //check permissions
    if(!myAPI->hasPermissions(src, READ_PERMISSION) && !myAPI->hasPermissions(dst, WRITE_PERMISSION)){
        fprintf(stderr, "User= %s doesn't have permissions top copy these files!\n", username);
        delete[] src;
        delete[] dst;
        return;
    }

    //get size
    size = myAPI->getSizeInode(src);
    if(size == 0)
        return;

    //copy from a inode to another
    while(size > offset){

        memset(data, '\0', Disk::BLOCK_SIZE + 1);

        byteRead = myAPI->readFile(src, data, byteRead, offset);
        byteRead = myAPI->writeFile(dst, data, bytesToRead, offset);

        offset += byteRead;
    }

    delete[] src;
    delete[] dst;
}

void ShellProgram::readFileCommand(char *parameters)
{
    //declare variables
    char data[Disk::BLOCK_SIZE + 1];
    size_t bytesRead, bytesToRead = 4096, size, offsset = 0;

    if(strncmp(PASSWORDS_FILE, parameters, (strlen(parameters) + 1)) == 0){
        if(!checkRootPrivilege())
            return;
    }

    //check permissions
    if(!myAPI->hasPermissions(parameters, READ_PERMISSION)){
        fprintf(stderr, "User= %s doesn't have permissions for file= %s to read\n", username, parameters);
        return;
    }

    //set size
    size = myAPI->getSizeInode(parameters);
    if(size == 0)
        return;

    //while size is greater than offset
    while(size > offsset){
        memset(data, '\0', Disk::BLOCK_SIZE + 1);

        bytesRead = myAPI->readFile(parameters, data, bytesToRead, offsset);

        offsset += bytesRead;

        printf("Data read= %s\n", data);
    }
}

void ShellProgram::delFileCommand(char *paramters)
{
    myAPI->removeFile(paramters);
}

void ShellProgram::formatCommand()
{
    if(checkRootPrivilege()){
        myAPI->formatFileSystem();
    }
}

void ShellProgram::mountCommand()
{
    if(checkRootPrivilege()){
        myAPI->mountFileSystem();
    }
}

void ShellProgram::unmountCommand()
{
    if(checkRootPrivilege()){
        myAPI->unmountFileSystem();
    }
}

void ShellProgram::showAllFilesCommand()
{
    myAPI->showFiles();
}

void ShellProgram::showOneFileCommand(char *parameters)
{
    statDetails fileStats = myAPI->getFileStat(parameters);

    if(!fileStats.Valid)
        return;

    printf("%s\t%d\t%d\t%d\t%o\n", fileStats.Filename, fileStats.OwnerUserID, fileStats.OwnerGroupID, fileStats.Size, fileStats.Permissions);
}

void ShellProgram::chgFilePermCommand(char *parameters)
{
    uint32_t permissions;
    char *filename;
    char *convertPointer;

    filename = new char[strlen(parameters) + 1]{};

    memcpy(filename, parameters, (strlen(parameters) + 1));

    parameters = strtok(NULL, " \n");

    if(parameters == NULL){
        fprintf(stderr, "Incorrect parameters!\n");
        delete[] filename;
        return;
    }

    permissions = strtol(parameters, &convertPointer, 8);

    if( (*convertPointer) != '\0'){
        fprintf(stderr, "Failed to convert in octal!\n");
        delete[] filename;
        return;
    }

    if(permissions > 0777 | permissions < 0){
        fprintf(stderr, "Wrong permissions given!\n");
        delete[] filename;
        return;
    }

    if(checkRootPrivilege())
        myAPI->setFilePermissions(filename, permissions);

    delete[] filename;
}

void ShellProgram::turnOffEcho()
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void ShellProgram::turnOnEcho() 
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void ShellProgram::readPassword(char *pswd) 
{
    turnOffEcho(); // Turn off echo while reading password
    
    if(fgets(pswd, LENGTH, stdin) != NULL){
        pswd[strcspn(pswd, "\n")] = '\0';
    }

    turnOnEcho(); // Turn on echo after reading password

    printf("\n");
}

void ShellProgram::fflushInputBuffer()
{
    int c;
    //printf("chars in buffer=");
    while((c = getchar()) != '\n' && c != EOF){
        //printf(" %c", c);
    }
    //printf("\n");
}

void ShellProgram::showInstructions()
{
    printf("\nInstructions for commands:\n");

    printf("%s\t-\tshow instruction\n", INSTRUCTIONS_COMMAND);
    printf("%s\t-\tshow all users\n", SHOW_USERS_COMMAND);
    printf("%s\t-\tshow all groups\n", SHOW_GROUPS_COMMAND);
    printf("%s\t-\tshow stats of all files\n", ALL_FILES_COMMAND);
    printf("%s\t-\tformat File System\n", FORMAT_COMMAND);
    printf("%s\t-\tmount File System\n", MOUNT_COMMAND);
    printf("%s\t-\tunmount File System\n", UMNOUNT_COMMAND);
    printf("%s\t-\treturn to start menu\n", RETURN_COMMAND);
    printf("%s\t-\texit from application\n", EXIT_COMMAND);

    printf("%s <filename>\t-\tprint file\n", READ_FILE_COMMAND);
    printf("%s <filename>\t-\tshow file's stats\n", ONE_FILE_COMMAND);
    printf("%s <groupname>\t-\tcreate group\n%s <groupname>\t-\tset group for current user\n", GROUP_ADD_COMMAND, SET_GROUP_COMMAND);
    printf("%s <username>\t-\tdelete user (require root privilege)\n", DELETE_USER_COMMAND);
    printf("%s <groupname>\t-\tdelete group (require root privilege)\n", DELETE_GROUP_COMMAND);
    printf("%s <permissions>\t-\tchange permissions for current user\n", CHANGE_USER_PERMISSIONS_COMMAND);
    printf("%s <permissions>\t-\tchange permissions for current user's group\n", CHANGE_GROUP_PERMISSIONS_COMMAND);
    printf("%s <filename>\t-\tcreate file\n", CREATE_FILE_COMMAND);
    printf("%s <filename>\t-\tdelete file\n", DELETE_FILE_COMMAND);
    
    printf("%s <filename> <permissions>\t-\tchange file's permissions (require root privilege)\n", CHG_PERM_FILE_COMMAND);
    printf("%s <src> <dest>\t-\tcopy from disk to emulator\n", COPY_IN_COMMAND);
    printf("%s <src> <dest>\t-\tcopy from emulator to disk\n", COPY_OUT_COMMAND);
    printf("%s <src> <dest>\t-\tcopy from emulator to emulator\n", COPY_COMMAND);
}
