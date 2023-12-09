#include "filesystem.h"

#define MAX_USERS 100
#define USERS_FILE "users_file.txt"
#define PASSWORDS_FILE "passwords_file.txt"

#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 50

#define WRITE_PERMISSION 4
#define READ_PERMISSION 6

struct User{
    char *username;
    char *password;
    uint32_t userID;
    uint32_t groupID;
    uint32_t permissions;
};


class fileSystemAPI {
public:
    
    static fileSystemAPI* getInstance(const char *disk_path, size_t disk_blocks);
    fileSystemAPI(fileSystemAPI&) = delete;
    static void destroyInstance();
    
    bool createUser(const char *username, const char *password, uint32_t userID);
    bool deleteUser(uint32_t userID);
    bool setUserGroup(uint32_t userID, uint32_t groupID); //schimba grupul userului
    bool addUserToGroup(uint32_t userID, uint32_t groupID); //adauga la un grup

    bool setFilePermissions(const char* filename, uint32_t permissions);
    uint32_t getFilePermissions(const char* filename);
    
    bool mountFileSystem();
    bool unmountFileSystem();
    bool formatFileSystem();

    ssize_t createFile(const char* filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions);
    bool removeFile(const char* filename);
    statDetails getFileStat(const char* filename);

    ssize_t readFile(const char* filename, char *data, size_t length, size_t offset = 0);
    ssize_t writeFile(const char* filename, const char *data, size_t length, size_t offset = 0);
    bool execute(const char* filename);

private:
    fileSystemAPI(const char *disk_path, size_t disk_blocks);
    ~fileSystemAPI();

    static fileSystemAPI *instance;
    static User *users;
    static FileSystem *myFileSystem;

    size_t diskBlocks;
    size_t totalUsers;
    size_t currentUser;

    Disk *disk;

    bool hasPermissions(const char *filename, uint32_t mode);
    void readUsersFile();
    void writeUsersFile(const char *filename);
};