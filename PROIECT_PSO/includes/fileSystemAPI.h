#include "filesystem.h"

#define MAX_USERS 10
#define MAX_GROUPS 10

#define USERS_FILE "users_file.txt"
#define PASSWORDS_FILE "passwords_file.txt"
#define GROUPS_FILE "groups_file.txt"

#define USERNAME_LENGTH 20
#define PASSWORD_LENGTH 20
#define GROUP_LENGTH 20

#define WRITE_PERMISSION 02
#define READ_PERMISSION  04

#define ROOT_NAME "root"
#define ROOT_PASSWORD "seful"
#define ROOT_GROUP "ROOT"

struct User{
    char *username;
    char *password;
    uint32_t userID;
    uint32_t groupID;
    uint32_t permissions;
};

struct Group{
    char *groupname;
    int *usersID;
    int nrUsers;
    uint32_t permissions;
    uint32_t groupID;
};

class FileSystemAPI {
private:
    friend class FileSystem;
    FileSystemAPI(Disk *disk_path, size_t disk_blocks);
    ~FileSystemAPI();

    static FileSystemAPI *instance;
    static User *users;
    static FileSystem *myFileSystem;
    static Group *groups;
    static Disk *disk;

    static size_t totalUsers;
    static size_t totalGroups;
    static size_t diskBlocks;
    static size_t currentUser;

    static bool isUsersFile;
    static bool isPasswordsFile;
    static bool isGroupsFile;
    static bool *bitmapUsers;
    static bool *bitmapGroups;

    bool hasPermissions(const char *filename, uint32_t mode);

    void readImportantFile(const char *filename);
    void readUsersFile(const char *token, int index);
    void readPassswordsFile(const char *token, int index);
    void readGroupsFile(const char *token, int index);

    void writeImportantFile(const char *filename);
    void writeUsersFile(char *line, size_t length, int i);
    void writePasswordsFile(char *line, size_t length, int i);
    void writeGroupsFile(char *line, size_t length, int i);

    void seeTypeImportantFile(const char *filename);

public:
    static FileSystemAPI* getInstance(Disk *disk_path, size_t disk_blocks);
    FileSystemAPI(FileSystemAPI&) = delete;
    static void destroyInstance();

    bool createUser(const char *username, const char *password, uint32_t userID);
    bool deleteUser(uint32_t userID);
    bool setUserGroup(uint32_t userID, uint32_t groupID); //schimba grupul userului
    bool createGroup(const char *groupname, uint32_t groupID);
    bool deleteGroup(uint32_t groupID);

    bool setFilePermissions(const char* filename, uint32_t permissions);
    uint32_t getFilePermissions(const char* filename);

    bool mountFileSystem();
    bool unmountFileSystem();
    bool formatFileSystem();


    bool checkCredentials(const char *username, const char *password);
    bool setCurrentUser(uint32_t userID);
    uint32_t setUserID();
    uint32_t setGroupID();
    
    uint32_t getUserID(const char *username);
    uint32_t getGroupID(const char *groupname);
    uint32_t getCurrentGroupID();

    ssize_t createFile(const char* filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions);
    bool removeFile(const char* filename);
    statDetails getFileStat(const char* filename);

    ssize_t readFile(const char* filename, char *data, size_t length, size_t offset = 0);
    ssize_t writeFile(const char* filename, const char *data, size_t length, size_t offset = 0);

    void showUsers();
    void showGroups();

    bool changeUserPermissions(uint32_t userID, uint32_t permissions);
    bool changeGroupPermissions(uint32_t groupID, uint32_t permissions);    
};