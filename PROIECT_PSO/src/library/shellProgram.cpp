#include "../../includes/shellProgram.h"
#include "shellProgram.h"

ShellProgram *ShellProgram::instance = nullptr;
FileSystemAPI *ShellProgram::myAPI = nullptr;
char *ShellProgram::username = nullptr;
char *ShellProgram::password = nullptr;

uint32_t ShellProgram::userID = 0;
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
    myAPI->createUser("root", "seful", 1);
    myAPI->createGroup("root", 1);
    myAPI->setUserGroup(1, 1);
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
    
    //check if is root's password
    if(strncmp(ROOT_PASSWORD, passwordRoot, (strlen(passwordRoot) + 1)) == 0)
        return true;

    printf("User= %s doesn't have root privilege!\n", username);
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
        if(strncmp(EXIT_COMMAND, command, strlen(EXIT_COMMAND)) == 0)
            exitFlag = true;
        
        else if(strncmp(RETURN_COMMAND, command, strlen(RETURN_COMMAND)) == 0)
            returnFlag = true;
        
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
    if(strncmp(GROUPADD_COMMAND, command, (strlen(command) + 1)) == 0){
        size_t groupID = myAPI->setGroupID();
        myAPI->createGroup(parameters, groupID);
    }

    else if(strncmp(SETGROUP_COMMAND, command, (strlen(command) + 1)) == 0){
        size_t groupID = atoi(parameters);
        myAPI->setUserGroup(userID, groupID);
    }

    else if(strncmp(DELETEUSER_COMMAND, command, (strlen(command) + 1)) == 0){
        
        if(checkRootPrivilege()){
            size_t userDeleteID = myAPI->getUserID(parameters);
            myAPI->deleteUser(userDeleteID);
        }
    }

    else if(strncmp(DELETEGROUP_COMMAND, command, (strlen(command)  + 1)) == 0){

        if(checkRootPrivilege()){
            size_t groupDeleteID = myAPI->getGroupID(parameters);
            myAPI->deleteGroup(groupDeleteID);
        }
    }

    else{
        printf("Incorrect command!\n");
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