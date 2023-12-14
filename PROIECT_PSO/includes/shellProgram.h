#include "fileSystemAPI.h"

#define COMMAND_LENGTH 20
#define LENGTH 20

#define DOMAIN_NAME "@Titanic"

class ShellProgram{
private:
    ShellProgram(Disk *disk, size_t blocks);
    ~ShellProgram();

    static ShellProgram *instance;
    static FileSystemAPI *myAPI;

    static bool canExecute;

    static char *username;
    static char *password;

    void createAccount();
    bool login();
    void executeCommands();

public:
    static ShellProgram* getInstance(Disk *disk, size_t blocks);
    static void destroyInstance();

    void run();
};