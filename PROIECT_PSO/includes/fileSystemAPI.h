#include "filesystem.h"

#define MAX_USERS 10
#define USERS_FILE "users_file.txt"
#define PASSWORDS_FILE "passwords_file.txt"
#define GROUPS_FILE "groups_file.txt"

#define USERNAME_LENGTH 20
#define PASSWORD_LENGTH 20

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
private:
    friend class FileSystem;
    fileSystemAPI(Disk *disk_path, size_t disk_blocks);
    ~fileSystemAPI();

    static fileSystemAPI *instance;
    static User *users;
    static FileSystem *myFileSystem;
    static Disk *disk;

    static size_t totalUsers;
    static size_t diskBlocks;
    static size_t currentUser;
    static size_t inumberUsersFile;
    static size_t inumberPasswordsFile;
    static size_t inumberGroupsFile;

    bool hasPermissions(const char *filename, uint32_t mode);
    void readImportantFile(const char *filename);
    void writeImportantFile(const char *filename);

public:
    static fileSystemAPI* getInstance(Disk *disk_path, size_t disk_blocks);
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
};