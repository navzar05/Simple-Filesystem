#include "fileSystemAPI.h"
#include "termios.h"

#define COMMAND_LENGTH 20
#define LENGTH 20
#define FILE_PERMISSIONS 0644

#define DOMAIN_NAME "Legendary"

#define EXIT_COMMAND "exit"
#define RETURN_COMMAND "return"
#define INSTRUCTIONS_COMMAND "help"

#define GROUP_ADD_COMMAND "groupadd"
#define SET_GROUP_COMMAND "setgroup"

#define CHANGE_USER_PERMISSIONS_COMMAND "chusr"
#define CHANGE_GROUP_PERMISSIONS_COMMAND "chgrp"
#define DELETE_USER_COMMAND "deleteuser"
#define DELETE_GROUP_COMMAND "deletegroup"

#define SHOW_USERS_COMMAND "showusers"
#define SHOW_GROUPS_COMMAND "showgroups"

#define CREATE_FILE_COMMAND "touch"
#define COPY_FILE_COMMAND "cp"
#define MOVE_FILE_COMMAND "mv"
#define READ_FILE_COMMAND "cat"
#define DELETE_FILE_COMMAND "rm"

#define FORMAT_COMMAND "format"
#define MOUNT_COMMAND "mount"
#define UMNOUNT_COMMAND "unmount"

enum class CommandType{
    GroupAddCommand,
    SetGroupCommand,
    ChangeUserPermissionsCommand,
    ChangeGroupPermissionsCommand,
    DeleteUserCommand,
    DeleteGroupCommand,
    ShowUsersCommand,
    ShowGroupsCommand,
    CreateFileCommand,
    CopyFileCommand,
    MoveFileCommand,
    ReadFileCommand,
    DeleteFileCommand,
    FormatCommand,
    MountCommand,
    UnmountCommand
};


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
    static uint32_t groupID;

    static bool executeFlag;
    static bool exitFlag;
    static bool returnFlag;

    void createAccount();
    bool login();

    bool checkCredentials();
    bool checkRootPrivilege();
    bool checkCommand(const char *shellCommand, const char* userCommand);

    void prepareCommands();
    void executeCommands(char *line);

    void turnOffEcho();
    void turnOnEcho();
    void readPassword(char *password);

    void fflushInputBuffer();
    void showInstructions();
    CommandType selectCommand(const char *command);

    void grpAddCommand(char *parameters);
    void setGrpCommand(char *parameters);
    void chUsrPermCommand(char *parameters);
    void chGrpPermCommand(char *parameters);
    void delUsrCommand(char *parameters);
    void delGrpCommand(char *parameters);
    void showUsrCommand(char *parameters);
    void showGrpCommand(char *parameters);
    void createFileCommand(char *parameters);
    void copyFileCommand(char *parameters);
    void moveFileCommand(char *parameters);
    void readFileCommand(char *parameters);
    void delFileCommand(char *paramters);
};