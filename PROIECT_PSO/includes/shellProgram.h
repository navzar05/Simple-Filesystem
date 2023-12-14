#include "fileSystemAPI.h"
#include "termios.h"

#define COMMAND_LENGTH 20
#define LENGTH 20

#define DOMAIN_NAME "Legendary"
#define exitCommand "exit"
#define returnCommand "return"

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

    void prepareCommands();
    void executeCommands(const char *command);

    void turnOffEcho();
    void turnOnEcho();
    void readPassword(char *password);

    void fflushInputBuffer();
};