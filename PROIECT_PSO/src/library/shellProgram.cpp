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
    //initialise variables, API and create root
    username = new char[LENGTH]{};
    password = new char[LENGTH]{};

    //set API and root
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
    char *confirmPassword = new char[LENGTH + 1]{};

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

    memcpy(password, confirmPassword, sizeof(password));
    memset(confirmPassword, '\0', strlen(confirmPassword));

    //rewrite
    printf("> Confirm password: ");
    readPassword(confirmPassword);

    printf("password= %s confirmpassword= %s \n", password, confirmPassword);

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

        //see if exit, return or execute commands
        if(strncmp(EXIT_COMMAND, command, (strlen(command) + 1)) == 0)
            exitFlag = true;
        
        else if(strncmp(RETURN_COMMAND, command, (strlen(command) + 1)) == 0)
            returnFlag = true;

        else if(strncmp(INSTRUCTIONS_COMMAND, command, (strlen(command) + 1)) == 0)
            showInstructions();
        
        else 
            executeCommands(command);
    }
}

void ShellProgram::executeCommands(char *line)
{
    //initialise command and parameters
    char *parameters = strtok(line, " \n");
    char *command = new char[strlen(parameters) + 1]{};

    //set command and parameters
    memcpy(command, parameters, strlen(parameters) + 1);
    parameters = strtok(NULL, " \n");

    //select command
    CommandType tmp = selectCommand(command);

    switch(tmp){
        
        case CommandType::GroupAddCommand:
            grpAddCommand(parameters);
            break;

        case CommandType::SetGroupCommand:
            setGrpCommand(parameters);
            break;

        case CommandType::DeleteUserCommand:
            delUsrCommand(parameters);
            break;

        case CommandType::DeleteGroupCommand:
            delGrpCommand(parameters);
            break;

        case CommandType::ShowUsersCommand:
            showUsrCommand(parameters);
            break;

        case CommandType::ShowGroupsCommand:
            showGrpCommand(parameters);
            break;

        case CommandType::ChangeUserPermissionsCommand:
            chUsrPermCommand(parameters);
            break;
        
        case CommandType::ChangeGroupPermissionsCommand:
            chGrpPermCommand(parameters);
            break;

        case CommandType::CreateFileCommand:
            createFileCommand(parameters);
            break;

        case CommandType::CopyFileCommand:
            copyFileCommand(parameters);
            break;

        case CommandType::MoveFileCommand:
            moveFileCommand(parameters);
            break;

        case CommandType::ReadFileCommand:
            readFileCommand(parameters);
            break;

        case CommandType::DeleteFileCommand:
            delFileCommand(parameters);
            break;

        default:
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

void ShellProgram::turnOffEcho() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void ShellProgram::turnOnEcho() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void ShellProgram::readPassword(char *pswd) {
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
    printf("chars in buffer=");
    while((c = getchar()) != '\n' && c != EOF){
        printf(" %c", c);
    }
    printf("\n");
}

void ShellProgram::showInstructions()
{
    printf("\nInstructions for commands:\n");
    printf("%s\t-\tshow instruction\n", INSTRUCTIONS_COMMAND);
    printf("%s <groupname>\t-\tcreate group\n%s <groupname>\t-\tset group for current user\n", GROUP_ADD_COMMAND, SET_GROUP_COMMAND);
    printf("%s <username>\t-\tdeletet user (required root privilege)\n", DELETE_USER_COMMAND);
    printf("%s <groupname>\t-\tdelete group (required root privilege)\n", DELETE_GROUP_COMMAND);
    printf("%s <permissions>\t-\tchange permissions for current user\n", CHANGE_USER_PERMISSIONS_COMMAND);
    printf("%s <permissions>\t-\tchange permissions for current user group\n", CHANGE_GROUP_PERMISSIONS_COMMAND);
    printf("%s\t-\tshow all users\n", SHOW_USERS_COMMAND);
    printf("%s\t-\tshow all groups\n", SHOW_GROUPS_COMMAND);
    printf("%s\n%s\n", EXIT_COMMAND, RETURN_COMMAND);
    printf("%s <filename>\t-\tcreate file\n", CREATE_FILE_COMMAND);
    printf("%s <filename>\t-\tdelete file\n", DELETE_FILE_COMMAND);
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

    //copy file
    else if(checkCommand(COPY_FILE_COMMAND, command))
        return CommandType::CopyFileCommand;

    //move  file
    else if(checkCommand(MOVE_FILE_COMMAND, command))
        return CommandType::MoveFileCommand;

    //read file
    else if(checkCommand(READ_FILE_COMMAND, command))
        return CommandType::ReadFileCommand;

    //delete file
    else if(checkCommand(DELETE_FILE_COMMAND, command))
        return CommandType::DeleteFileCommand;

    

    return CommandType();
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
        size_t permissions = atoi(parameters);

        myAPI->changeUserPermissions(userID, permissions);
    }
}

void ShellProgram::chGrpPermCommand(char *parameters)
{
    if(checkRootPrivilege()){
        size_t permissions = atoi(parameters);
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

void ShellProgram::copyFileCommand(char *parameters)
{

}

void ShellProgram::moveFileCommand(char *parameters)
{

}

void ShellProgram::readFileCommand(char *parameters)
{
    //declare variables
    char *data;
    size_t maxLength = 4960;

    myAPI->readFile(parameters, data, maxLength, 0);


    printf("Data read= %s\n", data);

    delete[] data;
}

void ShellProgram::delFileCommand(char *paramters)
{
    myAPI->removeFile(paramters);
}
