#include "../../includes/shellProgram.h"

ShellProgram *ShellProgram::instance = nullptr;
FileSystemAPI *ShellProgram::myAPI = nullptr;
bool ShellProgram::canExecute = false;

ShellProgram::ShellProgram(Disk *disk, size_t blocks)
{
    //initialise API and create root
    myAPI = FileSystemAPI::getInstance(disk, blocks);
    myAPI->createUser("root", "Adsajpvw!!13.", 1);
}

ShellProgram::~ShellProgram()
{
    FileSystemAPI::destroyInstance();
}

ShellProgram *ShellProgram::getInstance(Disk *disk, size_t blocks)
{
    if(!instance)
        instance = new ShellProgram(disk, blocks);

    return instance;
}

void ShellProgram::destroyInstance()
{
    if(instance){
        delete instance;
        instance = nullptr;
    }
}

void ShellProgram::createAccount()
{
    char *confirmPassword = new char[LENGTH]{};

    printf("\n> Create an account: \n\n");
    
    printf("> Enter username: ");
    fgets(username, LENGTH, stdin);

    printf("> Enter password: ");
    fgets(password, LENGTH, stdin);

    printf("> Confirm password: ");
    fgets(confirmPassword, LENGTH, stdin);

    if(strncmp(password, confirmPassword, LENGTH) == 0){
        myAPI->createUser(username, password, 1);
        myAPI->setCurrentUser(1);
        canExecute = true;
    }
    else
        fprintf(stderr, "Parolele nu sunt identice!\n");

    delete[] confirmPassword;
}

bool ShellProgram::login()
{

}

void ShellProgram::executeCommands()
{

}

void ShellProgram::run()
{
    char command[COMMAND_LENGTH];
    char option;

    username = new char[LENGTH]{};
    password = new char[LENGTH]{};

    //login and then execute commands
    while(true){
        printf(">");

        printf("Do you have an account? [Y/N]");

        option = fgetc(stdin);

        if(option == 'Y' || option == 'y'){
            if(!login())
                createAccount();
        }

        else if (option == 'N' || option == 'n')
            createAccount();

        if(canExecute)
            executeCommands;
    }
}


