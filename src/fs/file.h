#ifndef FILE_H
#define FILE_H

#include "vfs.h"
#include <stdint.h>
#include <stddef.h>

/* File descriptor structure */
typedef struct {
    fs_node_t* node;     /* VFS node */
    uint32_t offset;     /* Current file position */
    uint32_t flags;      /* Open flags */
    uint32_t refcount;   /* Reference count */
} file_descriptor_t;

/* File open flags */
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_APPEND    0x0008
#define O_CREAT     0x0100
#define O_TRUNC     0x0200
#define O_EXCL      0x0400
#define O_NOFOLLOW  0x0800

/* Initialize file system interface */
void file_init(void);

/* Open a file */
int file_open(const char* path, uint32_t flags);

/* Close a file */
int file_close(int fd);

/* Read from a file */
int file_read(int fd, void* buffer, uint32_t size);

/* Write to a file */
int file_write(int fd, const void* buffer, uint32_t size);

/* Seek within a file */
int file_seek(int fd, int offset, int whence);

/* Get file status */
int file_stat(const char* path, void* stat_buf);

/* Create a directory */
int file_mkdir(const char* path, uint32_t mode);

/* Remove a file */
int file_unlink(const char* path);

/* Remove a directory */
int file_rmdir(const char* path);

/* Change current directory */
int file_chdir(const char* path);

/* Get current directory */
char* file_getcwd(char* buf, uint32_t size);

#endif /* FILE_H */
