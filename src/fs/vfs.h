#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

/* File types */
#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_CHARDEVICE  0x03
#define VFS_BLOCKDEVICE 0x04
#define VFS_PIPE        0x05
#define VFS_SYMLINK     0x06
#define VFS_MOUNTPOINT  0x08

/* File permissions */
#define VFS_READ        0x01
#define VFS_WRITE       0x02
#define VFS_EXECUTE     0x04

/* Seek modes */
#define VFS_SEEK_SET    0x01
#define VFS_SEEK_CUR    0x02
#define VFS_SEEK_END    0x03

/* Forward declarations */
struct fs_node;
struct dirent;

/* Function pointer types for file operations */
typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent* (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node* (*finddir_type_t)(struct fs_node*, char* name);
typedef void (*create_type_t)(struct fs_node*, char* name, uint16_t permission);
typedef void (*unlink_type_t)(struct fs_node*, char* name);

/* File system node structure */
typedef struct fs_node {
    char name[256];             /* Filename */
    uint32_t mask;              /* Permission mask */
    uint32_t uid;               /* User ID */
    uint32_t gid;               /* Group ID */
    uint32_t flags;             /* File type and attributes */
    uint32_t inode;             /* Inode number */
    uint32_t length;            /* File size */
    uint32_t impl;              /* Implementation-specific data */
    
    /* File operations */
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    create_type_t create;
    unlink_type_t unlink;
    
    struct fs_node* ptr;        /* Used for mountpoints and symlinks */
} fs_node_t;

/* Directory entry structure */
typedef struct dirent {
    char name[256];             /* Filename */
    uint32_t inode;             /* Inode number */
} dirent_t;

/* VFS functions */
uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer);
void vfs_open(fs_node_t* node, uint8_t read, uint8_t write);
void vfs_close(fs_node_t* node);
dirent_t* vfs_readdir(fs_node_t* node, uint32_t index);
fs_node_t* vfs_finddir(fs_node_t* node, char* name);
void vfs_create(fs_node_t* node, char* name, uint16_t permission);
void vfs_unlink(fs_node_t* node, char* name);

/* Initialize the VFS */
void vfs_init(void);

/* Mount a file system */
int vfs_mount(char* path, fs_node_t* node);

/* Get the root node */
fs_node_t* vfs_get_root(void);

#endif /* VFS_H */
