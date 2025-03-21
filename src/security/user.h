#ifndef USER_H
#define USER_H

#include <stdint.h>

/* Maximum username length */
#define USER_MAX_NAME_LENGTH 32

/* Maximum password hash length */
#define USER_MAX_HASH_LENGTH 64

/* User structure */
typedef struct {
    uint32_t uid;                          /* User ID */
    uint32_t gid;                          /* Primary group ID */
    char username[USER_MAX_NAME_LENGTH];   /* Username */
    char password_hash[USER_MAX_HASH_LENGTH]; /* Password hash */
    char home_dir[256];                    /* Home directory */
    char shell[256];                       /* Login shell */
} user_t;

/* Group structure */
typedef struct {
    uint32_t gid;                          /* Group ID */
    char name[USER_MAX_NAME_LENGTH];       /* Group name */
    uint32_t members[32];                  /* Member UIDs */
    uint32_t member_count;                 /* Number of members */
} group_t;

/* Initialize the user management system */
void user_init(void);

/* Add a user */
int user_add(const char* username, const char* password, uint32_t uid, uint32_t gid);

/* Delete a user */
int user_delete(const char* username);

/* Find a user by username */
user_t* user_find_by_name(const char* username);

/* Find a user by UID */
user_t* user_find_by_uid(uint32_t uid);

/* Authenticate a user */
int user_authenticate(const char* username, const char* password);

/* Add a group */
int group_add(const char* name, uint32_t gid);

/* Delete a group */
int group_delete(const char* name);

/* Find a group by name */
group_t* group_find_by_name(const char* name);

/* Find a group by GID */
group_t* group_find_by_gid(uint32_t gid);

/* Add a user to a group */
int group_add_user(const char* group_name, const char* username);

/* Remove a user from a group */
int group_remove_user(const char* group_name, const char* username);

/* Check if a user is in a group */
int user_in_group(const char* username, const char* group_name);

/* Hash a password */
void hash_password(const char* password, char* hash, int hash_size);

/* Verify a password against a hash */
int verify_password(const char* password, const char* hash);

#endif /* USER_H */
