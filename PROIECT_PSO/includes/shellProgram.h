#include "fileSystemAPI.h"
#include "termios.h"

#define COMMAND_LENGTH 20
#define LENGTH 20

#define DOMAIN_NAME "Legendary"
#define ROOT_NAME "root"
#define ROOT_PASSWORD "seful"
#define ROOT_GROUP "ROOT"

#define EXIT_COMMAND "exit"
#define RETURN_COMMAND "return"
#define GROUPADD_COMMAND "groupadd"
#define SETGROUP_COMMAND "setgroup"
#define DELETEUSER_COMMAND "deleteuser"
#define DELETEGROUP_COMMAND "deletegroup"

class ShellProgram{
public:
    static ShellProgram* getInstance(Disk *disk, size_t blocks);
    static void destroyInstance();

    void run();

private:
    ShellProgram(Disk *disk, size_t blocks);
    ~ShellProgram();

    static ShellProgram *instance;
    static FileSystemAPI *myAPI;

    static char *username;
    static char *password;

    static uint32_t userID;
    static bool executeFlag;
    static bool exitFlag;
    static bool returnFlag;

    void createAccount();
    bool login();
    bool checkCredentials();
    bool checkRootPrivilege();

    void prepareCommands();
    void executeCommands(char *line);

    void turnOffEcho();
    void turnOnEcho();
    void readPassword(char *password);

    void fflushInputBuffer();
};