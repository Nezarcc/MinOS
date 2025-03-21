#include "user.h"
#include "../kernel/kernel.h"
#include "../fs/file.h"
#include <stdint.h>
#include <string.h>

/* Maximum number of users */
#define MAX_USERS 64

/* Maximum number of groups */
#define MAX_GROUPS 32

/* User database */
static user_t users[MAX_USERS];
static uint32_t user_count = 0;

/* Group database */
static group_t groups[MAX_GROUPS];
static uint32_t group_count = 0;

/* Initialize the user management system */
void user_init() {
    terminal_writestring("Initializing user management system...\n");
    
    // Clear user database
    for (int i = 0; i < MAX_USERS; i++) {
        users[i].uid = 0;
        users[i].gid = 0;
        users[i].username[0] = '\0';
        users[i].password_hash[0] = '\0';
        users[i].home_dir[0] = '\0';
        users[i].shell[0] = '\0';
    }
    
    // Clear group database
    for (int i = 0; i < MAX_GROUPS; i++) {
        groups[i].gid = 0;
        groups[i].name[0] = '\0';
        groups[i].member_count = 0;
        for (int j = 0; j < 32; j++) {
            groups[i].members[j] = 0;
        }
    }
    
    user_count = 0;
    group_count = 0;
    
    // Add root user
    user_add("root", "root", 0, 0);
    
    // Add root group
    group_add("root", 0);
    
    // Add wheel group (admin)
    group_add("wheel", 10);
    
    // Add users group
    group_add("users", 100);
    
    terminal_writestring("User management system initialized\n");
}

/* Add a user */
int user_add(const char* username, const char* password, uint32_t uid, uint32_t gid) {
    if (!username || !password) {
        return -1;
    }
    
    // Check if user already exists
    if (user_find_by_name(username) != NULL) {
        return -1; // User already exists
    }
    
    // Check if UID is already in use
    if (user_find_by_uid(uid) != NULL) {
        return -1; // UID already in use
    }
    
    // Check if we have room for another user
    if (user_count >= MAX_USERS) {
        return -1; // User database full
    }
    
    // Add the user
    users[user_count].uid = uid;
    users[user_count].gid = gid;
    
    strncpy(users[user_count].username, username, USER_MAX_NAME_LENGTH - 1);
    users[user_count].username[USER_MAX_NAME_LENGTH - 1] = '\0';
    
    // Hash the password
    hash_password(password, users[user_count].password_hash, USER_MAX_HASH_LENGTH);
    
    // Set home directory
    strcpy(users[user_count].home_dir, "/home/");
    strcat(users[user_count].home_dir, username);
    
    // Set shell
    strcpy(users[user_count].shell, "/bin/minsh");
    
    user_count++;
    
    return 0;
}

/* Delete a user */
int user_delete(const char* username) {
    if (!username) {
        return -1;
    }
    
    // Find the user
    for (uint32_t i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Remove the user from all groups
            for (uint32_t j = 0; j < group_count; j++) {
                for (uint32_t k = 0; k < groups[j].member_count; k++) {
                    if (groups[j].members[k] == users[i].uid) {
                        // Remove this member
                        for (uint32_t l = k; l < groups[j].member_count - 1; l++) {
                            groups[j].members[l] = groups[j].members[l + 1];
                        }
                        groups[j].member_count--;
                        break;
                    }
                }
            }
            
            // Remove the user
            for (uint32_t j = i; j < user_count - 1; j++) {
                users[j] = users[j + 1];
            }
            
            user_count--;
            
            return 0;
        }
    }
    
    return -1; // User not found
}

/* Find a user by username */
user_t* user_find_by_name(const char* username) {
    if (!username) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return &users[i];
        }
    }
    
    return NULL; // User not found
}

/* Find a user by UID */
user_t* user_find_by_uid(uint32_t uid) {
    for (uint32_t i = 0; i < user_count; i++) {
        if (users[i].uid == uid) {
            return &users[i];
        }
    }
    
    return NULL; // User not found
}

/* Authenticate a user */
int user_authenticate(const char* username, const char* password) {
    if (!username || !password) {
        return -1;
    }
    
    // Find the user
    user_t* user = user_find_by_name(username);
    if (!user) {
        return -1; // User not found
    }
    
    // Verify the password
    return verify_password(password, user->password_hash);
}

/* Add a group */
int group_add(const char* name, uint32_t gid) {
    if (!name) {
        return -1;
    }
    
    // Check if group already exists
    if (group_find_by_name(name) != NULL) {
        return -1; // Group already exists
    }
    
    // Check if GID is already in use
    if (group_find_by_gid(gid) != NULL) {
        return -1; // GID already in use
    }
    
    // Check if we have room for another group
    if (group_count >= MAX_GROUPS) {
        return -1; // Group database full
    }
    
    // Add the group
    groups[group_count].gid = gid;
    
    strncpy(groups[group_count].name, name, USER_MAX_NAME_LENGTH - 1);
    groups[group_count].name[USER_MAX_NAME_LENGTH - 1] = '\0';
    
    groups[group_count].member_count = 0;
    
    group_count++;
    
    return 0;
}

/* Delete a group */
int group_delete(const char* name) {
    if (!name) {
        return -1;
    }
    
    // Find the group
    for (uint32_t i = 0; i < group_count; i++) {
        if (strcmp(groups[i].name, name) == 0) {
            // Remove the group
            for (uint32_t j = i; j < group_count - 1; j++) {
                groups[j] = groups[j + 1];
            }
            
            group_count--;
            
            return 0;
        }
    }
    
    return -1; // Group not found
}

/* Find a group by name */
group_t* group_find_by_name(const char* name) {
    if (!name) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < group_count; i++) {
        if (strcmp(groups[i].name, name) == 0) {
            return &groups[i];
        }
    }
    
    return NULL; // Group not found
}

/* Find a group by GID */
group_t* group_find_by_gid(uint32_t gid) {
    for (uint32_t i = 0; i < group_count; i++) {
        if (groups[i].gid == gid) {
            return &groups[i];
        }
    }
    
    return NULL; // Group not found
}

/* Add a user to a group */
int group_add_user(const char* group_name, const char* username) {
    if (!group_name || !username) {
        return -1;
    }
    
    // Find the group
    group_t* group = group_find_by_name(group_name);
    if (!group) {
        return -1; // Group not found
    }
    
    // Find the user
    user_t* user = user_find_by_name(username);
    if (!user) {
        return -1; // User not found
    }
    
    // Check if user is already in the group
    for (uint32_t i = 0; i < group->member_count; i++) {
        if (group->members[i] == user->uid) {
            return 0; // User already in group
        }
    }
    
    // Check if group is full
    if (group->member_count >= 32) {
        return -1; // Group is full
    }
    
    // Add user to group
    group->members[group->member_count++] = user->uid;
    
    return 0;
}

/* Remove a user from a group */
int group_remove_user(const char* group_name, const char* username) {
    if (!group_name || !username) {
        return -1;
    }
    
    // Find the group
    group_t* group = group_find_by_name(group_name);
    if (!group) {
        return -1; // Group not found
    }
    
    // Find the user
    user_t* user = user_find_by_name(username);
    if (!user) {
        return -1; // User not found
    }
    
    // Find the user in the group
    for (uint32_t i = 0; i < group->member_count; i++) {
        if (group->members[i] == user->uid) {
            // Remove the user
            for (uint32_t j = i; j < group->member_count - 1; j++) {
                group->members[j] = group->members[j + 1];
            }
            
            group->member_count--;
            
            return 0;
        }
    }
    
    return -1; // User not in group
}

/* Check if a user is in a group */
int user_in_group(const char* username, const char* group_name) {
    if (!username || !group_name) {
        return 0;
    }
    
    // Find the user
    user_t* user = user_find_by_name(username);
    if (!user) {
        return 0; // User not found
    }
    
    // Find the group
    group_t* group = group_find_by_name(group_name);
    if (!group) {
        return 0; // Group not found
    }
    
    // Check if user's primary group is this group
    if (user->gid == group->gid) {
        return 1;
    }
    
    // Check if user is in the group's member list
    for (uint32_t i = 0; i < group->member_count; i++) {
        if (group->members[i] == user->uid) {
            return 1;
        }
    }
    
    return 0; // User not in group
}

/* Hash a password */
void hash_password(const char* password, char* hash, int hash_size) {
    if (!password || !hash || hash_size <= 0) {
        return;
    }
    
    // In a real implementation, this would use a secure hashing algorithm like bcrypt or Argon2
    // For now, we'll just use a simple hash function
    
    // Start with a simple salt
    const char* salt = "MinOS";
    
    // Combine salt and password
    char salted[256];
    strcpy(salted, salt);
    strcat(salted, password);
    
    // Simple hash function
    uint32_t h = 0;
    for (int i = 0; salted[i] != '\0'; i++) {
        h = 31 * h + salted[i];
    }
    
    // Convert to string
    snprintf(hash, hash_size, "%08x", h);
}

/* Verify a password against a hash */
int verify_password(const char* password, const char* hash) {
    if (!password || !hash) {
        return 0;
    }
    
    // Hash the password
    char computed_hash[USER_MAX_HASH_LENGTH];
    hash_password(password, computed_hash, USER_MAX_HASH_LENGTH);
    
    // Compare the hashes
    return strcmp(computed_hash, hash) == 0;
}
