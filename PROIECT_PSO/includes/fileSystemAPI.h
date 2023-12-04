#include "fileSystem.h"

#define MAX_USERS 100
#define USERS_FILE "users_file.txt"
#define USERNAME_LENGTH 100
#define PASSWORD_LENGTH 100

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
    bool formatFileSystem();

    ssize_t createFile(const char* filename, uint32_t ownerUserID, uint32_t ownerGroupID, uint32_t permissions);
    bool removeFile(const char* filename);
    statDetails getFileStat(const char* filename);

    ssize_t readFile(const char* filename, char *data, size_t length, size_t offset = 0);
    ssize_t writeFile(const char* filename, const char *data, size_t length, size_t offset = 0);
    bool execute(const char* filename);

    void readUsersFile();
    void writeUsersFile();

private:
    fileSystemAPI(const char *disk_path, size_t disk_blocks);
    ~fileSystemAPI();

    static fileSystemAPI *instance;
    static User *users;

    const char *diskPath;

    size_t diskBlocks;
    size_t totalUsers;
};