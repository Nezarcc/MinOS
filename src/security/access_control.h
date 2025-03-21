#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H

#include <stdint.h>

/* Permission bits */
#define PERM_READ    0x04
#define PERM_WRITE   0x02
#define PERM_EXECUTE 0x01

/* Special permission bits */
#define PERM_SETUID  0x800
#define PERM_SETGID  0x400
#define PERM_STICKY  0x200

/* Access control entry types */
#define ACE_TYPE_ALLOW 0
#define ACE_TYPE_DENY  1
#define ACE_TYPE_AUDIT 2

/* Access control entry structure */
typedef struct {
    uint8_t type;        /* Allow, deny, or audit */
    uint32_t subject_id; /* User or group ID */
    uint8_t is_group;    /* 1 if subject is a group, 0 if user */
    uint16_t perms;      /* Permission bits */
} access_control_entry_t;

/* Access control list structure */
typedef struct {
    access_control_entry_t* entries;
    uint32_t entry_count;
    uint32_t capacity;
} access_control_list_t;

/* Initialize the access control system */
void access_control_init(void);

/* Create a new access control list */
access_control_list_t* access_control_create(void);

/* Free an access control list */
void access_control_free(access_control_list_t* acl);

/* Add an entry to an access control list */
int access_control_add_entry(access_control_list_t* acl, uint8_t type, uint32_t subject_id, uint8_t is_group, uint16_t perms);

/* Remove an entry from an access control list */
int access_control_remove_entry(access_control_list_t* acl, uint32_t index);

/* Check if a user has permission to access a resource */
int access_control_check_permission(access_control_list_t* acl, uint32_t uid, uint32_t* groups, uint32_t group_count, uint16_t requested_perms);

/* Convert traditional Unix permissions to an access control list */
access_control_list_t* access_control_from_unix_perms(uint16_t mode, uint32_t owner_uid, uint32_t group_gid);

/* Convert an access control list to traditional Unix permissions */
uint16_t access_control_to_unix_perms(access_control_list_t* acl, uint32_t* owner_uid, uint32_t* group_gid);

#endif /* ACCESS_CONTROL_H */
