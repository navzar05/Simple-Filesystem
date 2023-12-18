#include "fileSystemAPI.h"
#include "termios.h"

#define COMMAND_LENGTH 50
#define LENGTH 50
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
#define COPY_IN_COMMAND "copyin"
#define COPY_OUT_COMMAND "copyout"
#define COPY_COMMAND "cp"
#define READ_FILE_COMMAND "cat"
#define DELETE_FILE_COMMAND "rm"

#define FORMAT_COMMAND "format"
#define MOUNT_COMMAND "mount"
#define UMNOUNT_COMMAND "unmount"

#define ALL_FILES_COMMAND "ls"
#define ONE_FILE_COMMAND "stat"
#define CHG_PERM_FILE_COMMAND "chmod"

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
    CopyInCommand,
    CopyOutCommand,
    CopyCommand,
    ReadFileCommand,
    DeleteFileCommand,
    FormatCommand,
    MountCommand,
    UnmountCommand,
    IncorrectCommand,
    AllFilesCommand,
    OneFileCommand,
    ChgFilePermCommand
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
    bool checkParameters(const char *command, const char *parameters);

    void prepareCommands();
    void executeCommands(char *line);
    CommandType selectCommand(const char *command);

    void turnOffEcho();
    void turnOnEcho();
    void readPassword(char *password);

    void fflushInputBuffer();
    void showInstructions();

    void grpAddCommand(char *parameters);
    void setGrpCommand(char *parameters);
    void chUsrPermCommand(char *parameters);
    void chGrpPermCommand(char *parameters);
    void delUsrCommand(char *parameters);
    void delGrpCommand(char *parameters);
    void showUsrCommand(char *parameters);
    void showGrpCommand(char *parameters);
    void createFileCommand(char *parameters);
    void copyInCommand(char *parameters);
    void copyOutCommand(char *parameters);
    void copyCommand(char *parameters);
    void readFileCommand(char *parameters);
    void delFileCommand(char *parameters);
    void formatCommand();
    void mountCommand();
    void unmountCommand();
    void showAllFilesCommand();
    void showOneFileCommand(char *parameters);
    void chgFilePermCommand(char *parameters);
};